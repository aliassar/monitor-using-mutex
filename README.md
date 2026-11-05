# Monitor Using Mutex

A small C++11 traffic simulation that implements the monitor concurrency pattern as a generic wrapper around a mutex, with one thread per car.

Operating Systems course assignment (the CMake project is named `Os_3`), written in late 2019.

## What it does

The program reads a road network and a set of paths from `test.txt`. Every car is a thread that drives its path road by road. Driving a road is a CPU-heavy calculation that produces a "pollution" number, and cars on the same road have to take turns.

Input format (`test.txt`):

```
A - B - 4        <- road "A - B" with weight 4 (one digit)
B - C - 1
...
#
A - B - C - D    <- a path: the roads A-B, B-C, C-D in order
3                <- number of cars that take that path
E - B - C - F
4
```

## How it works

- `monitor<T>` (in `main.cpp` and, as a standalone copy, `monitor.cpp`) owns an object of type `T` and a `std::mutex`. Its `operator->` returns a temporary `monitor_helper` that locks a `unique_lock` on construction and exposes `T*`. The temporary lives until the end of the full expression, so `road->calculation()` runs with the lock held and the lock is released automatically afterwards. `ManuallyLock()` returns the same helper for longer critical sections, and `GetThreadUnsafeAccess()` bypasses the lock (used only while parsing the input, before any thread starts).
- Each `road` is stored as a `monitor<road>`, so only one car at a time can be inside a given road's calculation. Cars on different roads run in parallel.
- After a road is finished, the car adds its result to the global `totalPollution`. That update is guarded by a separate POSIX semaphore (`sumProtector`, initialised to 1) used as a binary lock.
- For every car and road, the program writes a file named `<pathIndex>-<carIndex>` containing: start road, start time, end road, end time, the pollution of that road, and the running total. The file is opened fresh for each road, so after a run it holds the record of the last road the car drove. The `0-0` ... `1-3` files in the repo are output from an earlier run; for example `0-0` ends with a running total of `27000000`.
- When all threads are joined, the final total is printed.

`monitor.cpp` is only a scratch file: the same `monitor` template plus an unused `pushing()` function and a single-threaded loop.

## Tech

C++11, `std::thread`, `std::mutex`, POSIX semaphores.

## Run it

Needs Linux (or WSL), because it uses unnamed POSIX semaphores (`sem_init`), which macOS does not support.

```
g++ -std=c++11 -pthread main.cpp -o Os_3
./Os_3          # reads test.txt from the current directory
```

`CMakeLists.txt` defines the same two targets (`Os_3` from `main.cpp`, `monitor` from `monitor.cpp`) but does not link the threading library, so the direct `g++` command above is the simpler route.
