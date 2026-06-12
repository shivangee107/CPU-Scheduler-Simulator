# Scheduler Simulator Test Data

The CSV files are split into valid and invalid cases.

## Valid cases

| File | What it tests |
|---|---|
| `valid/01_sample_mixed_workload.csv` | Main demo workload with mixed arrivals, priorities, bursts, and deadlines. |
| `valid/02_idle_cpu_gaps.csv` | CPU idle at the beginning and between jobs; CPU utilization should be below 100%. |
| `valid/03_all_arrive_same_time_tie_breaking.csv` | All processes arrive at time 0; checks deterministic tie-breaking. |
| `valid/04_sjf_equal_burst_ties.csv` | SJF tie case when multiple jobs have equal burst time. |
| `valid/05_priority_equal_priority_ties.csv` | Priority tie case when multiple jobs have the same priority. |
| `valid/06_deadline_boundary_cases.csv` | Deadline edge case: finishing exactly at deadline is not a miss; finishing after deadline is a miss. |
| `valid/07_round_robin_quantum_boundary.csv` | Round Robin arrivals exactly at quantum boundaries. |
| `valid/08_single_process.csv` | Minimal valid workload with one process and initial idle time. |
| `valid/09_unsorted_input_arrivals.csv` | Input rows are not sorted by arrival time; scheduler should still behave correctly. |
| `valid/10_round_robin_quantum_larger_than_bursts.csv` | RR quantum is larger than most bursts; RR should become close to FCFS behavior. |
| `valid/11_comments_and_whitespace.csv` | CSV comments, blank lines, and spaces around fields. |

## Invalid cases

| File | Expected behavior |
|---|---|
| `invalid/01_empty_file.csv` | Reject because no processes are present. |
| `invalid/02_negative_arrival.csv` | Reject because arrival time is negative. |
| `invalid/03_zero_burst.csv` | Reject because burst time must be positive. |
| `invalid/04_missing_field.csv` | Reject because the row does not have exactly five fields. |
| `invalid/05_bad_integer.csv` | Reject because `burst` is not an integer. |
| `invalid/06_duplicate_process_id.csv` | Reject because process IDs must be unique. |
| `invalid/07_negative_priority.csv` | Reject because priority cannot be negative. |
