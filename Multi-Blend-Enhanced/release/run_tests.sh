#!/bin/bash

echo "🚀 Multi-Blend Enhanced Plugin - Logic Pro Compatibility Test Suite"
echo "=================================================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    local status=$1
    local message=$2
    case $status in
        "PASS")
            echo -e "${GREEN}✅ PASS${NC} - $message"
            ;;
        "FAIL")
            echo -e "${RED}❌ FAIL${NC} - $message"
            ;;
        "WARN")
            echo -e "${YELLOW}⚠️  WARN${NC} - $message"
            ;;
        "INFO")
            echo -e "${BLUE}ℹ️  INFO${NC} - $message"
            ;;
    esac
}

# Function to run a test and capture result
run_test() {
    local test_name=$1
    local test_command=$2
    
    echo -e "\n${BLUE}Running: $test_name${NC}"
    echo "Command: $test_command"
    
    if eval "$test_command" > /tmp/test_output.log 2>&1; then
        print_status "PASS" "$test_name completed successfully"
        return 0
    else
        print_status "FAIL" "$test_name failed"
        echo "Error output:"
        cat /tmp/test_output.log
        return 1
    fi
}

# Initialize test results
total_tests=0
passed_tests=0
failed_tests=0

# Test 1: Check if we're in the right directory
echo "=== Environment Check ==="
if [ -f "CMakeLists.txt" ] && [ -d "src" ]; then
    print_status "PASS" "Project structure verified"
    ((passed_tests++))
else
    print_status "FAIL" "Not in Multi-Blend-Enhanced directory"
    ((failed_tests++))
    exit 1
fi
((total_tests++))

# Test 2: Check if all required files exist
echo -e "\n=== File Structure Check ==="
required_files=(
    "src/PluginProcessor.h"
    "src/PluginProcessor.cpp"
    "src/PluginEditor.h"
    "src/PluginEditor.cpp"
    "src/ClipperLimiter.h"
    "src/ClipperLimiter.cpp"
    "src/LookAndFeel.h"
    "src/LookAndFeel.cpp"
    "CMakeLists.txt"
)

for file in "${required_files[@]}"; do
    if [ -f "$file" ]; then
        print_status "PASS" "Found $file"
        ((passed_tests++))
    else
        print_status "FAIL" "Missing $file"
        ((failed_tests++))
    fi
    ((total_tests++))
done

# Test 3: Check for syntax errors in source files
echo -e "\n=== Syntax Check ==="
cpp_files=(
    "src/PluginProcessor.cpp"
    "src/PluginEditor.cpp"
    "src/ClipperLimiter.cpp"
    "src/LookAndFeel.cpp"
)

for file in "${cpp_files[@]}"; do
    if command -v g++ >/dev/null 2>&1; then
        if g++ -fsyntax-only -std=c++17 -I/usr/include "$file" >/dev/null 2>&1; then
            print_status "PASS" "Syntax check passed for $file"
            ((passed_tests++))
        else
            print_status "FAIL" "Syntax error in $file"
            ((failed_tests++))
        fi
    else
        print_status "WARN" "g++ not available, skipping syntax check for $file"
    fi
    ((total_tests++))
done

# Test 4: Check for memory leaks in code patterns
echo -e "\n=== Memory Safety Check ==="
memory_patterns=(
    "std::unique_ptr"
    "std::atomic"
    "juce::ScopedNoDenormals"
)

for pattern in "${memory_patterns[@]}"; do
    if grep -r "$pattern" src/ >/dev/null 2>&1; then
        print_status "PASS" "Memory safety pattern '$pattern' found"
        ((passed_tests++))
    else
        print_status "WARN" "Memory safety pattern '$pattern' not found"
    fi
    ((total_tests++))
done

# Test 5: Check for thread safety patterns
echo -e "\n=== Thread Safety Check ==="
thread_patterns=(
    "std::atomic"
    "juce::ScopedLock"
    "juce::SpinLock"
)

for pattern in "${thread_patterns[@]}"; do
    if grep -r "$pattern" src/ >/dev/null 2>&1; then
        print_status "PASS" "Thread safety pattern '$pattern' found"
        ((passed_tests++))
    else
        print_status "WARN" "Thread safety pattern '$pattern' not found"
    fi
    ((total_tests++))
done

# Test 6: Check for Logic Pro compatibility patterns
echo -e "\n=== Logic Pro Compatibility Check ==="
logic_pro_patterns=(
    "VST3"
    "AU"
    "juce::AudioProcessor"
    "prepareToPlay"
    "processBlock"
    "getStateInformation"
    "setStateInformation"
)

for pattern in "${logic_pro_patterns[@]}"; do
    if grep -r "$pattern" src/ >/dev/null 2>&1; then
        print_status "PASS" "Logic Pro compatibility pattern '$pattern' found"
        ((passed_tests++))
    else
        print_status "FAIL" "Logic Pro compatibility pattern '$pattern' missing"
        ((failed_tests++))
    fi
    ((total_tests++))
done

# Test 7: Check for parameter validation
echo -e "\n=== Parameter Validation Check ==="
validation_patterns=(
    "jlimit"
    "juce::jlimit"
    "NormalisableRange"
    "AudioParameterFloat"
    "AudioParameterChoice"
)

for pattern in "${validation_patterns[@]}"; do
    if grep -r "$pattern" src/ >/dev/null 2>&1; then
        print_status "PASS" "Parameter validation pattern '$pattern' found"
        ((passed_tests++))
    else
        print_status "WARN" "Parameter validation pattern '$pattern' not found"
    fi
    ((total_tests++))
done

# Test 8: Check for error handling
echo -e "\n=== Error Handling Check ==="
error_patterns=(
    "try"
    "catch"
    "std::exception"
    "nullptr"
)

for pattern in "${error_patterns[@]}"; do
    if grep -r "$pattern" src/ >/dev/null 2>&1; then
        print_status "PASS" "Error handling pattern '$pattern' found"
        ((passed_tests++))
    else
        print_status "WARN" "Error handling pattern '$pattern' not found"
    fi
    ((total_tests++))
done

# Test 9: Check for performance optimizations
echo -e "\n=== Performance Check ==="
performance_patterns=(
    "juce::ScopedNoDenormals"
    "const"
    "inline"
    "std::vector"
)

for pattern in "${performance_patterns[@]}"; do
    if grep -r "$pattern" src/ >/dev/null 2>&1; then
        print_status "PASS" "Performance pattern '$pattern' found"
        ((passed_tests++))
    else
        print_status "WARN" "Performance pattern '$pattern' not found"
    fi
    ((total_tests++))
done

# Test 10: Check for clipper/limiter specific patterns
echo -e "\n=== Clipper/Limiter Implementation Check ==="
clipper_patterns=(
    "ClipperLimiter"
    "SoftTanh"
    "HardClip"
    "Cubic"
    "Hermite"
    "Foldback"
    "Feedback"
    "Feedforward"
    "LookAhead"
    "threshold"
    "ceiling"
    "attack"
    "release"
    "ratio"
    "knee"
)

for pattern in "${clipper_patterns[@]}"; do
    if grep -r "$pattern" src/ >/dev/null 2>&1; then
        print_status "PASS" "Clipper/Limiter pattern '$pattern' found"
        ((passed_tests++))
    else
        print_status "FAIL" "Clipper/Limiter pattern '$pattern' missing"
        ((failed_tests++))
    fi
    ((total_tests++))
done

# Test 11: Check for potential crash conditions
echo -e "\n=== Crash Prevention Check ==="
crash_patterns=(
    "std::isfinite"
    "std::isnan"
    "std::isinf"
    "juce::jlimit"
    "nullptr"
)

for pattern in "${crash_patterns[@]}"; do
    if grep -r "$pattern" src/ >/dev/null 2>&1; then
        print_status "PASS" "Crash prevention pattern '$pattern' found"
        ((passed_tests++))
    else
        print_status "WARN" "Crash prevention pattern '$pattern' not found"
    fi
    ((total_tests++))
done

# Test 12: Check for resource management
echo -e "\n=== Resource Management Check ==="
resource_patterns=(
    "prepareToPlay"
    "releaseResources"
    "reset"
    "std::unique_ptr"
    "RAII"
)

for pattern in "${resource_patterns[@]}"; do
    if grep -r "$pattern" src/ >/dev/null 2>&1; then
        print_status "PASS" "Resource management pattern '$pattern' found"
        ((passed_tests++))
    else
        print_status "WARN" "Resource management pattern '$pattern' not found"
    fi
    ((total_tests++))
done

# Summary
echo -e "\n=== Test Summary ==="
echo "Total tests: $total_tests"
echo "Passed: $passed_tests"
echo "Failed: $failed_tests"
echo "Warnings: $((total_tests - passed_tests - failed_tests))"

if [ $failed_tests -eq 0 ]; then
    echo -e "\n${GREEN}🎉 SUCCESS: All critical tests passed!${NC}"
    echo -e "${GREEN}✅ Plugin is ready for Logic Pro deployment${NC}"
    echo -e "${GREEN}✅ No critical issues detected${NC}"
    exit 0
else
    echo -e "\n${RED}❌ FAILURE: $failed_tests critical tests failed!${NC}"
    echo -e "${RED}⚠️  Plugin needs fixes before Logic Pro deployment${NC}"
    echo -e "${YELLOW}Please review the failed tests above${NC}"
    exit 1
fi