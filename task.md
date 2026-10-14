# Mini Anti-Cheat: Task Tracking & Implementation Roadmap

เอกสารติดตามความคืบหน้าของงาน (Task Tracking) สำหรับโครงการ **Mini Anti-Cheat: Process-Based Cheat Detection System**  
อ้างอิงข้อกำหนดตาม [README.md](README.md) และ [AGENTS.md](AGENTS.md)

---

## 📌 สถานะปัจจุบัน (Current Status)

- **Active Branch:** `main` (Merged `feature/game`)
- **Current Phase:** Phase 3 Completed / Ready for Phase 4
- **Build Status:** Passing (`gcc -std=c11 -Wall -Wextra -Wpedantic -O2`)
- **Test Status:** 12/12 Unit & Simulator Tests Passing (Scanner: 4, Detector: 5, Simulator: 3)

---

## 📋 แผนงานและรายการงาน (Task Checklist)

### Phase 0: Project Scaffolding & Build System
- [x] **Task 0.1:** สร้างโครงสร้างไดเรกทอรีโครงการ (`src/`, `include/`, `simulator/`, `tests/`, `logs/`, `docs/`)
- [x] **Task 0.2:** เขียน `Makefile` รองรับ target: `all`, `anticheat`, `test`, `clean` พร้อมคอมไพเลอร์แฟล็ก `-std=c11 -Wall -Wextra -Wpedantic -O2`
- [x] **Task 0.3:** กำหนด `.gitignore` เพื่อป้องกันการ commit binary files, object files, และ log files

### Phase 1: Process Scanner (Core /proc Reader)
> **Branch:** `feature/scanner` | **Status:** ✅ เสร็จสิ้น
- [x] **Task 1.1:** ออกแบบ Data Structures ใน `include/process_scanner.h`:
  - `ProcessInfo` (pid, ppid, name, state)
  - `ProcessList` (dynamic array พร้อมฟังก์ชัน init และ free)
- [x] **Task 1.2:** พัฒนาตัวอ่านและแยกวิเคราะห์ `/proc/[pid]/status` ใน `src/process_scanner.c`:
  - ฟังก์ชัน `parse_proc_status()` ดึง Name, State, Pid, PPid
  - จัดการ Error Handling ปลอดภัยเมื่อ process หายระหว่างสแกน (`ENOENT`) หรือติดสิทธิ์ (`EACCES`)
- [x] **Task 1.3:** พัฒนาฟังก์ชันสแกน `scan_processes()` กวาดอ่านเฉพาะ numeric PID directories ใต้ `/proc`
- [x] **Task 1.4:** พัฒนาฟังก์ชันจัดเรียงและกรองข้อมูล:
  - `filter_processes_by_state()` (เช่น กรองสถานะ Zombie `Z`)
  - `process_list_sort_by_pid()` จัดเรียงตาม PID เพื่อความสะดวกในการตรวจสอบ
- [x] **Task 1.5:** พัฒนา CLI Interface ใน `src/main.c`:
  - แสดงตารางผลลัพธ์ PID, PPID, NAME, STATE
  - รองรับ Option `--zombies` กรองแสดงเฉพาะ Zombie process
  - รองรับ Option `--proc-dir <dir>` สำหรับการทดสอบด้วย Mock directory
  - แจ้งเตือนอย่างถูกต้องเมื่อรันบนระบบที่ไม่มี `/proc` (เช่น macOS host)
- [x] **Task 1.6:** เขียนชุดทดสอบ Unit Test (`tests/test_scanner.c`) ครอบคลุม Mock proc scanning, filtering, sorting และ memory cleanup

---

### Phase 2: Cheat Detection Engine (Rule-based Analysis)
> **Branch:** `feature/detection` | **Status:** ✅ เสร็จสิ้น
- [x] **Task 2.1:** กำหนด Enum และ Data Structures ใน `include/cheat_detector.h`:
  - Detection States: `DETECTION_NORMAL`, `DETECTION_SUSPICIOUS`, `DETECTION_CONFIRMED_CHEAT`
  - `DetectionResult` เก็บ status, pid, process name, reason string
- [x] **Task 2.2:** สร้าง Rule Engine ใน `src/cheat_detector.c`:
  - กฎตรวจสอบชื่อ executable ที่น่าสงสัย (Known cheat / debugger / injector signatures: gdb, lldb, scanmem, strace, cheatengine, ฯลฯ)
  - กฎแยกระหว่าง "น่าสงสัย" (SUSPICIOUS) กับ "ยืนยันการโกง" (CONFIRMED_CHEAT)
- [x] **Task 2.3:** พัฒนาฟังก์ชันประเมินโพรเซส `evaluate_process()` และ `detect_cheats()`
- [x] **Task 2.4:** เชื่อมต่อ Detection Engine เข้ากับ CLI ใน `src/main.c` (เพิ่ม option `--detect` และ `--alerts-only`)
- [x] **Task 2.5:** เขียน Unit Tests สำหรับ Detection Engine ใน `tests/test_detector.c` ครอบคลุม Normal, Suspicious, Confirmed Cheat และ Filtering

---

### Phase 3: Game & Cheat Simulators
> **Branch:** `feature/game` | **Status:** ✅ เสร็จสิ้น
- [x] **Task 3.1:** พัฒนาเกมจำลอง CLI ใน `simulator/game.c`:
  - Game state: `health` (100), `score` (0), `ticks`
  - Event loop จำลองการเล่นและพิมพ์ log สถานะ
  - รองรับ `--ticks <N>`, `--interval <ms>`, `--ipc-fd <fd>`, และสัญญาณ `SIGTERM`/`SIGINT`
- [x] **Task 3.2:** พัฒนาโปรแกรมจำลองการโกงใน `simulator/cheat_test.c`:
  - จำลองพฤติกรรมผิดปกติสำหรับทดสอบตรวจจับใน controlled environment
  - รองรับ `--target <pid>`, `--duration <sec>`, `--mode <type>`, และสัญญาณ `SIGTERM`/`SIGINT`
- [x] **Task 3.3:** อัปเดต `Makefile` ให้ build targets: `game` และ `cheat_test` พร้อมชุดทดสอบ `tests/test_simulators.c`

---

### Phase 4: Real-time Monitoring & Game Launcher
> **Branch:** `feature/launcher` | **Status:** ⏳ รอเริ่มดำเนินงาน
- [ ] **Task 4.1:** สร้างโมดูล Game Launcher (`include/game_launcher.h`, `src/game_launcher.c`)
- [ ] **Task 4.2:** ใช้ System Calls `fork()` และ `execvp()` เพื่อรัน Game Simulator เป็น child process
- [ ] **Task 4.3:** ติดตามสถานะของ child process ด้วย `waitpid()` พร้อม flag `WNOHANG`
- [ ] **Task 4.4:** เชื่อมต่อ Inter-Process Communication (IPC) ด้วย `pipe()` หรือ `dup2()`
- [ ] **Task 4.5:** พัฒนา Monitoring Loop สแกนระบบเป็นระยะ (Interval scan) และแจ้งเตือนแบบเรียลไทม์

---

### Phase 5: Event Logging & Controlled Protection
> **Branch:** `feature/logging` | **Status:** ⏳ รอเริ่มดำเนินงาน
- [ ] **Task 5.1:** พัฒนาระบบ Logger ใน `include/logger.h` และ `src/logger.c`:
  - บันทึก Timestamp, Event Type, Target PID, Name, Reason ลงใน `logs/anticheat.log`
  - ใช้ System Calls `open()`, `write()`, `close()` อย่างรัดกุม
- [ ] **Task 5.2:** พัฒนากลไก Safe Process Termination:
  - ส่งสัญญาณ `kill(child_pid, SIGTERM)` **เฉพาะ Game Child Process ที่ Anti-cheat เป็นเจ้าของเท่านั้น**
  - ห้าม terminate process อื่นในระบบโดยเด็ดขาดตาม Security Policy
- [ ] **Task 5.3:** จัดการเก็บกวาดสถานะ child process ที่ถูก terminate เพื่อป้องกัน Zombie process

---

### Phase 6: Testing, Documentation & University Demo
> **Branch:** `main` | **Status:** ⏳ รอเริ่มดำเนินงาน
- [ ] **Task 6.1:** สร้าง automated integration test script (`tests/run_tests.sh`)
- [ ] **Task 6.2:** ตรวจสอบ Memory Safety ด้วย Valgrind (`valgrind --leak-check=full`)
- [ ] **Task 6.3:** ปรับปรุงเอกสาร `README.md` และสรุป Architecture ใน `docs/`
- [ ] **Task 6.4:** เตรียมสคริปต์ขั้นตอนการนำเสนอสด (Live Demonstration Script) สำหรับวิชา OS

---

## 🛠 คำสั่งการใช้งานพื้นฐาน (Commands)

```bash
# คอมไพล์โปรแกรม
make

# รันชุดทดสอบ Unit Tests
make test

# ทำความสะอาด build artifacts
make clean

# รัน Process Scanner ปกติ
./anticheat

# กรองแสดงผลเฉพาะ Zombie processes
./anticheat --zombies

# รันด้วย mock proc directory (สำหรับทดสอบ)
./anticheat --proc-dir <path-to-proc>
```
