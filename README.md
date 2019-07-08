# Micro Hierarchical State Machine (UHSM)
This is a C++ library implementing a hierarchical state machine (HSM) which:
* is header-only
* is 100% type-safe
* does not use dynamic memory allocation
* does not use exceptions

All of this makes it applicable in a real-time embedded system (bare-metal or using RTOS) based on a microcontroller.

## Features
* Nested states (of course)
* Transition actions
* OnEntry()/OnExit() functions
* Data in event and state types

### Upcoming
* Transition table validation (since the library already extensively processes the transition table in compile-time, it's easy to add compile-time checks for duplicates; see `utils::remove_duplicates` template which can be easily modified to detect duplicates instead of removing them)

## Implementation
The library utilizes the *curiously recurring template pattern* (CRTP) and template metaprogramming techniques. It takes inspiration from Boost Meta State Machine (Boost MSM) in terms of the way of defining the HSM. UHSM is, however, orders of magnitude simpler and does not allocate memory on the heap.

## Requirements
The library requires a C++ compiler with a support for C++17 standard. Tests are built using CMake and CppUTest is used as a test harness and provides mocking support.

## Usage in your project
UHSM is a header-only library. Just copy the topmost include folder to your project and add it to your include directories. CMake interface library target will be provided soon for convenience.

For examples on how to use the library, see [Tests](##Tests).

### Use with event queues
As UHSM is completely type-oriented, it might pose a problem to use it with event queues which can only hold events of a single type or perform type erasure (access via `void*` etc.). The following solution exists:
* if the number of events in reasonably small, a simple `switch` statement can be used
* use multiple queues, each holding events of a different type (only for small number of distinct event types)
* use a queue holding the sum type of all used events (like `std::variant`)
* a declarative framework can be written, which *restores* an event type based on a compile-time table which maps an event ID (known at runtime) to event type

## Tests
To build tests for the host machine, navigate to `test` subfolder and execute:
```bash
mkdir build
cd build
cmake ..
make
```
CppUTest will be automatically fetched from it's official repo, built and used as a part of test application. To execute tests, run (from `test/build` folder):
```bash
./uhsm_test
```
### State machine used for tests
A sample test machine used as a base for testing, models a simple music player with a power switch, 4 buttons (play/pause, stop, backward, forward) and a single LED. It's behaviour has been depicted on the following diagram:

![alt text](https://github.com/borysjelenski/uhsm/raw/develop/doc/test_state_machine_diag.png "")