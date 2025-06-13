#!/bin/bash
#
# build_all.sh
#
# Build all available board configurations to verify the build system.
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

conf_DIR="conf/boards"
PASSED=0
FAILED=0
FAILED_BOARDS=()

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "========================================"
echo "  NOVA Build All Boards"
echo "========================================"
echo

rm -fr build

# Find all board configurations
for conf in $(find "$conf_DIR" -name "*.conf" | sort); do
    # Extract arch and board from path: conf/boards/<arch>/<board>.conf
    arch=$(echo "$conf" | sed 's|.*/boards/\([^/]*\)/.*|\1|')
    board=$(basename "$conf" .conf)

    echo -n "Building ${arch}/${board}... "

    # Clean and build
    if make clean ARCH="$arch" BOARD="$board" > /dev/null 2>&1 && \
       make -j$(nproc) ARCH="$arch" BOARD="$board" > /dev/null 2>&1; then
        echo -e "${GREEN}PASS${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}FAIL${NC}"
        FAILED=$((FAILED + 1))
        FAILED_BOARDS+=("${arch}/${board}")
    fi
done

echo
echo "========================================"
echo "  Summary"
echo "========================================"
echo -e "Passed: ${GREEN}${PASSED}${NC}"
echo -e "Failed: ${RED}${FAILED}${NC}"

if [ ${#FAILED_BOARDS[@]} -gt 0 ]; then
    echo
    echo -e "${RED}Failed boards:${NC}"
    for board in "${FAILED_BOARDS[@]}"; do
        echo "  - $board"
    done
    exit 1
fi

echo
echo -e "${GREEN}All boards built successfully!${NC}"
exit 0
