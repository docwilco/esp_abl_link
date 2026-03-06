#!/bin/bash
# Build the basic example for all ESP32 targets and report results
#
# Usage: ./scripts/build_all_targets.sh [ESP-IDF export.sh path]
# Example: ./scripts/build_all_targets.sh ~/esp/esp-idf-v5.5/export.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
EXAMPLE_DIR="$PROJECT_ROOT/examples/basic"

# Track results
declare -A RESULTS

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Source ESP-IDF if path provided
if [[ -n "$1" ]]; then
    echo "Sourcing ESP-IDF from: $1"
    unset IDF_PATH
    unset IDF_PYTHON_ENV_PATH
    source "$1"
elif ! command -v idf.py &> /dev/null; then
    echo "Error: idf.py not found. Please source ESP-IDF export.sh or provide path as argument."
    exit 1
fi

echo "ESP-IDF version: $(idf.py --version)"

# Get targets from ESP-IDF
mapfile -t TARGETS < <(idf.py --list-targets)
echo "Available targets: ${TARGETS[*]}"
echo "Building example in: $EXAMPLE_DIR"
echo ""

cd "$EXAMPLE_DIR"

for target in "${TARGETS[@]}"; do
    echo "=============================================="
    echo "Building for: $target"
    echo "=============================================="
    
    # Clean build directory
    rm -rf build managed_components
    
    # Try to build
    if idf.py set-target "$target" && idf.py build; then
        RESULTS[$target]="SUCCESS"
        echo -e "${GREEN}✓ $target: BUILD SUCCESS${NC}"
    else
        RESULTS[$target]="FAILED"
        echo -e "${RED}✗ $target: BUILD FAILED${NC}"
    fi
    echo ""
done

# Print summary
echo ""
echo "=============================================="
echo "BUILD SUMMARY"
echo "=============================================="

SUCCESS_COUNT=0
FAIL_COUNT=0

for target in "${TARGETS[@]}"; do
    result="${RESULTS[$target]}"
    if [[ "$result" == "SUCCESS" ]]; then
        echo -e "${GREEN}✓ $target${NC}"
        ((SUCCESS_COUNT++))
    else
        echo -e "${RED}✗ $target${NC}"
        ((FAIL_COUNT++))
    fi
done

echo ""
echo "Total: $SUCCESS_COUNT succeeded, $FAIL_COUNT failed"

# Exit with error if any failed
if [[ $FAIL_COUNT -gt 0 ]]; then
    exit 1
fi
