#include "scheduler.h"

#include <algorithm>
#include <limits>
#include <numeric>
#include <queue>
#include <stdexcept>
#include <utility>

namespace schedsim {
namespace {

constexpr const char* kIdle = "IDLE";

enum class NonPreemptivePolicy {
    Fcfs,
    Sjf,
    Priority
};

struct Job {
    Process process;
    int input_order = 0;
};

std::vector<Job> make_jobs(const std::vector<Process>& processes) {
    std::vector<Job> jobs;
    jobs.reserve(processes.size());

    for (int i = 0; i < static_cast<int>(processes.size()); ++i) {
        jobs.push_back({processes[i], i});
    }
    return jobs;
}

std::vector<ProcessResult> make_initial_results(const std::vector<Job>& jobs) {
    std::vector<ProcessResult> results(jobs.size());
    for (std::size_t i = 0; i < jobs.size(); ++i) {
        results[i].process = jobs[i].process;
    }
    return results;
}

void append_segment(std::vector<TimelineSegment>& timeline,
                    int start_time,
                    int end_time,
                    const std::string& label) {
    if (start_time < end_time) {
        timeline.push_back({start_time, end_time, label});
    }
}

bool arrived(const Job& job, int time) {
    return job.process.arrival_time <= time;
}

bool input_order_tie_break(const Job& a, const Job& b) {
    if (a.process.arrival_time != b.process.arrival_time) {
        return a.process.arrival_time < b.process.arrival_time;
    }
    return a.input_order < b.input_order;
}

bool better_job(const Job& a, const Job& b, NonPreemptivePolicy policy) {
    if (policy == NonPreemptivePolicy::Fcfs) {
        return input_order_tie_break(a, b);
    }

    if (policy == NonPreemptivePolicy::Sjf) {
        if (a.process.burst_time != b.process.burst_time) {
            return a.process.burst_time < b.process.burst_time;
        }
        return input_order_tie_break(a, b);
    }

    if (a.process.priority != b.process.priority) {
        return a.process.priority < b.process.priority;
    }
    return input_order_tie_break(a, b);
}

int next_arrival_after_idle(const std::vector<Job>& jobs, const std::vector<bool>& done) {
    int next_time = std::numeric_limits<int>::max();

    for (std::size_t i = 0; i < jobs.size(); ++i) {
        if (!done[i]) {
            next_time = std::min(next_time, jobs[i].process.arrival_time);
        }
    }

    return next_time;
}

int choose_ready_job(const std::vector<Job>& jobs,
                     const std::vector<bool>& done,
                     int current_time,
                     NonPreemptivePolicy policy) {
    int best = -1;

    for (int i = 0; i < static_cast<int>(jobs.size()); ++i) {
        if (done[i] || !arrived(jobs[i], current_time)) {
            continue;
        }

        if (best == -1 || better_job(jobs[i], jobs[best], policy)) {
            best = i;
        }
    }

    return best;
}

SimulationResult finish_result(std::string policy_name,
                               std::string description,
                               std::vector<TimelineSegment> timeline,
                               std::vector<ProcessResult> results) {
    SimulationResult out;
    out.policy_name = std::move(policy_name);
    out.description = std::move(description);
    out.timeline = std::move(timeline);
    out.processes = std::move(results);

    if (out.processes.empty()) {
        return out;
    }

    int first_time = std::numeric_limits<int>::max();
    int last_time = 0;
    int total_busy_time = 0;

    for (const auto& segment : out.timeline) {
        first_time = std::min(first_time, segment.start_time);
        last_time = std::max(last_time, segment.end_time);

        if (segment.label != kIdle) {
            total_busy_time += segment.end_time - segment.start_time;
        }
    }

    int total_waiting = 0;
    int total_turnaround = 0;
    int total_response = 0;
    int missed_deadlines = 0;

    for (auto& result : out.processes) {
        result.turnaround_time = result.completion_time - result.process.arrival_time;
        result.waiting_time = result.turnaround_time - result.process.burst_time;
        result.response_time = result.first_start_time - result.process.arrival_time;
        result.missed_deadline = result.completion_time > result.process.deadline;

        total_waiting += result.waiting_time;
        total_turnaround += result.turnaround_time;
        total_response += result.response_time;
        missed_deadlines += result.missed_deadline ? 1 : 0;
    }

    std::sort(out.processes.begin(), out.processes.end(),
              [](const ProcessResult& a, const ProcessResult& b) {
                  return a.process.id < b.process.id;
              });

    const int count = static_cast<int>(out.processes.size());
    out.makespan = last_time - first_time;
    out.total_busy_time = total_busy_time;
    out.deadline_misses = missed_deadlines;
    out.average_waiting_time = static_cast<double>(total_waiting) / count;
    out.average_turnaround_time = static_cast<double>(total_turnaround) / count;
    out.average_response_time = static_cast<double>(total_response) / count;

    if (out.makespan > 0) {
        out.cpu_utilization_percent = 100.0 * static_cast<double>(total_busy_time) / out.makespan;
        out.throughput = static_cast<double>(count) / out.makespan;
    }

    return out;
}

SimulationResult run_non_preemptive(const std::vector<Process>& processes,
                                    NonPreemptivePolicy policy,
                                    const std::string& policy_name,
                                    const std::string& description) {
    const int n = static_cast<int>(processes.size());
    const auto jobs = make_jobs(processes);

    std::vector<bool> done(n, false);
    auto results = make_initial_results(jobs);
    std::vector<TimelineSegment> timeline;

    int now = 0;
    int finished = 0;

    while (finished < n) {
        const int next = choose_ready_job(jobs, done, now, policy);

        if (next == -1) {
            const int wake_up_time = next_arrival_after_idle(jobs, done);
            append_segment(timeline, now, wake_up_time, kIdle);
            now = wake_up_time;
            continue;
        }

        const Process& p = jobs[next].process;
        results[next].first_start_time = now;
        append_segment(timeline, now, now + p.burst_time, p.id);

        now += p.burst_time;
        results[next].completion_time = now;
        done[next] = true;
        ++finished;
    }

    return finish_result(policy_name, description, std::move(timeline), std::move(results));
}

std::vector<int> arrival_order(const std::vector<Job>& jobs) {
    std::vector<int> order(jobs.size());
    std::iota(order.begin(), order.end(), 0);

    std::sort(order.begin(), order.end(), [&](int lhs, int rhs) {
        return input_order_tie_break(jobs[lhs], jobs[rhs]);
    });

    return order;
}

} // namespace

SimulationResult Scheduler::run_fcfs(const std::vector<Process>& processes) {
    return run_non_preemptive(
        processes,
        NonPreemptivePolicy::Fcfs,
        "FCFS - First Come First Served",
        "Non-preemptive. Among available processes, run the earliest arriving process first. Ties use input order.");
}

SimulationResult Scheduler::run_sjf(const std::vector<Process>& processes) {
    return run_non_preemptive(
        processes,
        NonPreemptivePolicy::Sjf,
        "SJF - Shortest Job First",
        "Non-preemptive. Whenever the CPU becomes free, run the available process with the smallest burst time.");
}

SimulationResult Scheduler::run_priority(const std::vector<Process>& processes) {
    return run_non_preemptive(
        processes,
        NonPreemptivePolicy::Priority,
        "Priority Scheduling",
        "Non-preemptive. Whenever the CPU becomes free, run the available process with the highest priority. Smaller priority number means higher priority.");
}

SimulationResult Scheduler::run_round_robin(const std::vector<Process>& processes, int quantum) {
    if (quantum <= 0) {
        throw std::invalid_argument("Round Robin quantum must be positive");
    }

    const int n = static_cast<int>(processes.size());
    const auto jobs = make_jobs(processes);
    const auto order = arrival_order(jobs);

    std::vector<int> remaining_time(n, 0);
    auto results = make_initial_results(jobs);

    for (int i = 0; i < n; ++i) {
        remaining_time[i] = jobs[i].process.burst_time;
    }

    std::queue<int> ready;
    std::vector<TimelineSegment> timeline;

    int now = 0;
    int finished = 0;
    std::size_t next_arrival = 0;

    auto enqueue_arrived_jobs = [&]() {
        while (next_arrival < order.size() &&
               jobs[order[next_arrival]].process.arrival_time <= now) {
            ready.push(order[next_arrival]);
            ++next_arrival;
        }
    };

    while (finished < n) {
        enqueue_arrived_jobs();

        if (ready.empty()) {
            const int wake_up_time = jobs[order[next_arrival]].process.arrival_time;
            append_segment(timeline, now, wake_up_time, kIdle);
            now = wake_up_time;
            continue;
        }

        const int current = ready.front();
        ready.pop();

        const Process& p = jobs[current].process;
        if (results[current].first_start_time == -1) {
            results[current].first_start_time = now;
        }

        const int slice = std::min(quantum, remaining_time[current]);
        append_segment(timeline, now, now + slice, p.id);

        now += slice;
        remaining_time[current] -= slice;

        // Boundary convention used here:
        // jobs that arrive exactly at the end of a time slice enter the queue
        // before the just-preempted job is put back.
        enqueue_arrived_jobs();

        if (remaining_time[current] == 0) {
            results[current].completion_time = now;
            ++finished;
        } else {
            ready.push(current);
        }
    }

    return finish_result(
        "Round Robin",
        "Preemptive. Each ready process gets at most quantum=" + std::to_string(quantum) +
            " time units per turn. Arrivals at a time-slice boundary are queued before the preempted process.",
        std::move(timeline),
        std::move(results));
}

} // namespace schedsim
