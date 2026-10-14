# Mini Anti-Cheat: Live University Demonstration Guide

คู่มือขั้นตอนการนำเสนอสด (Live Demo Script) สำหรับการสอบและพรีเซนต์ในวิชา:  
**Operating Systems and System Calls**

---

## 🎯 วัตถุประสงค์ของการนำเสนอ (Demonstration Objectives)

1. แสดงการคอมไพล์โปรแกรมภาษา C ด้วย GCC และ GNU Make โดยไม่มี Compiler Warnings
2. แสดงการทำงานของชุดทดสอบอัตโนมัติ (Automated Unit & Integration Tests)
3. สาธิตการอ่านและวิเคราะห์ข้อมูลจาก `/proc` Filesystem (PID, PPID, Name, State)
4. สาธิตระบบ Rule-Based Cheat Detection และการจำแนกสถานะ
5. สาธิตการรันเกมจำลองผ่าน Game Launcher โดยใช้ System Calls (`fork`, `execvp`, `pipe`, `waitpid`)
6. สาธิตการตรวจจับโปรแกรมโกงแบบเรียลไทม์ และการทำ Safe Process Termination (`kill`)
7. แสดงผลการตรวจสอบประวัติเหตุการณ์ย้อนหลังผ่าน Audit Logs

---

## 🚀 ลำดับขั้นตอนการนำเสนอสด (Step-by-Step Script)

### ขั้นตอนที่ 1: ตรวจสอบความสะอาดของโค้ดและการคอมไพล์ (Build System)
**คำอธิบายให้อาจารย์/ผู้ฟัง:**  
"โปรเจกต์นี้พัฒนาด้วยภาษา C มาตรฐาน C11 โดยคอมไพล์ผ่าน GNU Make ด้วยแฟล็กความปลอดภัยสูงสุด `-std=c11 -Wall -Wextra -Wpedantic -O2`"

```bash
# ล้างไฟล์คอมไพล์เดิมและบิลด์ใหม่ทั้งหมด
make clean && make
```
*สิ่งที่สังเกต:* คอมไพล์ได้ 3 ไบนารีหลัก (`anticheat`, `game`, `cheat_test`) โดยไม่มี warning ใดๆ เกิดขึ้น

---

### ขั้นตอนที่ 2: รันชุดทดสอบอัตโนมัติ (Automated Test Verification)
**คำอธิบายให้อาจารย์/ผู้ฟัง:**  
"เราใช้แนวทาง Test-Driven Development (TDD) ครอบคลุม Unit Tests 18 ตัว และ Integration Tests อีก 10 ตัว"

```bash
# รัน Unit Tests ทั้ง 5 โมดูล
make test

# รัน Integration Test Suite ทั้งหมด
./tests/run_tests.sh
```
*สิ่งที่สังเกต:* ทุกเทสต์ผ่านฉลุย (`PASS: 18/18 Unit Tests` และ `PASS: 10/10 Integration Tests`)

---

### ขั้นตอนที่ 3: สาธิต Process Scanner (เปรียบเทียบกับคำสั่ง OS)
**คำอธิบายให้อาจารย์/ผู้ฟัง:**  
"โมดูล Process Scanner สแกนอ่าน `/proc/[pid]/status` เพื่อดึง PID, Parent PID, ชื่อ และสถานะ"

```bash
# รัน Scanner ปกติ
./anticheat

# กรองแสดงผลเฉพาะ Zombie processes
./anticheat --zombies

# เปรียบเทียบผลลัพธ์กับคำสั่งของ Linux
ps -eo pid,ppid,comm,state | head -n 15
```

---

### ขั้นตอนที่ 4: สาธิต Cheat Detection Engine
**คำอธิบายให้อาจารย์/ผู้ฟัง:**  
"Detection Engine ใช้ Rule-Based วิเคราะห์จำแนกระหว่าง NORMAL, SUSPICIOUS (เช่น gdb, scanmem) และ CONFIRMED_CHEAT (cheat_test)"

```bash
# แสดงการประเมินโพรเซสทั้งหมดพร้อมเหตุผล
./anticheat --detect

# กรองแสดงเฉพาะโพรเซสที่มีการแจ้งเตือน (Alerts Only)
./anticheat --alerts-only
```

---

### ขั้นตอนที่ 5: สาธิตแบบเรียลไทม์ (Live Interactive Demo)
**ไฮไลต์สำคัญของการสาธิต:** ให้เปิด 2 หน้าต่างเทอร์มินัล (Split Terminal)

#### เทอร์มินัลที่ 1: เปิด Anti-Cheat ควบคุมและเปิดเกม
```bash
./anticheat --launch --protect --interval 500
```
*สิ่งที่สังเกต:*
- Anti-Cheat แตก child process ด้วย `fork()` และสั่งรันเกมด้วย `execvp()`
- ท่อ IPC (`pipe()`) รับส่งข้อมูล State ของเกม (Ticks, Health, Score) มาแสดงผลที่ Anti-Cheat แบบเรียลไทม์

#### เทอร์มินัลที่ 2: เปิดโปรแกรมจำลองการโกงเข้าสู่ระบบ
```bash
./cheat_test --mode score_hack
```
*สิ่งที่สังเกตในเทอร์มินัลที่ 1:*
1. Anti-Cheat ตรวจพบโพรเซส `cheat_test` ผ่านการสแกนรอบถัดไป
2. ส่งสัญญาณแจ้งเตือนตัวหนังสือสีแดง: `[ALERT] CONFIRMED CHEAT DETECTED!`
3. กลไก **Active Protection Policy** ทำงานทันที: ส่งสัญญาณ `kill(game_pid, SIGTERM)` ยุติ **Controlled Game Process** อย่างปลอดภัย
4. เรียก `waitpid()` ดักเก็บสถานะ ไม่ทิ้ง Zombie process ไว้ในระบบ
5. โพรเซสอื่นๆ ในระบบไม่ได้รับผลกระทบใดๆ ทั้งสิ้น

---

### ขั้นตอนที่ 6: ตรวจสอบ Audit Log
**คำอธิบายให้อาจารย์/ผู้ฟัง:**  
"เหตุการณ์ทั้งหมดถูกบันทึกด้วย POSIX `open()` และ `write()` ลงใน Log File อย่างแม่นยำ พร้อม Timestamp สำหรับการตรวจสอบย้อนหลัง"

```bash
cat logs/anticheat.log
```
*ตัวอย่างข้อมูลที่บันทึก:*
```text
[2026-10-15 01:37:40] [GAME_LAUNCH] PID: 99108 (game) - Controlled game simulator launched
[2026-10-15 01:37:40] [CONFIRMED_CHEAT] PID: 300 (cheat_test) - Controlled cheat simulator detected: cheat_test
[2026-10-15 01:37:40] [PROTECTION_ACTION] PID: 99108 (game) - Terminated controlled game to protect session
[2026-10-15 01:37:40] [GAME_EXIT] PID: 99108 (game) - Game process ended
```

---

## 💡 สรุปข้อคิดและทักษะที่ได้เรียนรู้ (Takeaway)

- ความเข้าใจอย่างลึกซึ้งในโครงสร้างของ `/proc` Filesystem ในระบบปฏิบัติการ Linux
- การจัดการ Process Lifecycle (`fork`, `execvp`, `waitpid`, `kill`) และการป้องกันการเกิด Zombie / Orphan processes
- การสื่อสารระหว่างโพรเซส (IPC) ผ่านท่อข้อมูล `pipe()` แบบ Non-blocking
- การเขียนโปรแกรมภาษา C ระดับระบบตามมาตรฐาน POSIX และความปลอดภัยของหน่วยความจำ
