---
marp: true
theme: default
paginate: true
header: "Mini Anti-Cheat: Process-Based Cheat Detection System | OS Mini-Project"
footer: "Operating Systems and System Calls (Ubuntu Linux / C11)"
style: |
  section {
    font-family: 'Prompt', 'Kanit', 'Helvetica Neue', sans-serif;
    font-size: 22px;
    padding: 35px 50px;
  }
  h1 {
    color: #007acc;
  }
  h2 {
    color: #2b7489;
  }
  code {
    background-color: #f4f4f4;
    color: #d14;
    font-family: 'JetBrains Mono', 'Fira Code', monospace;
  }
  pre {
    background-color: #282c34;
    color: #abb2bf;
    font-family: 'JetBrains Mono', 'Fira Code', monospace;
  }
---

<!-- 
=============================================================================
SLIDE 1: หน้าปก (TITLE SLIDE)
=============================================================================
-->

# Mini Anti-Cheat
### Process-Based Cheat Detection System
**การตรวจจับพฤติกรรมโกงระดับโพรเซสด้วย POSIX System Calls และ Linux `/proc`**

---

* **รายวิชา:** Operating Systems and System Calls
* **แพลตฟอร์ม:** Ubuntu Linux (C11, GCC, GNU Make)
* **ผู้จัดทำ:** [ใส่รายชื่อสมาชิกในกลุ่ม และรหัสนักศึกษา]
* **สถานะโครงการ:** ผ่านการทดสอบ 100% (18 Unit Tests + 10 Integration Tests)

<!--
[SPEAKER NOTES / บทพูดนำเสนอ]:
"สวัสดีครับอาจารย์และเพื่อนๆ ทุกคน วันนี้กลุ่มของพวกเราจะมานำเสนอโปรเจกต์ 'Mini Anti-Cheat: Process-Based Cheat Detection System' สำหรับรายวิชา Operating Systems and System Calls ครับ
โปรเจกต์นี้เป็นการนำทฤษฎีและแนวคิดพื้นฐานของระบบปฏิบัติการ ไม่ว่าจะเป็นเรื่อง Process Management, Virtual Filesystem, IPC และ Signal Handling มาพัฒนาเป็นระบบตรวจจับกระบวนการโกงบน Ubuntu Linux โดยเขียนด้วยภาษา C มาตรฐาน C11 ครับ"
-->

---

<!-- 
=============================================================================
SLIDE 2: ปัญหาและที่มา (PROBLEM STATEMENT & MOTIVATION)
=============================================================================
-->

## 1. ที่มาและความสำคัญ (Motivation & Problem)

* **ความท้าทายในระดับ Operating System:**
  * โพรเซสในระบบ Linux ทำงานใน User-space แยก Virtual Address Space ออกจากกันอย่างอิสระ
  * โปรแกรมโกงเกม (Game Cheats / Debuggers) มักใช้เครื่องมือเช่น Memory Scanners (`scanmem`, `Cheat Engine`) หรือ Debuggers (`gdb`, `strace`) เพื่อดักจับและแก้ไขข้อมูล
* **ปัญหาของระบบ Anti-Cheat ทั่วไป:**
  * มักใช้สิทธิ์ Kernel Driver ระดับสูง ซึ่งอาจทำให้ระบบเสี่ยงต่อเสถียรภาพหรือละเมิดความเป็นส่วนตัว
  * ขาดความปลอดภัยในการจัดการ Process (เช่น การสั่งปิดโปรแกรมผิดพลาด หรือการสร้าง Zombie Process)
* **เป้าหมายเพื่อการศึกษา:**
  * ต้องการจำลองระบบ Anti-Cheat ระดับ User-Space ที่ถูกต้อง ปลอดภัย และโปร่งใสตามหลักการ OS

<!--
[SPEAKER NOTES / บทพูดนำเสนอ]:
"ในโลกความเป็นจริง การตรวจจับโปรแกรมโกงเป็นความท้าทายระดับระบบปฏิบัติการครับ เพราะ OS ถูกออกแบบมาให้โพรเซสแยกพื้นที่หน่วยความจำกัน แต่โปรแกรมโกงมักพยายามใช้ System Call หรือเครื่องมือ Debugger เข้าไปแทรกแซงหน่วยความจำของเกม
กลุ่มเราจึงต้องการพัฒนา Mini Anti-Cheat เพื่อศึกษาว่าเราจะสามารถตรวจสอบความสมบูรณ์ของเกม ตรวจจับโพรเซสต้องสงสัย และจัดการโพรเซสได้อย่างปลอดภัย โดยอาศัยเพียง POSIX System Calls มาตรฐานได้อย่างไรครับ"
-->

---

<!-- 
=============================================================================
SLIDE 3: วัตถุประสงค์และขอบเขตโครงการ (OBJECTIVES & SCOPE)
=============================================================================
-->

## 2. วัตถุประสงค์และขอบเขตโครงการ (Objectives & Scope)

### วัตถุประสงค์หลัก
1. **เข้าใจโครงสร้าง Process:** ศึกษาการทำงานของ Linux Process Table ผ่าน Linux `/proc` Filesystem
2. **ประยุกต์ใช้ POSIX APIs:** ฝึกการใช้ System Calls ครอบคลุม Process, IPC, Signals และ I/O
3. **ระบบตรวจจับอัจฉริยะ (Rule-Based):** จำแนกระดับความเสี่ยงของโพรเซสในระบบ
4. **Controlled Environment:** สร้างเกมและโปรแกรมโกงจำลองเพื่อการทดสอบที่ควบคุมผลได้ 100%

### ขอบเขตความปลอดภัย (Safety & Educational Policy)
* ❌ ไม่พัฒนา Kernel Driver หรือ Bypass สิทธิ์ความปลอดภัยของ OS
* ❌ **ไม่สั่ง Terminate โพรเซสภายนอกหรือระบบหลักของเครื่องเด็ดขาด**
* ✅ ควบคุมและสั่งปิดเฉพาะ **Controlled Game Process** ที่ตนเองเป็นผู้สร้างขึ้นเท่านั้น

<!--
[SPEAKER NOTES / บทพูดนำเสนอ]:
"วัตถุประสงค์หลักของโปรเจกต์คือการเชื่อมโยงทฤษฎีในห้องเรียนเข้ากับการเขียนโปรแกรมระดับระบบจริงครับ โดยเราตั้งขอบเขตความปลอดภัยอย่างเข้มงวดว่า โปรเจกต์นี้เป็น Educational Project ระบบของเราจะไม่มีการไปแทรกแซงหรือสั่ง Kill โพรเซสใดๆ ของระบบเด็ดขาด การสั่งปิดจะเกิดขึ้นเฉพาะกับ Child Process ของเกมที่เราควบคุมเอง เพื่อรักษาความปลอดภัยของระบบครับ"
-->

---

<!-- 
=============================================================================
SLIDE 4: สถาปัตยกรรมระบบ (SYSTEM ARCHITECTURE)
=============================================================================
-->

## 3. สถาปัตยกรรมระบบ (System Architecture)

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
            | IPC Messages                                | Process Info
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

<!--
[SPEAKER NOTES / บทพูดนำเสนอ]:
"นี่คือภาพรวมสถาปัตยกรรม 5 โมดูลหลักของระบบครับ:
1. Game Launcher รับหน้าที่แตกกระบวนการลูกและสื่อสารผ่าน IPC Pipe
2. Process Scanner ทำหน้าที่สแกน Virtual Filesystem /proc
3. Detection Engine นำข้อมูลโพรเซสและข้อมูลเกมมาวิเคราะห์ความผิดปกติ
4. Active Protection ทำหน้าที่ตัดวงจรความเสียหายด้วยการส่งสัญญาณปิดเกมอย่างปลอดภัย
5. POSIX Logger ทำการบันทึกประวัติเหตุการณ์ทั้งหมดลงสู่ Audit Log ครับ"
-->

---

<!-- 
=============================================================================
SLIDE 5: โมดูล PROCESS SCANNER & /PROC FILESYSTEM
=============================================================================
-->

## 4. เจาะลึก: Process Scanner & Linux `/proc`

### การทำงานกับ Linux Virtual Filesystem
* เข้าถึงไดเรกทอรี `/proc` ด้วย `opendir()` และ `readdir()`
* กรองอ่านเฉพาะ Entry ที่เป็นตัวเลข (Numeric Directory = PID)
* สกัดข้อมูลจาก `/proc/[pid]/status` แบบ Line-by-Line:
  * `Name:` ชื่อกระบวนการ
  * `State:` สถานะโพรเซส (`R` Running, `S` Sleeping, `Z` Zombie)
  * `Pid:` Process ID และ `PPid:` Parent Process ID

### การจัดการข้อผิดพลาดและ Edge Cases (Robustness)
* **Process Disappearing (`ENOENT`):** โพรเซสปิดตัวลงไประหว่างที่กำลังสแกน $\rightarrow$ ดักจับและข้ามอย่างปลอดภัย
* **Permission Denied (`EACCES`):** สิทธิ์ไม่เพียงพอในการอ่านไฟล์ของ Root $\rightarrow$ ข้าม ไม่ทำให้ Scanner แครช

<!--
[SPEAKER NOTES / บทพูดนำเสนอ]:
"ในส่วนของ Process Scanner เราเลือกอ่านจาก /proc filesystem โดยตรง ซึ่งเป็น Virtual Filesystem ใน Kernel Memory แทนการใช้คำสั่ง shell อย่าง ps
เราเปิดอ่านไฟล์ /proc/[pid]/status เพื่อดึง PID, PPID, ชื่อ และ State
จุดเด่นสำคัญของการเขียนโปรแกรมระดับ OS ตรงนี้คือ Race Condition ครับ โพรเซสอาจจะตายไประหว่างที่เรากำลังจะเปิดอ่านไฟล์ โค้ดของเราจึงดักจับ ENOENT และ EACCES ไว้เพื่อไม่ให้โปรแกรม Crash ครับ"
-->

---

<!-- 
=============================================================================
SLIDE 6: กลไกการตรวจจับ (CHEAT DETECTION ENGINE)
=============================================================================
-->

## 5. กลไกการตรวจจับ (Cheat Detection Engine)

### นโยบายการจำแนกสถานะ 3 ระดับ (Detection Policy)

| ระดับการตรวจจับ | ตัวอย่าง Process | ความหมายและการดำเนินการ |
|:---|:---|:---|
| **NORMAL** | `bash`, `code`, `systemd` | โพรเซสปกติของระบบและผู้ใช้ ทำงานได้ตามปกติ |
| **SUSPICIOUS** | `gdb`, `scanmem`, `strace`, `lldb` | เครื่องมือ Debugger/Scanner **แจ้งเตือนแต่ไม่สั่งปิด** (ป้องกัน False Positive) |
| **CONFIRMED_CHEAT** | `cheat_test` (Known Signature) | ยืนยันการโกงในสภาพแวดล้อมจำลอง **สั่งเริ่มกลไก Active Protection** |

> **Highlight:** การออกแบบ Rule-Based ที่แยก "น่าสงสัย" ออกจาก "โกงแน่นอน" ช่วยป้องกันการทำลายโพรเซสของนักพัฒนาหรือแอปพลิเคชันทั่วไปในระบบ

<!--
[SPEAKER NOTES / บทพูดนำเสนอ]:
"Detection Engine ใช้หลักการ Rule-based และแยกแยะสถานะออกเป็น 3 ระดับชัดเจนครับ
สิ่งสำคัญคือ เราไม่ตัดสินว่าโพรเซสกำลังโกงเพียงเพราะชื่อของมัน หากผู้ใช้กำลังเปิด gdb เพื่อดีบักงานอื่น ระบบจะจัดเป็น SUSPICIOUS เท่านั้นและไม่มีการสั่ง Kill ใดๆ ทั้งสิ้น
ระบบจะยืนยันว่าเป็น CONFIRMED_CHEAT เฉพาะเมื่อพบโมเดลจำลองการโกงที่ระบุไว้ชัดเจนเท่านั้นครับ"
-->

---

<!-- 
=============================================================================
SLIDE 7: ตาราง POSIX SYSTEM CALLS ที่ใช้ในโครงการ
=============================================================================
-->

## 6. POSIX System Calls & APIs ในโปรเจกต์

| หมวดหมู่ | System Call / API | หน้าที่และบทบาทในโปรเจกต์ |
|:---|:---|:---|
| **Process Management** | `fork()` | สร้าง Child Process แยกตัวเกมจำลองออกจาก Anti-Cheat |
| | `execvp()` | โหลดไฟล์ไบนารี `./game` เข้ามาแทนที่ Image ของ Child Process |
| | `waitpid()` | ตรวจสถานะ Child ด้วย `WNOHANG` และเก็บกวาดเพื่อ **ป้องกัน Zombie Process** |
| | `kill()` | ส่งสัญญาณ `SIGTERM` ยุติเกมจำลองเมื่อตรวจพบการโกง |
| **IPC** | `pipe()` | สร้างท่อทางเดียว (Unidirectional Pipe) รับ Event จาก Game สู่ Anti-Cheat |
| | `fcntl()` | กำหนดค่า Pipe ฝั่งอ่านให้เป็น `O_NONBLOCK` ป้องกัน Monitoring Loop ค้าง |
| **Directory & File** | `opendir()`, `readdir()` | สแกนหาไดเรกทอรี PID ใต้ `/proc` |
| | `open()`, `write()`, `close()`| บันทึกเหตุการณ์ลง `logs/anticheat.log` ตามมาตรฐาน POSIX I/O |
| **Signal Handling** | `sigaction()` | ดักจับ `SIGINT` (Ctrl+C) สำหรับ Graceful Clean Shutdown |

<!--
[SPEAKER NOTES / บทพูดนำเสนอ]:
"สไลด์นี้คือหัวใจสำคัญของวิชา OS ครับ เราได้นำ System Calls มาใช้ครบทุกหมวดหมู่หลัก:
- fork() และ execvp() ใช้ตอนเปิดเกม
- pipe() และ fcntl() ทำ IPC แบบ Non-blocking ส่งข้อมูลคะแนนและพลังชีวิตของเกม
- waitpid() คอยตรวจสอบว่าเกมยังอยู่ไหม และป้องกันการเกิด Zombie Process
- kill() ส่ง SIGTERM ปิดเกม
- และ sigaction() ดูแลเรื่อง Signal Handler ตอนผู้ใช้กด Ctrl+C ครับ"
-->

---

<!-- 
=============================================================================
SLIDE 8: CONTROLLED GAME & SAFE TERMINATION
=============================================================================
-->

## 7. Controlled Simulation & Safe Termination

### การทำงานร่วมกันระหว่าง Launcher, Game และ Cheat

1. **Game Simulator (`simulator/game.c`):**
   * ทำงานแบบ Game Loop จำลอง Ticks, Health (100) และ Score (0)
   * ส่งข้อมูล State ผ่าน File Descriptor ของ Pipe ไปยัง Launcher แบบ Real-Time
2. **Cheat Simulator (`simulator/cheat_test.c`):**
   * จำลองกระบวนการแฮกคะแนน (Score Hack) หรือแก้ไขหน่วยความจำ
3. **Safe Process Termination Mechanism:**
   * เมื่อ Anti-Cheat ตรวจพบ `cheat_test`:
     $$\text{Detection Engine} \xrightarrow{\text{Alert}} \text{Launcher} \xrightarrow{\text{kill(game\_pid, SIGTERM)}} \text{Game Process}$$
   * ปิดเฉพาะเซสชันเกมที่ถูกบุกรุก ไม่แตะต้องโพรเซสอื่นในระบบ
   * เรียก `waitpid(game_pid, &status, 0)` เก็บกวาดทรัพยากรทันที

<!--
[SPEAKER NOTES / บทพูดนำเสนอ]:
"เพื่อการทดสอบที่ปลอดภัย เราจึงสร้างทั้ง Game Simulator และ Cheat Simulator ขึ้นมาเองครับ
เมื่อตัวเกมรันอยู่ มันจะส่งค่า Ticks, Health และ Score ผ่าน Pipe กลับมาให้ Anti-Cheat อย่างต่อเนื่อง
และเมื่อเราเปิด cheat_test ขึ้นมา Anti-Cheat จะตรวจพบ และสั่ง kill(game_pid, SIGTERM) เพื่อปิดตัวเกมทันที
ทำไมเราสั่งปิดเกมแทนที่จะสั่งปิดตัวโกง? เพราะในทาง OS เราไม่มีสิทธิ์และไม่ควรยุ่งกับโพรเซสภายนอก แต่ในฐานะผู้ดูแลเกม การปิดเกมเพื่อรักษา Game Integrity คือแนวทางที่ถูกต้องและปลอดภัยที่สุดครับ"
-->

---

<!-- 
=============================================================================
SLIDE 9: การทดสอบและการรับประกันคุณภาพ (TESTING & QUALITY)
=============================================================================
-->

## 8. การทดสอบและการรับประกันคุณภาพ (Quality Assurance)

### มาตรฐานการพัฒนาและการคอมไพล์
* **Compiler Flags:** คอมไพล์ด้วย GCC แฟล็กความปลอดภัยสูงสุด:
  `-std=c11 -Wall -Wextra -Wpedantic -O2` $\rightarrow$ **Zero Compiler Warnings**
* **Dynamic Memory Safety:** ทุกการจองหน่วยความจำ (`malloc`/`calloc`) มีการตรวจสอบ Pointer และคืนหน่วยความจำ (`free`) ทุกครั้ง ป้องกัน Memory Leaks

### ผลการทดสอบอัตโนมัติ (Automated Test Suites)
* ✅ **Unit Tests (18/18 ผ่าน 100%):**
  * ครอบคลุมการ Parse `/proc`, Dynamic Array Scaling, Rule Matching, IPC Protocol, POSIX Logger
* ✅ **Integration Tests (10/10 ผ่าน 100%):**
  * รัน End-to-End ผ่านสคริปต์ `tests/run_tests.sh` ทดสอบ Zombie Filtering, Detection Alert, IPC Launch, และ Active Protection

<!--
[SPEAKER NOTES / บทพูดนำเสนอ]:
"ในด้านคุณภาพของโค้ด เรายึดแนวทาง Test-Driven Development (TDD) ครับ
เราตั้งค่า Compiler Flags อย่างเข้มงวด ทั้ง -Wall, -Wextra, -Wpedantic โดยไม่มี Warning แม้แต่จุดเดียว
นอกจากนี้เรายังมีชุดทดสอบ Unit Tests 18 ตัว และ Integration Tests อีก 10 ตัว ซึ่งทดสอบการทำงานจริงของระบบทั้งหมดและผ่านการทดสอบ 100% ครับ"
-->

---

<!-- 
=============================================================================
SLIDE 10: ขั้นตอนการสาธิตสด (LIVE DEMONSTRATION PLAN)
=============================================================================
-->

## 9. ขั้นตอนการสาธิตสด (Live Demonstration Plan)

### แบ่งการสาธิตเป็น 2 หน้าต่างเทอร์มินัล (Split Terminal)

```bash
# Terminal 1: ฝั่ง Anti-Cheat & Game (ผู้ดูแลระบบ)
./anticheat --launch --protect --interval 500
```
* สังเกต: Anti-Cheat แตก child process รันเกม, เชื่อม IPC Pipe แสดง Ticks, Health, Score

```bash
# Terminal 2: ฝั่ง Hacker (ผู้จำลองการโกง)
./cheat_test --mode score_hack
```
* สังเกตผลใน Terminal 1:
  1. แจ้งเตือนสีแดง: `[ALERT] CONFIRMED CHEAT DETECTED (PID: xxxxx, Name: cheat_test)`
  2. ส่งสัญญาณ `SIGTERM` ปิดเกมทันที
  3. บันทึกผลลงใน `logs/anticheat.log`

<!--
[SPEAKER NOTES / บทพูดนำเสนอ]:
"เดี๋ยวในส่วนของการ Live Demo เราจะแบ่งหน้าจอเป็น 2 ฝั่งครับ
ฝั่งซ้ายคือ Anti-Cheat กำลังเปิดเกมและรับส่งข้อมูลเกมผ่าน IPC Pipe
และฝั่งขวา เราจะจำลองเป็นผู้เล่นไม่หวังดีเปิดโปรแกรม cheat_test เข้ามา
และเราจะเห็นระบบตรวจจับได้แบบเรียลไทม์ และทำการ Terminate เกมอย่างปลอดภัยทันทีครับ"
-->

---

<!-- 
=============================================================================
SLIDE 11: ปัญหาที่พบและบทเรียนที่ได้รับ (LESSONS LEARNED)
=============================================================================
-->

## 10. ปัญหาที่พบและบทเรียนที่ได้รับ (Lessons Learned)

1. **Race Conditions ใน Virtual Filesystem:**
   * *ปัญหา:* โพรเซสเปิดตัวและปิดตัวอย่างรวดเร็ว ทำให้ไฟล์ใน `/proc/[pid]` หายไประหว่างสแกน
   * *วิธีแก้:* ตรวจสอบ Error Code `ENOENT` และข้ามอย่างนุ่มนวล (Graceful Skip)
2. **Blocking I/O ใน IPC Pipe:**
   * *ปัญหา:* การเรียก `read()` จาก Pipe ปกติจะทำให้ Monitoring Loop หยุดชะงัก (Block) เพื่อรอข้อความจากเกม
   * *วิธีแก้:* ใช้ `fcntl(fd, F_SETFL, O_NONBLOCK)` ทำให้ Loop ยังคงสแกนโพรเซสรอบตัวต่อไปได้
3. **Zombie Process Accumulation:**
   * *ปัญหา:* เมื่อสั่ง `kill()` Child Game Process หากไม่รอรับสถานะ จะเกิด Zombie ค้างในระบบ
   * *วิธีแก้:* ใช้ `waitpid(game_pid, &status, WNOHANG)` ตรวจสอบและเก็บกวาดสถานะอย่างสมบูรณ์

<!--
[SPEAKER NOTES / บทพูดนำเสนอ]:
"สิ่งสำคัญที่สุดที่เราได้จากการทำโปรเจกต์นี้คือบทเรียนในการแก้ปัญหาทางวิศวกรรมระบบครับ:
เรื่องแรกคือ Race condition บน /proc ที่โพรเซสเกิดขึ้นและดับไปตลอดเวลา
เรื่องที่สองคือการใช้ fcntl เปลี่ยน Pipe เป็น Non-blocking I/O ซึ่งจำเป็นมากเพื่อให้ลูปตรวจจับทำงานได้ต่อเนื่อง
และเรื่องที่สามคือการเข้าใจวงจรชีวิตของ Process เพื่อป้องกันไม่ให้เกิด Zombie Process ในระบบครับ"
-->

---

<!-- 
=============================================================================
SLIDE 12: สรุปผลและช่วงถาม-ตอบ (CONCLUSION & Q&A)
=============================================================================
-->

## 11. สรุปผลการดำเนินงาน (Conclusion & Q&A)

### สรุปความสำเร็จของโครงการ
* ✅ ประยุกต์ใช้ POSIX System Calls ได้ครบถ้วนตามข้อกำหนดของวิชา OS
* ✅ อ่านและวิเคราะห์โพรเซสจาก Linux `/proc` ได้อย่างถูกต้องและทนทานต่อข้อผิดพลาด
* ✅ มีระบบตรวจจับแบบ Rule-Based พร้อมกลไก Active Protection และ Audit Logging
* ✅ ทดสอบผ่าน 100% ทั้ง Unit Tests และ Integration Tests

---

### ขอขอบคุณอาจารย์และผู้ฟังทุกท่าน
**ยินดีรับฟังทุกคำแนะนำและตอบข้อซักถามครับ (Q&A)**

* **Source Code Repository:** [GitHub Link ของกลุ่ม]
* **เอกสารสถาปัตยกรรมเพิ่มเติม:** `docs/ARCHITECTURE.md`
* **คู่มือการสาธิตสด:** `docs/DEMO_GUIDE.md`

<!--
[SPEAKER NOTES / บทพูดนำเสนอ]:
"สรุปภาพรวม โปรเจกต์ Mini Anti-Cheat บรรลุวัตถุประสงค์ทั้งการทำงานที่ปลอดภัย การนำ System Calls มาประยุกต์ใช้จริง และความสมบูรณ์ของการทดสอบครับ
กลุ่มของพวกเราขอจบการนำเสนอแต่เพียงเท่านี้ และพร้อมตอบคำถามของอาจารย์และเพื่อนๆ ครับ ขอบคุณครับ"
-->

---

<!-- 
=============================================================================
APPENDIX: คลังคำถาม-คำตอบเจาะลึกสำหรับอาจารย์วิชา OS (Q&A CHEAT SHEET)
=============================================================================
-->

## ภาคผนวก: แนวทางการตอบคำถามอาจารย์วิชา OS (Q&A Preparation)

### Q1: ทำไมถึงไม่อ่านข้อมูลจากคำสั่ง `ps` หรือรันผ่าน `system("ps")`?
* **แนวทางตอบ:** การใช้ `system("ps")` ต้องสร้าง Shell Process ขึ้นมาใหม่ สิ้นเปลือง Overhead สูง ช้า และเสี่ยงต่อการถูกแทรกแซง (Command Injection) การอ่านตรงจาก Kernel Virtual Filesystem `/proc` ผ่าน `opendir()` และ `read()` มีประสิทธิภาพสูงกว่าและเป็นวิธีมาตรฐานที่โปรแกรมระดับ System Monitor บน Linux ทำงานจริงครับ

### Q2: หากโปรแกรมโกงรันด้วยสิทธิ์ Root แต่ Anti-Cheat รันด้วยสิทธิ์ User ธรรมดา จะเกิดอะไรขึ้น?
* **แนวทางตอบ:** ในระบบ Linux ข้อมูลพื้นฐานใน `/proc/[pid]/status` อนุญาตให้ User ทั่วไปอ่านค่า PID, PPID, Name และ State ได้ ทำให้เรายังคงสแกนและตรวจพบชื่อโพรเซสต้องสงสัยได้ แต่ถ้าเราพยายามเปิดดู Memory (`/proc/[pid]/mem`) จะติดสิทธิ์ `EACCES` ทันที ซึ่งสอดคล้องกับหลัก Least Privilege ครับ

### Q3: ท่อ IPC Pipe เป็นแบบ Half-Duplex หรือ Full-Duplex?
* **แนวทางตอบ:** System Call `pipe()` ในมาตรฐาน POSIX เป็นแบบ Unidirectional (Half-Duplex) ครับ โดยมี 2 File Descriptors คือ `fd[0]` สำหรับอ่าน (Read end) และ `fd[1]` สำหรับเขียน (Write end) โดยเกมจะถือ Write end และ Launcher จะถือ Read end ครับ

### Q4: ป้องกัน Zombie Process ได้อย่างไรในทางทฤษฎีและทางปฏิบัติ?
* **แนวทางตอบ:** Zombie Process เกิดจากการที่ Child Process สิ้นสุดการทำงานแล้ว แต่ Parent ยังไม่ได้อ่าน Exit Status ทางทฤษฎีแก้ได้โดย Parent ต้องเรียก `wait()` หรือ `waitpid()` ในโปรเจกต์นี้เราใช้ `waitpid(child_pid, &status, WNOHANG)` ในลูปตรวจสอบ และเรียก `waitpid(child_pid, &status, 0)` เมื่อสั่งปิดเกม เพื่อเก็บกวาดข้อมูลออกจาก Process Table ทันทีครับ
