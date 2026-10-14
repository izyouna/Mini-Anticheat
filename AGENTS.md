
# AGENTS.md

## 1. Project Overview

Project Name: Mini Anti-Cheat
Full Name: Mini Anti-Cheat: Process-Based Cheat Detection System

Mini Anti-Cheat is an educational Linux system-level software
project developed in C.

The project demonstrates operating system concepts through
process monitoring, process management, inter-process
communication (IPC), and rule-based cheat detection.

The system will monitor Linux processes, identify suspicious
processes, and detect simulated cheating behavior in a
controlled game environment.

This is a university mini project for the course:
Operating Systems and System Calls.

## 2. Technology Stack

- Programming Language: C (C11)
- Operating System: Ubuntu Linux
- Compiler: GCC
- Build System: GNU Make
- Interface: CLI (Terminal)
- Version Control: Git
- Repository Hosting: GitHub

Supported development environments:

- Ubuntu Linux
- Ubuntu VM on macOS (UTM)
- Ubuntu WSL2 on Windows

The application targets Linux and uses the Linux /proc
filesystem. Native macOS and Windows execution are not
required.

## 3. Project Goals

The main objectives are:

1. Understand Linux process management.
2. Practice POSIX system calls and APIs.
3. Monitor active Linux processes.
4. Read process information from /proc.
5. Implement rule-based suspicious process detection.
6. Simulate cheating behavior in a controlled environment.
7. Demonstrate inter-process communication.
8. Log detection events.
9. Demonstrate safe process termination.
10. Apply modular C programming practices.

## 4. Project Structure

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

This structure represents the intended architecture.
Some modules may not exist during early development.

Do not assume planned features are already implemented.

## 5. Development Rules

All AI coding agents must follow these rules.

### Language

- Use C11.
- Prefer POSIX-compatible APIs.
- Use Linux-specific interfaces only when necessary.
- Do not introduce C++, Python, Java, or JavaScript
  into the core application without approval.
- Keep the core application CLI-based.

### Code Quality

- Write clean and readable C code.
- Use meaningful variable and function names.
- Separate functionality into modules.
- Keep functions focused on a single responsibility.
- Add comments for complex logic.
- Avoid unnecessary dependencies.
- Avoid global mutable state when possible.

### Memory Safety

- Check malloc(), calloc(), and realloc().
- Free dynamically allocated memory.
- Avoid buffer overflows.
- Use bounded string operations.
- Validate user input.
- Check array bounds.
- Initialize structures before use.

### Error Handling

Always check relevant system call return values.

Handle errors from:

- fork()
- execvp()
- waitpid()
- pipe()
- open()
- read()
- write()
- close()
- kill()
- opendir()

Use perror() or appropriate error messages.

Handle processes disappearing during /proc scanning.

Permission errors must not crash the scanner.

## 6. Operating System APIs

The project should demonstrate the following APIs
where relevant.

### Process Management

- fork()
- execvp()
- waitpid()
- getpid()
- getppid()
- kill()

### File Operations

- open()
- read()
- write()
- close()

### Directory Operations

- opendir()
- readdir()
- closedir()

### IPC

- pipe()
- dup2()

### Linux Process Information

- /proc
- /proc/[pid]/status
- /proc/[pid]/cmdline
- /proc/[pid]/exe

Note:

The /proc filesystem is Linux-specific.

Not every library function listed here is itself a
kernel system call.

## 7. Core Modules

### Process Scanner

Responsibilities:

- Enumerate numeric PID directories under /proc.
- Read process information.
- Extract PID, PPID, process name, and state.
- Store process information in structures.
- Handle inaccessible or terminated processes.
- Return process information to other modules.

### Cheat Detector

Responsibilities:

- Analyze scanned processes.
- Compare process information against detection rules.
- Identify suspicious test processes.
- Distinguish suspicious from confirmed cheating.
- Report detection reasons.

Do not assume a process is cheating solely because
its name matches a suspicious string.

### Game Simulator

Responsibilities:

- Simulate a simple game.
- Maintain game state such as health and score.
- Generate legitimate game events.
- Expose controlled test events for detection.

### Cheat Simulator

Responsibilities:

- Simulate cheating behavior only within
  the educational game environment.
- Generate predefined suspicious events.
- Support repeatable testing.

Do not modify external games or unrelated processes.

### Logger

Responsibilities:

- Record detection events.
- Store timestamps, PIDs, and detection reasons.
- Handle file errors.
- Keep logs readable.

### Game Launcher

Responsibilities:

- Launch the controlled game process.
- Track the launched child PID.
- Monitor child termination.
- Send termination signals only to processes
  owned and managed by the launcher.

## 8. Detection Policy

Use rule-based detection.

Possible detection signals:

- Known test-process executable identity.
- Suspicious process metadata.
- Unexpected game state modification events.
- Invalid game actions reported by the simulator.

Detection results should include:

- NORMAL
- SUSPICIOUS
- CONFIRMED_CHEAT

A suspicious result is not sufficient evidence
for automatic process termination.

Confirmed cheating should be based on controlled
simulation evidence.

## 9. Security and Safety Rules

This project is for educational use.

Agents must NOT:

- Develop kernel-level anti-cheat drivers.
- Modify unrelated process memory.
- Bypass real anti-cheat systems.
- Inject code into third-party applications.
- Disable operating system security controls.
- Terminate arbitrary system processes.
- Run destructive shell commands.
- Require root privileges without justification.

Process termination must be restricted to
controlled simulator processes.

Never automatically terminate unrelated processes
based only on their name or PID.

## 10. Development Workflow

Follow these phases:

Phase 1:
Process Scanner

Phase 2:
Cheat Detection Engine

Phase 3:
Game and Cheat Simulators

Phase 4:
Real-time Monitoring

Phase 5:
Logging and Game Protection

Phase 6:
Testing, Documentation, and Demo

Complete and test each phase before moving forward.

Do not implement unrelated features without approval.

## 11. Build Instructions

The project should use GNU Make.

Expected commands:

make

./anticheat

make clean

Recommended compiler flags:

-std=c11
-Wall
-Wextra
-Wpedantic
-O2

Do not ignore compiler warnings.

## 12. Testing Requirements

Tests should cover:

- Process enumeration.
- Process information parsing.
- Process termination during scanning.
- Permission errors.
- Empty detection results.
- Suspicious process detection.
- False positive scenarios.
- Simulator event detection.
- Logging failures.
- Controlled game termination.

Testing must not disrupt system processes.

## 13. Git Workflow

The project is developed collaboratively.

Recommended branches:

- main
- feature/scanner
- feature/detection
- feature/game
- feature/logging

Rules:

1. Keep main stable.
2. Create branches for new features.
3. Write meaningful commit messages.
4. Test before committing.
5. Use Pull Requests for integration.
6. Resolve merge conflicts carefully.
7. Do not commit generated binaries.
8. Do not commit secrets or credentials.

## 14. AI Agent Instructions

Before changing code:

1. Inspect the existing project structure.
2. Read README.md.
3. Understand the current implementation.
4. Identify the requested development phase.
5. Preserve working functionality.

When implementing code:

1. Make small, focused changes.
2. Explain important design decisions.
3. Follow the existing architecture.
4. Avoid unnecessary refactoring.
5. Update headers when interfaces change.
6. Add or update tests when appropriate.
7. Keep the build system working.

After implementation:

1. Compile the project.
2. Run available tests.
3. Check for compiler warnings.
4. Summarize modified files.
5. Explain how to test the changes.
6. Report known limitations.

Never claim that code was compiled or tested
unless the corresponding command was executed.

## 15. Project Scope

This project is NOT intended to be a production
anti-cheat solution.

It is an educational demonstration of:

- Linux process monitoring
- POSIX APIs
- Process lifecycle
- IPC
- Rule-based detection
- Logging
- Controlled process management

Prioritize correctness, safety, and explainability
over complex features.

## 16. Definition of Done

A feature is complete when:

- The implementation compiles.
- Relevant functionality works as expected.
- Errors are handled.
- Tests pass where available.
- Documentation is updated.
- No unrelated functionality is broken.

The final project must be suitable for a live
university demonstration.
