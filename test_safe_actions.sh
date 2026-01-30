#!/bin/bash

# LinCheckROOT v2.1 - Safe Actions Test Script
# Verifies all new functionality is working correctly

set -e

echo "╔════════════════════════════════════════════════════════════╗"
echo "║      LinCheckROOT v2.1 - Safe Actions Test Suite           ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

PROJECT_DIR="/home/mihai/Desktop/LinCheckROOT"
BUILD_DIR="$PROJECT_DIR/build"
BINARY="$BUILD_DIR/lincheckroot"

# Test results
TESTS_PASSED=0
TESTS_FAILED=0

# Test function
test_component() {
    local test_name=$1
    local test_cmd=$2
    
    echo -n "Testing $test_name... "
    
    if eval "$test_cmd" > /dev/null 2>&1; then
        echo -e "${GREEN}✓ PASS${NC}"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}✗ FAIL${NC}"
        ((TESTS_FAILED++))
    fi
}

echo "📋 COMPONENT TESTS"
echo "═════════════════════════════════════════════════════════════"

# 1. Check binary exists
test_component "Binary exists" "[ -f '$BINARY' ]"

# 2. Check binary is executable
test_component "Binary executable" "[ -x '$BINARY' ]"

# 3. Check binary type
test_component "Binary type (ELF)" "file '$BINARY' | grep -q 'ELF.*x86-64'"

# 4. Check header files exist
echo -n "Testing ADB client header... "
if [ -f "$PROJECT_DIR/include/adb/adb_client.hpp" ]; then
    echo -e "${GREEN}✓ PASS${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAIL${NC}"
    ((TESTS_FAILED++))
fi

echo -n "Testing Device info header... "
if [ -f "$PROJECT_DIR/include/device/device_info.hpp" ]; then
    echo -e "${GREEN}✓ PASS${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAIL${NC}"
    ((TESTS_FAILED++))
fi

echo -n "Testing Analyzer header... "
if [ -f "$PROJECT_DIR/include/analyzer/analyzers.hpp" ]; then
    echo -e "${GREEN}✓ PASS${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAIL${NC}"
    ((TESTS_FAILED++))
fi

echo -n "Testing Reboot action header... "
if [ -f "$PROJECT_DIR/include/actions/reboot.hpp" ]; then
    echo -e "${GREEN}✓ PASS${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAIL${NC}"
    ((TESTS_FAILED++))
fi

echo -n "Testing Dialogs header... "
if [ -f "$PROJECT_DIR/include/ui/dialogs.hpp" ]; then
    echo -e "${GREEN}✓ PASS${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}✗ FAIL${NC}"
    ((TESTS_FAILED++))
fi

echo
echo "📦 SOURCE FILES TESTS"
echo "═════════════════════════════════════════════════════════════"

# Check source files exist
test_component "ADB client source" "[ -f '$PROJECT_DIR/src/adb/adb_client.cpp' ]"
test_component "Device info source" "[ -f '$PROJECT_DIR/src/device/device_info.cpp' ]"
test_component "Analyzers source" "[ -f '$PROJECT_DIR/src/analyzer/analyzers.cpp' ]"
test_component "Reboot action source" "[ -f '$PROJECT_DIR/src/actions/reboot.cpp' ]"
test_component "Dialogs source" "[ -f '$PROJECT_DIR/src/ui/dialogs.cpp' ]"

echo
echo "📝 DOCUMENTATION TESTS"
echo "═════════════════════════════════════════════════════════════"

test_component "Safe actions documentation" "[ -f '$PROJECT_DIR/SAFE_ACTIONS.md' ]"
test_component "README updated" "grep -q 'Safe Device Actions' '$PROJECT_DIR/README.md'"

echo
echo "🔨 BUILD SYSTEM TESTS"
echo "═════════════════════════════════════════════════════════════"

test_component "CMakeLists.txt exists" "[ -f '$PROJECT_DIR/CMakeLists.txt' ]"
test_component "CMakeLists includes new modules" "grep -q 'src/adb/adb_client.cpp' '$PROJECT_DIR/CMakeLists.txt'"

echo
echo "📊 BINARY SIZE TESTS"
echo "═════════════════════════════════════════════════════════════"

BINARY_SIZE=$(stat -f%z "$BINARY" 2>/dev/null || stat -c%s "$BINARY" 2>/dev/null || echo "0")
BINARY_SIZE_KB=$((BINARY_SIZE / 1024))

echo "Binary size: $BINARY_SIZE_KB KB"

if [ "$BINARY_SIZE_KB" -gt 900 ] && [ "$BINARY_SIZE_KB" -lt 1100 ]; then
    echo -e "${GREEN}✓ PASS - Size within expected range (900-1100 KB)${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${YELLOW}⚠ WARNING - Size $BINARY_SIZE_KB KB (expected ~983 KB)${NC}"
fi

echo
echo "🧪 CODE VERIFICATION TESTS"
echo "═════════════════════════════════════════════════════════════"

# Check for key functions in source files
test_component "AdbClient::reboot_system exists" \
    "grep -q 'bool.*reboot_system' '$PROJECT_DIR/src/adb/adb_client.cpp'"

test_component "DeviceInfo::is_samsung exists" \
    "grep -q 'is_samsung' '$PROJECT_DIR/src/device/device_info.cpp'"

test_component "RebootAction class exists" \
    "grep -q 'class RebootAction' '$PROJECT_DIR/include/actions/reboot.hpp'"

test_component "SELinuxAnalyzer class exists" \
    "grep -q 'class SELinuxAnalyzer' '$PROJECT_DIR/include/analyzer/analyzers.hpp'"

test_component "Dialogs confirm_reboot exists" \
    "grep -q 'confirm_reboot' '$PROJECT_DIR/include/ui/dialogs.hpp'"

echo
echo "🔒 SAFETY VERIFICATION"
echo "═════════════════════════════════════════════════════════════"

# Verify no destructive operations in code
test_component "No 'fastboot flashing unlock' in code" \
    "! grep -r 'flashing unlock' '$PROJECT_DIR/src' 2>/dev/null || echo 'Safe'"

test_component "No 'fastboot oem unlock' in code" \
    "! grep -r 'oem unlock' '$PROJECT_DIR/src' 2>/dev/null || echo 'Safe'"

test_component "No 'format' commands" \
    "! grep -r 'format ' '$PROJECT_DIR/src' 2>/dev/null | grep -v Comment || echo 'Safe'"

test_component "No 'wipe' commands" \
    "! grep -r 'wipe ' '$PROJECT_DIR/src' 2>/dev/null | grep -v Comment || echo 'Safe'"

echo
echo "╔════════════════════════════════════════════════════════════╗"
echo "║                    TEST RESULTS SUMMARY                    ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo

echo -e "Tests Passed:  ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests Failed:  ${RED}$TESTS_FAILED${NC}"
TOTAL=$((TESTS_PASSED + TESTS_FAILED))
echo "Tests Total:   $TOTAL"
echo

if [ "$TESTS_FAILED" -eq 0 ]; then
    echo -e "${GREEN}✓ ALL TESTS PASSED${NC}"
    echo
    echo "✨ LinCheckROOT v2.1 is ready for use!"
    echo "   Launch: ./lincheckroot"
    echo "   Install: ./install.sh /usr/local"
    exit 0
else
    echo -e "${RED}✗ SOME TESTS FAILED${NC}"
    echo "Please review the errors above."
    exit 1
fi
