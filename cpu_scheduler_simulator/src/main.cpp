#include "scheduler.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace {

using schedsim::Process;
using schedsim::Scheduler;
using schedsim::SimulationResult;

struct Options {
    std::string input_file;
    std::string policy = "all";
    int quantum = 3;
    bool help = false;
};

std::string trim(const std::string& s) {
    std::size_t first = 0;
    while (first < s.size() && std::isspace(static_cast<unsigned char>(s[first]))) {
        ++first;
    }

    std::size_t last = s.size();
    while (last > first && std::isspace(static_cast<unsigned char>(s[last - 1]))) {
        --last;
    }

    return s.substr(first, last - first);
}

std::string lower_case(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

bool looks_like_integer(const std::string& s) {
    if (s.empty()) {
        return false;
    }

    std::size_t pos = 0;
    if (s[pos] == '+' || s[pos] == '-') {
        ++pos;
    }
    if (pos == s.size()) {
        return false;
    }

    while (pos < s.size()) {
        if (!std::isdigit(static_cast<unsigned char>(s[pos]))) {
            return false;
        }
        ++pos;
    }
    return true;
}

int parse_int(const std::string& text, const std::string& field, int line_number) {
    if (!looks_like_integer(text)) {
        throw std::runtime_error("Line " + std::to_string(line_number) +
                                 ": expected integer for " + field + ", got '" + text + "'");
    }
    return std::stoi(text);
}

std::vector<std::string> split_csv(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;

    while (std::getline(ss, field, ',')) {
        fields.push_back(trim(field));
    }

    return fields;
}

void validate_process(const Process& p, int line_number) {
    if (p.id.empty()) {
        throw std::runtime_error("Line " + std::to_string(line_number) + ": process id cannot be empty");
    }
    if (p.arrival_time < 0) {
        throw std::runtime_error("Line " + std::to_string(line_number) + ": arrival time cannot be negative");
    }
    if (p.burst_time <= 0) {
        throw std::runtime_error("Line " + std::to_string(line_number) + ": burst time must be positive");
    }
    if (p.priority < 0) {
        throw std::runtime_error("Line " + std::to_string(line_number) + ": priority cannot be negative");
    }
    if (p.deadline < 0) {
        throw std::runtime_error("Line " + std::to_string(line_number) + ": deadline cannot be negative");
    }
}

std::vector<Process> read_processes(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("could not open input file: " + path);
    }

    std::vector<Process> processes;
    std::unordered_set<std::string> seen_ids;

    std::string line;
    int line_number = 0;

    while (std::getline(file, line)) {
        ++line_number;
        line = trim(line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        const auto fields = split_csv(line);
        if (fields.size() != 5) {
            throw std::runtime_error("Line " + std::to_string(line_number) +
                                     ": expected fields id,arrival,burst,priority,deadline");
        }

        // Simple CSV header support.
        if (processes.empty() && lower_case(fields[0]) == "id") {
            continue;
        }

        Process p;
        p.id = fields[0];
        p.arrival_time = parse_int(fields[1], "arrival", line_number);
        p.burst_time = parse_int(fields[2], "burst", line_number);
        p.priority = parse_int(fields[3], "priority", line_number);
        p.deadline = parse_int(fields[4], "deadline", line_number);

        validate_process(p, line_number);

        if (!seen_ids.insert(p.id).second) {
            throw std::runtime_error("Line " + std::to_string(line_number) +
                                     ": duplicate process id '" + p.id + "'");
        }

        processes.push_back(p);
    }

    if (processes.empty()) {
        throw std::runtime_error("no processes found in input file: " + path);
    }

    return processes;
}

std::vector<Process> sample_workload() {
    return {
        {"P1", 0, 8, 2, 18},
        {"P2", 1, 4, 1, 10},
        {"P3", 2, 9, 3, 25},
        {"P4", 3, 5, 2, 16},
        {"P5", 6, 2, 1, 12},
    };
}

Options parse_options(int argc, char** argv) {
    Options options;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        auto next_arg = [&](const std::string& option) -> std::string {
            if (i + 1 >= argc) {
                throw std::runtime_error("missing value after " + option);
            }
            return argv[++i];
        };

        if (arg == "--help" || arg == "-h") {
            options.help = true;
        } else if (arg == "--input") {
            options.input_file = next_arg(arg);
        } else if (arg == "--policy") {
            options.policy = lower_case(next_arg(arg));
        } else if (arg == "--quantum") {
            const std::string value = next_arg(arg);
            if (!looks_like_integer(value)) {
                throw std::runtime_error("--quantum expects an integer");
            }
            options.quantum = std::stoi(value);
            if (options.quantum <= 0) {
                throw std::runtime_error("--quantum must be positive");
            }
        } else {
            throw std::runtime_error("unknown argument: " + arg);
        }
    }

    return options;
}

void print_usage(const char* program) {
    std::cout << "CPU Scheduler Simulator\n\n"
              << "Usage:\n"
              << "  " << program << " [--input file.csv] [--policy all|fcfs|sjf|priority|rr] [--quantum N]\n\n"
              << "CSV format:\n"
              << "  id,arrival,burst,priority,deadline\n\n"
              << "Notes:\n"
              << "  - SJF and Priority Scheduling are non-preemptive in this simulator.\n"
              << "  - Smaller priority number means higher priority.\n"
              << "  - A process misses its deadline if completion_time > deadline.\n"
              << "  - If --input is omitted, a built-in sample workload is used.\n";
}

void print_workload(const std::vector<Process>& processes) {
    std::cout << "\n==================== INPUT WORKLOAD ====================\n";
    std::cout << "Priority convention: smaller number = higher priority\n";
    std::cout << "Deadline rule: missed if completion_time > deadline\n\n";

    std::cout << std::left << std::setw(10) << "Process"
              << std::right << std::setw(10) << "Arrival"
              << std::setw(10) << "Burst"
              << std::setw(10) << "Priority"
              << std::setw(10) << "Deadline" << '\n';
    std::cout << std::string(50, '-') << '\n';

    for (const auto& p : processes) {
        std::cout << std::left << std::setw(10) << p.id
                  << std::right << std::setw(10) << p.arrival_time
                  << std::setw(10) << p.burst_time
                  << std::setw(10) << p.priority
                  << std::setw(10) << p.deadline << '\n';
    }
}

void print_timeline(const SimulationResult& result) {
    std::cout << "\nExecution timeline:\n";
    for (const auto& segment : result.timeline) {
        std::cout << "  [" << std::setw(2) << segment.start_time
                  << ", " << std::setw(2) << segment.end_time << ")  "
                  << segment.label << '\n';
    }

    std::cout << "\nCompact Gantt view:\n  ";
    for (const auto& segment : result.timeline) {
        std::cout << '|' << segment.start_time << ' ' << segment.label << ' ';
    }
    if (!result.timeline.empty()) {
        std::cout << '|' << result.timeline.back().end_time;
    }
    std::cout << '\n';
}

void print_process_table(const SimulationResult& result) {
    std::cout << "\nPer-process results:\n";
    std::cout << std::left << std::setw(10) << "Process"
              << std::right << std::setw(8) << "Arr"
              << std::setw(8) << "Burst"
              << std::setw(8) << "Prio"
              << std::setw(10) << "Deadline"
              << std::setw(12) << "FirstStart"
              << std::setw(10) << "Finish"
              << std::setw(10) << "Waiting"
              << std::setw(12) << "Turnaround"
              << std::setw(10) << "Response"
              << std::setw(9) << "Missed" << '\n';

    std::cout << std::string(107, '-') << '\n';

    for (const auto& p : result.processes) {
        std::cout << std::left << std::setw(10) << p.process.id
                  << std::right << std::setw(8) << p.process.arrival_time
                  << std::setw(8) << p.process.burst_time
                  << std::setw(8) << p.process.priority
                  << std::setw(10) << p.process.deadline
                  << std::setw(12) << p.first_start_time
                  << std::setw(10) << p.completion_time
                  << std::setw(10) << p.waiting_time
                  << std::setw(12) << p.turnaround_time
                  << std::setw(10) << p.response_time
                  << std::setw(9) << (p.missed_deadline ? "yes" : "no")
                  << '\n';
    }
}

void print_summary(const SimulationResult& result) {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\nSummary metrics:\n"
              << "  Makespan              : " << result.makespan << '\n'
              << "  Total busy time       : " << result.total_busy_time << '\n'
              << "  CPU utilization       : " << result.cpu_utilization_percent << "%\n"
              << "  Throughput            : " << result.throughput << " processes/unit-time\n"
              << "  Average waiting time  : " << result.average_waiting_time << '\n'
              << "  Average turnaround    : " << result.average_turnaround_time << '\n'
              << "  Average response time : " << result.average_response_time << '\n'
              << "  Deadline misses       : " << result.deadline_misses
              << " / " << result.processes.size() << '\n';
}

void print_result(const SimulationResult& result) {
    std::cout << "\n\n========================================================\n";
    std::cout << result.policy_name << '\n';
    std::cout << "========================================================\n";
    std::cout << result.description << '\n';

    print_timeline(result);
    print_process_table(result);
    print_summary(result);
}

bool is_rr_policy(const std::string& policy) {
    return policy == "rr" || policy == "round-robin" || policy == "round_robin";
}

std::vector<SimulationResult> run_policies(const std::vector<Process>& processes, const Options& options) {
    std::vector<SimulationResult> results;

    if (options.policy == "all" || options.policy == "fcfs") {
        results.push_back(Scheduler::run_fcfs(processes));
    }
    if (options.policy == "all" || options.policy == "sjf") {
        results.push_back(Scheduler::run_sjf(processes));
    }
    if (options.policy == "all" || options.policy == "priority") {
        results.push_back(Scheduler::run_priority(processes));
    }
    if (options.policy == "all" || is_rr_policy(options.policy)) {
        results.push_back(Scheduler::run_round_robin(processes, options.quantum));
    }

    if (results.empty()) {
        throw std::runtime_error("unknown policy: " + options.policy);
    }

    return results;
}

void print_comparison(const std::vector<SimulationResult>& results) {
    if (results.size() <= 1) {
        return;
    }

    std::cout << "\n\n==================== POLICY COMPARISON ====================\n";
    std::cout << std::left << std::setw(32) << "Policy"
              << std::right << std::setw(12) << "Avg Wait"
              << std::setw(16) << "Avg Turnaround"
              << std::setw(14) << "Avg Resp"
              << std::setw(10) << "Misses"
              << std::setw(12) << "Util %" << '\n';
    std::cout << std::string(96, '-') << '\n';

    std::cout << std::fixed << std::setprecision(2);
    for (const auto& result : results) {
        std::cout << std::left << std::setw(32) << result.policy_name
                  << std::right << std::setw(12) << result.average_waiting_time
                  << std::setw(16) << result.average_turnaround_time
                  << std::setw(14) << result.average_response_time
                  << std::setw(10) << result.deadline_misses
                  << std::setw(12) << result.cpu_utilization_percent
                  << '\n';
    }

    std::cout << "\nInterpretation hints:\n"
              << "  - Lower waiting/turnaround time is usually better for batch-style workloads.\n"
              << "  - Lower response time is important for interactive workloads.\n"
              << "  - Round Robin often improves response time, but can increase turnaround time.\n"
              << "  - Deadline misses show whether a policy is suitable for time-bound workloads.\n";
}

} // namespace

int main(int argc, char** argv) {
    try {
        const Options options = parse_options(argc, argv);
        if (options.help) {
            print_usage(argv[0]);
            return 0;
        }

        const std::vector<Process> processes = options.input_file.empty()
            ? sample_workload()
            : read_processes(options.input_file);

        std::cout << "CPU Scheduler Simulator\n";
        std::cout << "Simulating FCFS, SJF, Priority Scheduling, and Round Robin.\n";
        std::cout << "Round Robin quantum: " << options.quantum << "\n";

        print_workload(processes);

        const auto results = run_policies(processes, options);
        for (const auto& result : results) {
            print_result(result);
        }

        print_comparison(results);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n\n";
        print_usage(argv[0]);
        return 1;
    }
}
