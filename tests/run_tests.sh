#!/usr/bin/env bash
# ==============================================================================
# Mini Anti-Cheat: Automated End-to-End Integration Test Suite
# Course: Operating Systems and System Calls
# ==============================================================================

set -e

GREEN="\033[1;32m"
RED="\033[1;31m"
CYAN="\033[1;36m"
YELLOW="\033[1;33m"
RESET="\033[0m"

PASSED_COUNT=0
TOTAL_COUNT=10

pass_test() {
    PASSED_COUNT=$((PASSED_COUNT + 1))
    echo -e "  ${GREEN}✓ PASS:${RESET} $1\n"
}

fail_test() {
    echo -e "  ${RED}✗ FAIL:${RESET} $1\n"
    exit 1
}

echo -e "\n${CYAN}======================================================${RESET}"
echo -e "${CYAN}   MINI ANTI-CHEAT: AUTOMATED INTEGRATION TESTS       ${RESET}"
echo -e "${CYAN}======================================================${RESET}\n"

# 1. Clean Build & Unit Tests
echo -e "${YELLOW}[1/9] Building project and running unit tests...${RESET}"
make clean > /dev/null
make > /dev/null
make test > /dev/null
pass_test "All unit test suites (Scanner, Detector, Simulator, Launcher, Logger) passed"

# 2. CLI Help & Usage
echo -e "${YELLOW}[2/9] Testing CLI usage flags (--help)...${RESET}"
HELP_OUT=$(./anticheat --help)
if echo "$HELP_OUT" | grep -q "Usage: ./anticheat"; then
    pass_test "CLI prints usage options correctly"
else
    fail_test "CLI help message missing or malformed"
fi

# 3. Process Scanner Table Output
echo -e "${YELLOW}[3/9] Testing Process Scanner with mock /proc directory...${RESET}"
MOCK_PROC=$(mktemp -d)
mkdir -p "$MOCK_PROC/1" "$MOCK_PROC/1042"
cat << 'EOF' > "$MOCK_PROC/1/status"
Name:	systemd
State:	S (sleeping)
Pid:	1
PPid:	0
EOF
cat << 'EOF' > "$MOCK_PROC/1042/status"
Name:	NetworkManager
State:	S (sleeping)
Pid:	1042
PPid:	1
EOF

SCAN_OUT=$(./anticheat --proc-dir "$MOCK_PROC")
if echo "$SCAN_OUT" | grep -q "systemd" && echo "$SCAN_OUT" | grep -q "NetworkManager"; then
    pass_test "Process scanner parses and displays process table correctly"
else
    rm -rf "$MOCK_PROC"
    fail_test "Process scanner table output missing mock processes"
fi

# 4. Zombie Process Filtering
echo -e "${YELLOW}[4/9] Testing Zombie process filtering (--zombies)...${RESET}"
mkdir -p "$MOCK_PROC/999"
cat << 'EOF' > "$MOCK_PROC/999/status"
Name:	defunct_proc
State:	Z (zombie)
Pid:	999
PPid:	1
EOF

ZOMBIE_OUT=$(./anticheat --proc-dir "$MOCK_PROC" --zombies)
if echo "$ZOMBIE_OUT" | grep -q "defunct_proc" && ! echo "$ZOMBIE_OUT" | grep -q "NetworkManager"; then
    pass_test "Zombie process filter correctly displays only zombie processes"
else
    rm -rf "$MOCK_PROC"
    fail_test "Zombie process filtering failed"
fi

# 5. Rule-Based Cheat Detection
echo -e "${YELLOW}[5/9] Testing Cheat Detection Engine (--detect & --alerts-only)...${RESET}"
mkdir -p "$MOCK_PROC/200" "$MOCK_PROC/300"
cat << 'EOF' > "$MOCK_PROC/200/status"
Name:	gdb
State:	S (sleeping)
Pid:	200
PPid:	1
EOF
cat << 'EOF' > "$MOCK_PROC/300/status"
Name:	cheat_test
State:	R (running)
Pid:	300
PPid:	1
EOF

DETECT_OUT=$(./anticheat --proc-dir "$MOCK_PROC" --detect)
if echo "$DETECT_OUT" | grep -q "SUSPICIOUS" && echo "$DETECT_OUT" | grep -q "CONFIRMED_CHEAT"; then
    pass_test "Cheat detection engine classifies SUSPICIOUS and CONFIRMED_CHEAT accurately"
else
    rm -rf "$MOCK_PROC"
    fail_test "Detection classification error"
fi

ALERTS_OUT=$(./anticheat --proc-dir "$MOCK_PROC" --alerts-only)
if ! echo "$ALERTS_OUT" | grep -q "systemd" && echo "$ALERTS_OUT" | grep -q "cheat_test"; then
    pass_test "Alerts-only filter displays only flagged processes"
else
    rm -rf "$MOCK_PROC"
    fail_test "Alerts-only filtering failed"
fi
rm -rf "$MOCK_PROC"

# 6. Standalone Simulators
echo -e "${YELLOW}[6/9] Testing standalone Game & Cheat simulators...${RESET}"
./game --ticks 2 --interval 10 > /dev/null
./cheat_test --duration 1 > /dev/null
pass_test "Game simulator and Cheat simulator run and terminate cleanly"

# 7. Real-Time Monitoring & IPC
echo -e "${YELLOW}[7/9] Testing Real-time Monitoring & IPC event transfer...${RESET}"
MOCK_PROC=$(mktemp -d)
mkdir -p "$MOCK_PROC/1"
cat << 'EOF' > "$MOCK_PROC/1/status"
Name:	systemd
State:	S (sleeping)
Pid:	1
PPid:	0
EOF

MONITOR_OUT=$(./anticheat --launch --proc-dir "$MOCK_PROC" --max-ticks 2 --interval 50)
if echo "$MONITOR_OUT" | grep -q "Game IPC Event: EVENT:TICK:"; then
    pass_test "Anti-cheat monitors game session and receives IPC events via pipe"
else
    rm -rf "$MOCK_PROC"
    fail_test "IPC communication failed during monitoring"
fi
rm -rf "$MOCK_PROC"

# 8. Active Protection Policy & Safe Termination
echo -e "${YELLOW}[8/9] Testing Active Protection Policy (Controlled Safe Termination)...${RESET}"
MOCK_PROC=$(mktemp -d)
mkdir -p "$MOCK_PROC/1" "$MOCK_PROC/300"
cat << 'EOF' > "$MOCK_PROC/1/status"
Name:	systemd
State:	S (sleeping)
Pid:	1
PPid:	0
EOF
cat << 'EOF' > "$MOCK_PROC/300/status"
Name:	cheat_test
State:	R (running)
Pid:	300
PPid:	1
EOF

TEST_LOG="/tmp/anticheat_integration_test.log"
rm -f "$TEST_LOG"

PROTECT_OUT=$(./anticheat --launch --protect --proc-dir "$MOCK_PROC" --log "$TEST_LOG" --interval 50)
if echo "$PROTECT_OUT" | grep -q "Safely terminating controlled game process" && \
   echo "$PROTECT_OUT" | grep -q "Session terminated under Protection Policy"; then
    pass_test "Active protection policy terminates game process safely upon cheat detection"
else
    rm -rf "$MOCK_PROC" "$TEST_LOG"
    fail_test "Active protection policy did not terminate controlled game"
fi

# 9. Audit Log Verification
echo -e "${YELLOW}[9/9] Verifying Event Log entries...${RESET}"
if grep -q "CONFIRMED_CHEAT" "$TEST_LOG" && \
   grep -q "PROTECTION_ACTION" "$TEST_LOG" && \
   grep -q "GAME_LAUNCH" "$TEST_LOG"; then
    pass_test "Audit log recorded GAME_LAUNCH, CONFIRMED_CHEAT, and PROTECTION_ACTION"
else
    rm -rf "$MOCK_PROC" "$TEST_LOG"
    fail_test "Log file verification failed"
fi

rm -rf "$MOCK_PROC" "$TEST_LOG"

echo -e "${GREEN}======================================================${RESET}"
echo -e "${GREEN}   ALL $PASSED_COUNT / $TOTAL_COUNT INTEGRATION TESTS PASSED SUCCESSFULLY! ${RESET}"
echo -e "${GREEN}======================================================${RESET}\n"
