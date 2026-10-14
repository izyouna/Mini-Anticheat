# Mini Anti-Cheat: System Architecture & System Calls Guide

เอกสารอธิบายสถาปัตยกรรมระบบ การทำงานภายใน และการประยุกต์ใช้ POSIX System Calls  
สำหรับวิชา: **Operating Systems and System Calls**

---

## 1. ภาพรวมสถาปัตยกรรมระบบ (System Architecture)

```text
               +----------------------------------+
               |        CLI Entry (main.c)        |
               +-----------------+----------------+
                                 |
        +------------------------+------------------------+
        |                                                 |
        v                                                 v
+-----------------------+                     +-----------------------+
|  Game Launcher        |                     | Process Scanner       |
|  (game_launcher.c)    |                     | (process_scanner.c)   |
+-----------+-----------+                     +-----------+-----------+
            | fork() & pipe()                             | opendir() & read()
            v                                             v
+-----------------------+                     +-----------------------+
| Child Game Process    |                     | Linux /proc           |
| (simulator/game.c)    |                     | (/proc/[pid]/status)  |
+-----------+-----------+                     +-----------+-----------+
            |                                             |
            | IPC Event messages                          | Process info
            v                                             v
+---------------------------------------------------------------------+
|                      Cheat Detection Engine                         |
|                       (cheat_detector.c)                            |
+---------------------------------+-----------------------------------+
                                  |
               +------------------+------------------+
               |                                     |
               v                                     v
+-------------------------------+   +-------------------------------+
|       Active Protection       |   |       POSIX Logger            |
| kill(game_pid, SIGTERM)       |   | (logger.c -> logs/*.log)      |
+-------------------------------+   +-------------------------------+
```

---

## 2. ตารางสรุป System Calls และ Library APIs ที่ใช้ในโปรเจกต์

| หมวดหมู่ | System Call / API | หน้าที่ในโปรเจกต์ Mini Anti-Cheat |
|---|---|---|
| **Process Management** | `fork()` | แตกกระบวนการลูก (Child Process) จาก Anti-Cheat เพื่อรัน Game Simulator |
| | `execvp()` | โหลดและรันไบนารีของเกม (`./game`) ทับ image ของ child process |
| | `waitpid()` | ตรวจสอบสถานะการทำงานของ child process (`WNOHANG`) และเก็บกวาดสถานะหลังเกมจบเพื่อป้องกัน Zombie Process |
| | `getpid()` | อ่าน Process ID ของตนเองสำหรับแสดงผลและจับคู่ PID |
| | `getppid()` | อ่าน Parent Process ID เพื่อตรวจดูลำดับสายตระกูลของโพรเซส |
| | `kill()` | ส่งสัญญาณ `SIGTERM` ยุติ **Controlled Game Child Process** ทันทีที่ตรวจพบการโกงที่ยืนยันแล้ว |
| **Inter-Process Communication** | `pipe()` | สร้างท่อสื่อสาร (IPC Pipe) แบบ Unidirectional ส่งข้อมูลเหตุการณ์จาก Game ไปยัง Anti-Cheat |
| | `fcntl()` | ปรับแต่ง File Descriptor ของ Pipe ให้ทำงานแบบ `O_NONBLOCK` ป้องกัน Monitoring Loop ค้าง |
| **Directory Operations** | `opendir()` | เปิดไดเรกทอรี `/proc` เพื่อเตรียมอ่านรายชื่อโพรเซสในระบบ |
| | `readdir()` | กวาดอ่าน entries ทั้งหมดใน `/proc` กรองเฉพาะ entry ที่เป็นตัวเลข (PID) |
| | `closedir()` | ปิด directory stream เพื่อคืนทรัพยากรให้ OS |
| **File Operations** | `open()` | เปิด/สร้างไฟล์บันทึกประวัติ `logs/anticheat.log` ด้วย flag `O_CREAT \| O_WRONLY \| O_APPEND` |
| | `read()` | อ่านข้อความ IPC จากท่อส่งข้อมูล และอ่านไฟล์ `/proc/[pid]/status` |
| | `write()` | เขียนบันทึกเหตุการณ์ลงใน Log File และส่งข้อมูล IPC ผ่าน Pipe |
| | `close()` | ปิด File Descriptor ทุกตัวอย่างปลอดภัย ป้องกัน File Descriptor Leak |
| **Signal Handling** | `sigaction()` | ดักจับสัญญาณ `SIGINT` และ `SIGTERM` เพื่อปิดโปรแกรมอย่างปลอดภัย (Clean Shutdown) |

---

## 3. การทำงานของโมดูลสำคัญ (Module Breakdown)

### 3.1 Process Scanner (`src/process_scanner.c`)
- **การทำงาน:** สแกนหาโพรเซสที่กำลังทำงานอยู่บน Linux โดยเข้าถึง Linux Virtual Filesystem `/proc`
- **การสกัดข้อมูล:** เปิดไฟล์ `/proc/[pid]/status` และอ่านค่า:
  - `Name:` ชื่อโพรเซส
  - `State:` สถานะของโพรเซส (เช่น `R` Running, `S` Sleeping, `Z` Zombie)
  - `Pid:` Process ID
  - `PPid:` Parent Process ID
- **การจัดการข้อผิดพลาด:** หากโพรเซสปิดตัวลงไปกะทันหันขณะกำลังสแกน (`ENOENT`) หรือติดสิทธิ์การเข้าถึง (`EACCES`) ฟังก์ชันจะข้ามโพรเซสนั้นไปอย่างนุ่มนวลโดยไม่ทำให้ระบบ Crash

### 3.2 Cheat Detection Engine (`src/cheat_detector.c`)
จำแนกสถานะการตรวจจับตาม Detection Policy ออกเป็น 3 ระดับ:
1. `NORMAL`: โพรเซสปกติทั่วไป เช่น daemon ของระบบ หรือแอปพลิเคชันทั่วไป
2. `SUSPICIOUS`: โพรเซสที่มีชื่อตรงกับเครื่องมือดีบักเกอร์หรือเครื่องมือตรวจสอบหน่วยความจำ (เช่น `gdb`, `lldb`, `scanmem`, `strace`, `cheatengine`) **ระบบจะไม่สั่ง Kill อัตโนมัติเด็ดขาด** เพื่อป้องกัน False Positive
3. `CONFIRMED_CHEAT`: ตรวจพบโปรแกรมโกงที่สร้างขึ้นสำหรับสภาพแวดล้อมจำลองการเรียนรู้นี้โดยเฉพาะ (เช่น `cheat_test`)

### 3.3 Game Launcher & Real-Time Monitoring (`src/game_launcher.c`)
- ใช้ System Call `pipe()` สร้างท่อสื่อสาร จากนั้นเรียก `fork()`
- ใน Child Process: สั่งรัน `./game --ipc-fd <write_fd>` ผ่าน `execvp()`
- ใน Parent Process: ตั้งค่า Read Pipe เป็น `O_NONBLOCK` และเข้าสู่ Monitoring Loop
- อ่าน IPC Event จากเกม พร้อมวนสแกน `/proc` เป็นระยะ

### 3.4 Active Protection & Safe Termination
- เพื่อความปลอดภัยตามหลักการของระบบ Anti-cheat เพื่อการศึกษา ระบบมีกฎเหล็กว่า:
  > **"ห้ามแตะต้องหรือสั่ง Kill โพรเซสภายนอกหรือโพรเซสอื่นในระบบเด็ดขาด"**
- เมื่อตรวจพบ `CONFIRMED_CHEAT` กลไก Active Protection จะส่งสัญญาณ `kill(game_pid, SIGTERM)` ไปยัง **Controlled Game Child Process** ของตนเองเท่านั้น เพื่อหยุดการทำงานของเซสชันเกมที่ถูกคุกคาม
- เรียก `waitpid(game_pid, &status, 0)` ดักรอเก็บกวาดโพรเซสลูกทันที เพื่อไม่ให้กลายเป็น Zombie process

### 3.5 Event Logger (`src/logger.c`)
- ใช้ `open()` และ `write()` ระดับ POSIX บันทึกเหตุการณ์ลง `logs/anticheat.log`
- แต่ละบรรทัดประกอบด้วย:
  `[YYYY-MM-DD HH:MM:SS] [EVENT_TYPE] PID: <pid> (<name>) - <reason>`
- ใช้ตรวจสอบย้อนหลัง (Audit Trail) ได้อย่างโปร่งใส

---

## 4. ความปลอดภัยและการจัดการหน่วยความจำ (Memory Safety)
- ทุก Dynamic Allocation (`realloc`, `malloc`) มีการตรวจสอบค่า `NULL` เสมอ
- มีฟังก์ชัน Free สำหรับทุก Data Structure: `process_list_free()`, `detection_result_list_free()`, `game_process_cleanup()`
- ใช้ `snprintf` และ `strncpy` ที่มีขอบเขตชัดเจน ป้องกันปัญหา Buffer Overflow
- คอมไพล์ด้วย Flag เข้มงวดสูงสุด: `-std=c11 -Wall -Wextra -Wpedantic -O2` โดยไม่พบ Warning แม้แต่อันเดียว
