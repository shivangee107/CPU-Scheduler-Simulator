# CPU Scheduler Simulator

A clean C++17 project that simulates classic CPU scheduling policies and compares their behavior on the same workload.

Implemented policies:

1. FCFS - First Come First Served
2. SJF - Shortest Job First, non-preemptive
3. Priority Scheduling, non-preemptive
4. Round Robin, preemptive

Metrics shown:

- Execution timeline
- Compact Gantt view
- First start time
- Completion time
- Waiting time
- Turnaround time
- Response time
- Deadline miss count
- Average waiting time
- Average turnaround time
- Average response time
- CPU utilization
- Throughput

## Policy assumptions

- Time values are integer simulation units. They can represent milliseconds, ticks, or arbitrary units.
- Smaller priority number means higher priority.
- A process misses its deadline if `completion_time > deadline`.
- SJF and Priority Scheduling are non-preemptive.
- Round Robin is preemptive and uses a configurable time quantum.
- Ties are deterministic and use arrival time, then input order.

## Project structure

```text
cpu_scheduler_simulator/
  CMakeLists.txt
  README.md
  data/
    README.md
    sample_processes.csv
    valid/
      01_sample_mixed_workload.csv
      02_idle_cpu_gaps.csv
      ...
    invalid/
      01_empty_file.csv
      02_negative_arrival.csv
      ...
  scripts/
    run_valid_cases.sh
    run_invalid_cases.sh
  src/
    main.cpp
    scheduler.h
    scheduler.cpp
```

## Build with CMake

From the project root:

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

Run with the built-in sample workload:

```bash
./cpu_scheduler_simulator
```

Run using the sample CSV file:

```bash
./cpu_scheduler_simulator --input ../data/sample_processes.csv
```

Run only one policy:

```bash
./cpu_scheduler_simulator --policy fcfs
./cpu_scheduler_simulator --policy sjf
./cpu_scheduler_simulator --policy priority
./cpu_scheduler_simulator --policy rr --quantum 2
```

## Build without CMake

From the project root:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic src/main.cpp src/scheduler.cpp -o cpu_scheduler_simulator
./cpu_scheduler_simulator
```


## Test data suite

The `data/` folder contains named CSV workloads that intentionally cover normal cases and edge cases.

Valid cases:

| File | What it covers |
|---|---|
| `data/valid/01_sample_mixed_workload.csv` | Main mixed workload used for demo output. |
| `data/valid/02_idle_cpu_gaps.csv` | CPU idle at the beginning and between jobs. |
| `data/valid/03_all_arrive_same_time_tie_breaking.csv` | All jobs arrive at time `0`; deterministic tie-breaking. |
| `data/valid/04_sjf_equal_burst_ties.csv` | SJF equal-burst tie handling. |
| `data/valid/05_priority_equal_priority_ties.csv` | Priority scheduling tie handling. |
| `data/valid/06_deadline_boundary_cases.csv` | `completion_time == deadline` is not a miss; `completion_time > deadline` is a miss. |
| `data/valid/07_round_robin_quantum_boundary.csv` | Arrivals exactly at Round Robin quantum boundaries. |
| `data/valid/08_single_process.csv` | Minimal valid case with a single process. |
| `data/valid/09_unsorted_input_arrivals.csv` | Input rows are intentionally not sorted by arrival time. |
| `data/valid/10_round_robin_quantum_larger_than_bursts.csv` | RR quantum larger than most bursts. |
| `data/valid/11_comments_and_whitespace.csv` | CSV comments, blank lines, and spaces around fields. |

Invalid cases:

| File | Expected behavior |
|---|---|
| `data/invalid/01_empty_file.csv` | Reject because no process rows exist. |
| `data/invalid/02_negative_arrival.csv` | Reject negative arrival time. |
| `data/invalid/03_zero_burst.csv` | Reject non-positive burst time. |
| `data/invalid/04_missing_field.csv` | Reject malformed row with missing field. |
| `data/invalid/05_bad_integer.csv` | Reject non-integer numeric field. |
| `data/invalid/06_duplicate_process_id.csv` | Reject duplicate process IDs. |
| `data/invalid/07_negative_priority.csv` | Reject negative priority. |

## Commands to run tests

From the project root, build first:

```bash
cmake -S . -B build
cmake --build build
```

Run all valid workloads and save outputs under `outputs/`:

```bash
./scripts/run_valid_cases.sh
```

Run all invalid workloads and verify that each one fails with a clear error:

```bash
./scripts/run_invalid_cases.sh
```

Run a specific workload manually:

```bash
./build/cpu_scheduler_simulator --input data/valid/02_idle_cpu_gaps.csv
./build/cpu_scheduler_simulator --input data/valid/07_round_robin_quantum_boundary.csv --policy rr --quantum 3
```

You can also run the same workload with different Round Robin quantums:

```bash
./build/cpu_scheduler_simulator --input data/valid/07_round_robin_quantum_boundary.csv --policy rr --quantum 1
./build/cpu_scheduler_simulator --input data/valid/07_round_robin_quantum_boundary.csv --policy rr --quantum 2
./build/cpu_scheduler_simulator --input data/valid/07_round_robin_quantum_boundary.csv --policy rr --quantum 5
```

## Input CSV format

```csv
id,arrival,burst,priority,deadline
P1,0,8,2,18
P2,1,4,1,10
P3,2,9,3,25
P4,3,5,2,16
P5,6,2,1,12
```

Fields:

- `id`: process identifier
- `arrival`: time at which the process becomes ready
- `burst`: CPU time required by the process
- `priority`: smaller number means higher priority
- `deadline`: process deadline

## Example commands for testing

```bash
# All policies with default quantum = 3
./cpu_scheduler_simulator

# Round Robin with smaller quantum
./cpu_scheduler_simulator --policy rr --quantum 2

# All policies with custom workload
./cpu_scheduler_simulator --input ../data/sample_processes.csv --quantum 4
```
