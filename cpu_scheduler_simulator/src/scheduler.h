#pragma once

#include <string>
#include <vector>

namespace schedsim {

struct Process {
    std::string id;
    int arrival_time = 0;
    int burst_time = 0;
    int priority = 0;   // Smaller number means higher priority.
    int deadline = 0;
};

struct TimelineSegment {
    int start_time = 0;
    int end_time = 0;
    std::string label;
};

struct ProcessResult {
    Process process;
    int first_start_time = -1;
    int completion_time = -1;
    int waiting_time = 0;
    int turnaround_time = 0;
    int response_time = 0;
    bool missed_deadline = false;
};

struct SimulationResult {
    std::string policy_name;
    std::string description;
    std::vector<TimelineSegment> timeline;
    std::vector<ProcessResult> processes;

    int makespan = 0;
    int total_busy_time = 0;
    int deadline_misses = 0;

    double average_waiting_time = 0.0;
    double average_turnaround_time = 0.0;
    double average_response_time = 0.0;
    double cpu_utilization_percent = 0.0;
    double throughput = 0.0;
};

class Scheduler {
public:
    static SimulationResult run_fcfs(const std::vector<Process>& processes);
    static SimulationResult run_sjf(const std::vector<Process>& processes);
    static SimulationResult run_priority(const std::vector<Process>& processes);
    static SimulationResult run_round_robin(const std::vector<Process>& processes, int quantum);
};

} // namespace schedsim
