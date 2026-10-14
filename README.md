
# Mini Anti-Cheat

### Process-Based Cheat Detection System

An educational Linux anti-cheat simulation developed
in C using POSIX APIs and Linux process information.

**Course:** Operating Systems and System Calls

**Platform:** Ubuntu Linux

**Language:** C

**Interface:** Command Line Interface (CLI)

**Status:** Early Development

---

## Overview

Mini Anti-Cheat is a system-level software project
designed to demonstrate operating system concepts
through process monitoring and simulated cheat detection.

The system aims to detect suspicious processes
and simulated cheating behavior in a controlled
Linux game environment.

The project focuses on:

- Linux Process Management
- POSIX System Calls
- Process Monitoring
- Inter-Process Communication (IPC)
- Rule-Based Detection
- Event Logging
- Controlled Process Termination

This project is intended for educational purposes
and is not a production anti-cheat system.

---

## Objectives

1. Develop a process scanner using C.
2. Understand Linux process information.
3. Practice POSIX system calls.
4. Identify suspicious processes using detection rules.
5. Build a controlled game simulator.
6. Simulate cheating behavior.
7. Implement detection alerts and logging.
8. Demonstrate safe process management.

---

## System Architecture

The planned architecture is:

```text
          +----------------------+
          |     Game Launcher    |
          +----------+-----------+
                     |
                     v
          +----------------------+
          |   Anti-Cheat Engine  |
          +----------+-----------+
                     |
          +----------+-----------+
          |                      |
          v                      v
+------------------+  +------------------+
| Process Scanner  |  | Detection Engine |
+--------+---------+  +---------+--------+
         |                      |
         v                      v
+------------------+  +------------------+
| Linux /proc      |  | Detection Rules  |
+------------------+  +------------------+
                                |
                                v
                      +------------------+
                      | Alert & Logging  |
                      +------------------+
```

The scanner collects process information.

The detection engine evaluates suspicious activity.

The game simulator provides a controlled environment
for testing detection behavior.

---

## Features

### Phase 1: Process Scanner

Initial implementation target:

- Scan active Linux processes.
- Read process information from /proc.
- Display Process ID (PID).
- Display Parent Process ID (PPID).
- Display process names.
- Display process states.
- Handle processes that terminate during scanning.

### Phase 2: Cheat Detection

Planned features:

- Rule-based suspicious process detection.
- Detection status classification.
- Detection reason reporting.
- Test-process identification.

### Phase 3: Game Simulation

Planned features:

- Simple CLI game simulator.
- Health and score management.
- Controlled cheat simulation.
- Game event reporting.

### Phase 4: Real-time Monitoring

Planned features:

- Periodic process scanning.
- New process detection.
- Detection alerts.

### Phase 5: Logging and Protection

Planned features:

- Detection event logs.
- Process inspection.
- Controlled game termination.
- Detection history.

---

## Technology Stack

| Component | Technology |
|-----------|------------|
| Language | C11 |
| OS | Ubuntu Linux |
| Compiler | GCC |
| Build Tool | GNU Make |
| Interface | CLI |
| Version Control | Git |
| Repository | GitHub |
| Process Information | Linux /proc |

---

## Supported Environments

The project targets Ubuntu Linux.

Development environments include:

- Native Ubuntu Linux
- Ubuntu VM on macOS
- Ubuntu WSL2 on Windows

For Apple Silicon Macs, Ubuntu ARM64 is recommended.

For Intel Macs and typical Windows PCs,
Ubuntu AMD64 may be used.

The same C source code is intended to work
on both Linux architectures.

Native Windows and macOS execution are not supported.

---

## Requirements

Before building the project, install:

- GCC
- GNU Make
- Git
- Ubuntu Linux

Install dependencies:

```bash
sudo apt update
sudo apt install build-essential git
```

Verify installation:

```bash
gcc --version
make --version
git --version
```

---

## Installation

Clone the repository:

```bash
git clone https://github.com/USERNAME/mini-anticheat.git
```

Replace USERNAME with the repository owner's username.

Navigate to the project:

```bash
cd mini-anticheat
```

Build:

```bash
make
```

---

## Usage

The initial Process Scanner is intended to run with:

```bash
./anticheat
```

Expected example output:

```text
====================================
       MINI ANTI-CHEAT v0.1
         PROCESS SCANNER
====================================

PID      PPID     NAME          STATE
----------------------------------------
1        0        systemd       S
1042     1        NetworkManager S
1890     1700     bash          S
2050     1890     anticheat     R

Processes scanned: 142
Processes displayed: 142
```

Actual output depends on the running Linux system.

To display only zombie processes:

```bash
./anticheat --zombies
```

To compare the scanner with Linux process information:

```bash
ps -eo pid,ppid,comm,state
```

The detection and game simulation commands will
be documented when those features are implemented.

---

## Project Structure

The intended project structure is:

```text
mini-anticheat/
|
|-- AGENTS.md
|-- README.md
|-- Makefile
|
|-- src/
|   |-- main.c
|   |-- process_scanner.c
|   |-- cheat_detector.c
|   |-- game_launcher.c
|   |-- logger.c
|
|-- include/
|   |-- process_scanner.h
|   |-- cheat_detector.h
|   |-- game_launcher.h
|   |-- logger.h
|
|-- simulator/
|   |-- game.c
|   |-- cheat_test.c
|
|-- tests/
|
|-- logs/
|
|-- docs/
|
|-- .gitignore
```

Some files will be added during later phases.

---

## POSIX APIs and Linux Interfaces

The project is designed to demonstrate several
operating system interfaces.

| API | Purpose |
|-----|---------|
| fork() | Create child processes |
| execvp() | Execute programs |
| waitpid() | Wait for child processes |
| kill() | Send process signals |
| pipe() | Create IPC channels |
| dup2() | Redirect file descriptors |
| open() | Open files |
| read() | Read process information |
| write() | Write logs and IPC data |
| close() | Close file descriptors |
| opendir() | Open process directories |
| readdir() | Enumerate directories |
| closedir() | Close directories |

The project also uses the Linux-specific /proc
filesystem for process information.

Some APIs listed above are library interfaces
rather than direct kernel system calls.

---

## Detection Approach

The planned detection system uses rule-based
analysis.

Detection results include:

NORMAL

The process does not match the current
suspicious-process rules.

SUSPICIOUS

The process matches a detection rule but
cheating has not been confirmed.

CONFIRMED_CHEAT

Controlled simulator evidence confirms a
predefined cheating event.

The system must not terminate unrelated
processes based only on a suspicious name.

---

## Development Roadmap

- [x] Phase 1: Process Scanner
- [x] Phase 2: Cheat Detection Engine
- [x] Phase 3: Game Simulator
- [x] Phase 4: Cheat Simulator
- [x] Phase 5: Real-time Monitoring
- [x] Phase 6: Logging and Game Protection
- [x] Phase 7: Testing and Documentation

Update this checklist when features are completed
and tested.

---

## Testing

Compile the project:

```bash
make
```

Run all unit test suites (18 unit tests):

```bash
make test
```

Run automated end-to-end integration test suite:

```bash
./tests/run_tests.sh
```

Run the scanner:

```bash
./anticheat
```

Compare the results:

```bash
ps -eo pid,ppid,comm,state
```

Clean generated binaries:

```bash
make clean
```

For a comprehensive presentation script and architecture guide, see:
- [System Architecture & System Calls Guide](docs/ARCHITECTURE.md)
- [Live University Demonstration Script](docs/DEMO_GUIDE.md)
- [Task Implementation Checklist](task.md)

---

## Collaboration

This project is developed collaboratively
using Git and GitHub.

Recommended branches:

```text
main
feature/scanner
feature/detection
feature/game
feature/logging
```

Create a feature branch:

```bash
git switch -c feature/scanner
```

Stage changes:

```bash
git add .
```

Commit changes:

```bash
git commit -m "Implement process scanner"
```

Push the branch:

```bash
git push -u origin feature/scanner
```

Create a Pull Request on GitHub to merge
the feature into main.

Keep main stable and avoid directly pushing
unfinished features.

---

## Safety and Limitations

Mini Anti-Cheat is an educational simulation.

It does not provide production-grade
game cheat detection.

Limitations include:

- Linux-only process scanning.
- Rule-based detection.
- Limited visibility into protected processes.
- Possible false positives.
- No kernel-level monitoring.
- No detection of arbitrary real-world cheats.

The project does not modify external game memory.

Process termination is limited to controlled
game simulator processes.

The application should not require root privileges
for normal operation.

---

## Future Improvements

Possible future improvements include:

- Configurable detection rules.
- More detailed process inspection.
- Process tree visualization.
- Additional IPC mechanisms.
- Detection statistics.
- Automated test cases.
- Improved false-positive handling.

These are optional and outside the initial scope.

---

## Academic Purpose

This project was created as a Mini Project for:

Operating Systems and System Calls

The goal is to demonstrate practical knowledge
of operating system concepts through C programming
and Linux system interfaces.

---

## License

License to be determined by the project team.
