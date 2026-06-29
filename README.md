# CPU Scheduler Simulator

A deterministic CPU scheduling simulator written in C++ to model and compare classic operating system scheduling policies such as **FCFS**, **SJF**, **Priority Scheduling**, and **Round Robin**.

The simulator takes process workloads from a CSV file, executes them according to the selected scheduling policy, generates compact Gantt-style execution traces, and reports detailed per-process and aggregate scheduling metrics.

---

## Overview

CPU scheduling is a core operating system concept where the scheduler decides which process gets CPU time and in what order. Different scheduling policies optimize for different goals such as fairness, throughput, responsiveness, or low average waiting time.

This project simulates multiple scheduling algorithms and helps visualize how scheduling choices affect:

* Process waiting time
* Turnaround time
* Response time
* Finish time
* Deadline misses
* CPU utilization
* Throughput
* Preemption behavior
* Idle CPU gaps
* Fairness and starvation

Although this is an operating systems project, the underlying ideas are also relevant to low-latency systems and trading infrastructure, where deterministic event ordering, queueing delay, priority handling, and latency deadlines matter.

---

## Features

* Deterministic CPU scheduling simulation
* Supports multiple scheduling policies:

  * First Come First Serve, FCFS
  * Shortest Job First, SJF
  * Priority Scheduling
  * Round Robin
* CSV workload parsing
* Input validation for:

  * Malformed rows
  * Duplicate process IDs
  * Invalid arrival time
  * Invalid burst time
  * Invalid priority
  * Invalid deadline
* Execution timeline generation
* Compact Gantt-style trace output
* Per-process metric calculation:

  * Waiting time
  * Turnaround time
  * Response time
  * Finish time
  * Deadline miss status
* Aggregate metric calculation:

  * Average waiting time
  * Average turnaround time
  * CPU utilization
  * Throughput
* Edge-case workload support:

  * Idle CPU gaps
  * Same arrival time
  * Same priority
  * Deadline misses
  * Round Robin quantum boundaries

---

## Scheduling Algorithms

### 1. First Come First Serve, FCFS

FCFS schedules processes in the order of their arrival time.

It is simple and deterministic, but can suffer from the **convoy effect**, where a long process delays many shorter processes behind it.

```text
Example:
P1: arrival=0, burst=5
P2: arrival=1, burst=2
P3: arrival=2, burst=1

FCFS Timeline:
[P1: 0-5] [P2: 5-7] [P3: 7-8]
```

---

### 2. Shortest Job First, SJF

SJF selects the process with the shortest burst time among all currently available processes.

It can reduce average waiting time, but long processes may starve if short processes keep arriving.

```text
Selection rule:
burst time -> arrival time -> process id
```

---

### 3. Priority Scheduling

Priority Scheduling selects the process with the highest priority among all available processes.

Depending on the implementation convention, either a lower priority value or a higher priority value can mean higher priority.

```text
Selection rule:
priority -> arrival time -> process id
```

This policy is useful when some processes are more critical than others, but it can cause starvation for low-priority processes.

---

### 4. Round Robin

Round Robin gives each process a fixed time quantum.

If a process does not finish within its quantum, it is preempted and placed back into the ready queue.

```text
Example with quantum = 2:

P1: arrival=0, burst=5
P2: arrival=1, burst=2
P3: arrival=2, burst=1

Round Robin Timeline:
[P1: 0-2] [P2: 2-4] [P3: 4-5] [P1: 5-7] [P1: 7-8]
```

Round Robin improves fairness and response time, but a very small quantum can increase context-switch overhead. A very large quantum makes Round Robin behave like FCFS.

---

## Input Format

The simulator reads process workloads from a CSV file.

Example:

```csv
pid,arrival_time,burst_time,priority,deadline
1,0,5,2,10
2,1,2,1,6
3,2,1,3,8
```

### Field Description

| Field          | Description                             |
| -------------- | --------------------------------------- |
| `pid`          | Unique process ID                       |
| `arrival_time` | Time at which process enters the system |
| `burst_time`   | Total CPU time required                 |
| `priority`     | Process priority                        |
| `deadline`     | Expected completion deadline            |

---

## Output

The simulator prints:

1. Execution timeline
2. Gantt-style trace
3. Per-process metrics
4. Aggregate scheduler metrics

Example output:

```text
Scheduling Policy: Round Robin
Quantum: 2

Execution Timeline:
[P1: 0-2] [P2: 2-4] [P3: 4-5] [P1: 5-7] [P1: 7-8]

Per-Process Metrics:
PID  Arrival  Burst  Finish  Waiting  Turnaround  Response  Deadline Miss
1    0        5      8       3        8           0         No
2    1        2      4       1        3           1         No
3    2        1      5       2        3           2         No

Aggregate Metrics:
Average Waiting Time     : 2.00
Average Turnaround Time  : 4.67
CPU Utilization          : 100.00%
Throughput               : 0.375 processes/unit time
```

---

## Metric Formulas

### Finish Time

Time at which the process completes execution.

```text
Finish Time = completion timestamp
```

### Turnaround Time

Total time spent by the process in the system.

```text
Turnaround Time = Finish Time - Arrival Time
```

### Waiting Time

Total time spent waiting in the ready queue.

```text
Waiting Time = Turnaround Time - Burst Time
```

### Response Time

Time from process arrival until it gets CPU for the first time.

```text
Response Time = First Start Time - Arrival Time
```

### Deadline Miss

A process misses its deadline if it finishes after its deadline.

```text
Deadline Miss = Finish Time > Deadline
```

### CPU Utilization

Percentage of total simulation time during which CPU was busy.

```text
CPU Utilization = Busy Time / Total Simulation Time
```

### Throughput

Number of processes completed per unit time.

```text
Throughput = Completed Processes / Total Simulation Time
```

---

## Core Design

The simulator is implemented as an event-driven simulation engine.

At each step:

```text
1. Add all processes whose arrival time is less than or equal to current time.
2. If the ready queue is empty, record an IDLE segment and jump to the next arrival time.
3. Select the next process according to the scheduling policy.
4. Run the process fully or for one quantum depending on the policy.
5. Record the execution segment.
6. Update remaining time, first start time, and finish time.
7. Repeat until all processes complete.
```

This avoids inefficient tick-by-tick simulation and correctly handles idle CPU gaps.

---

## Example Data Structures

```cpp
struct Process {
    int pid;
    int arrivalTime;
    int burstTime;
    int priority;
    int deadline;

    int remainingTime;
    int firstStartTime = -1;
    int finishTime = -1;
};
```

```cpp
struct Segment {
    int start;
    int end;
    int pid;   // -1 represents IDLE
};
```

---

## Data Structures Used

| Component                | Data Structure                               |
| ------------------------ | -------------------------------------------- |
| Process storage          | `std::vector<Process>`                       |
| FCFS ready queue         | `std::queue<int>`                            |
| Round Robin ready queue  | `std::queue<int>`                            |
| SJF selection            | `std::priority_queue` with custom comparator |
| Priority selection       | `std::priority_queue` with custom comparator |
| Timeline                 | `std::vector<Segment>`                       |
| Duplicate PID validation | `std::unordered_set<int>`                    |

---

## Deterministic Tie-Breaking

The simulator uses fixed tie-breaking rules so that the same input always produces the same output.

Examples:

```text
FCFS:
arrival time -> process id

SJF:
burst time -> arrival time -> process id

Priority Scheduling:
priority -> arrival time -> process id

Round Robin:
arrival order -> ready queue order
```

Determinism makes the simulator easier to test, debug, and reason about.

---

## Edge Cases Covered

* Empty workload
* Single process workload
* CPU idle before first process arrival
* Idle gaps between processes
* Multiple processes arriving at the same time
* Same burst time in SJF
* Same priority in Priority Scheduling
* Round Robin quantum smaller than burst time
* Round Robin quantum larger than burst time
* Process arriving exactly at quantum boundary
* Duplicate process IDs
* Malformed CSV rows
* Invalid process fields
* Deadline miss scenarios

---

## Build Instructions

### Using g++

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic src/*.cpp -o scheduler_sim
```

### Run

```bash
./scheduler_sim --input workloads/sample.csv --policy rr --quantum 2
```

Example policies:

```bash
./scheduler_sim --input workloads/sample.csv --policy fcfs
./scheduler_sim --input workloads/sample.csv --policy sjf
./scheduler_sim --input workloads/sample.csv --policy priority
./scheduler_sim --input workloads/sample.csv --policy rr --quantum 3
```

---

## Suggested Project Structure

```text
cpu-scheduler-simulator/
├── README.md
├── src/
│   ├── main.cpp
│   ├── Process.hpp
│   ├── Scheduler.hpp
│   ├── FCFSScheduler.cpp
│   ├── SJFScheduler.cpp
│   ├── PriorityScheduler.cpp
│   ├── RoundRobinScheduler.cpp
│   ├── CsvParser.cpp
│   └── Metrics.cpp
├── include/
│   └── common.hpp
├── workloads/
│   ├── sample.csv
│   ├── idle_gap.csv
│   ├── same_arrival.csv
│   ├── deadline_miss.csv
│   └── round_robin_quantum.csv
└── tests/
    └── scheduler_tests.cpp
```

---

## Complexity Analysis

### FCFS

```text
Time Complexity  : O(n log n) if input sorting is required, otherwise O(n)
Space Complexity : O(n)
```

### SJF

```text
Time Complexity  : O(n log n)
Space Complexity : O(n)
```

### Priority Scheduling

```text
Time Complexity  : O(n log n)
Space Complexity : O(n)
```

### Round Robin

```text
Time Complexity  : O(number of time slices + n log n)
Space Complexity : O(n + timeline segments)
```

For Round Robin, the number of time slices depends on the total burst time and quantum size.

```text
Smaller quantum -> more slices -> more preemptions
Larger quantum  -> fewer slices -> closer to FCFS behavior
```

---

## Testing Strategy

The simulator is tested using small, hand-verifiable workloads.

Test cases include:

* Basic FCFS ordering
* SJF shortest burst selection
* Priority-based selection
* Round Robin preemption
* Idle CPU gaps
* Same arrival time tie-breaking
* Deadline misses
* Invalid CSV rows
* Duplicate process IDs
* Quantum boundary behavior

Golden-output testing can be used by comparing expected timelines and metrics against simulator output.

---

## Relevance to Low-Latency Systems

This project is not only about OS scheduling theory. It also models concepts that are important in low-latency and trading systems:

| Scheduler Concept | Low-Latency / Trading Analogy |
| ----------------- | ----------------------------- |
| Arrival time      | Event arrival timestamp       |
| Burst time        | Processing cost               |
| Priority          | Business criticality          |
| Deadline          | Latency SLA                   |
| Waiting time      | Queueing delay                |
| Response time     | First service latency         |
| Turnaround time   | End-to-end latency            |
| Timeline          | Replay/debug trace            |
| Deadline miss     | SLA violation                 |
| Determinism       | Reproducible event replay     |

Examples of high-priority trading events:

* Kill switch
* Cancel-all request
* Risk breach
* Exchange disconnect
* Market data gap recovery
* Order reject
* Order acknowledgment

Lower-priority tasks may include:

* Logging
* Metrics aggregation
* Debug dumps
* Background cleanup
* Offline analytics

This makes the simulator useful for reasoning about queueing, fairness, starvation, priority handling, and deterministic replay.

---

## Future Improvements

Possible extensions:

* Preemptive SJF / Shortest Remaining Time First, SRTF
* Preemptive Priority Scheduling
* Aging to prevent starvation
* Priority inheritance for priority inversion scenarios
* Context-switch overhead modeling
* Multi-core scheduling
* CPU affinity support
* Cache locality-aware scheduling
* Real-time scheduling policies
* Workload generator
* JSON output for metrics
* Visualization dashboard for timelines

---

## Key Learnings

This project helped build a deeper understanding of:

* CPU scheduling algorithms
* Preemptive vs non-preemptive scheduling
* Ready queues and priority queues
* Event-driven simulation
* Deterministic tie-breaking
* Gantt-style execution tracing
* Scheduling metrics
* Starvation and fairness
* Input validation
* C++ data structure design
* Testing edge cases in systems code

---
---

## License

This project is open-source and available under the MIT License.
