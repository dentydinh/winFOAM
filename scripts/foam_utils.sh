#!/bin/bash
# winFOAM Utility Functions for OpenFOAM Operations
# Source this file: source /path/to/foam_utils.sh

set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Log functions
log_info() { echo -e "${BLUE}[INFO]${NC} $*"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $*"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }
log_error() { echo -e "${RED}[ERROR]${NC} $*"; }

# Check if OpenFOAM is sourced
check_foam() {
    if [[ -z "${WM_PROJECT_VERSION:-}" ]]; then
        log_error "OpenFOAM not sourced. Run: source /opt/openfoam10/etc/bashrc"
        return 1
    fi
    return 0
}

# Run blockMesh with logging
run_blockMesh() {
    local case_dir="${1:-.}"
    check_foam || return 1

    log_info "Running blockMesh in $case_dir"
    (cd "$case_dir" && blockMesh 2>&1 | tee log.blockMesh)
    local ret=${PIPESTATUS[0]}
    if [[ $ret -eq 0 ]]; then
        log_success "blockMesh completed"
    else
        log_error "blockMesh failed (exit code $ret)"
    fi
    return $ret
}

# Run snappyHexMesh with logging
run_snappyHexMesh() {
    local case_dir="${1:-.}"
    local overwrite="${2:-false}"
    check_foam || return 1

    local args="-overwrite"
    [[ "$overwrite" != "true" ]] && args=""

    log_info "Running snappyHexMesh $args in $case_dir"
    (cd "$case_dir" && snappyHexMesh $args 2>&1 | tee log.snappyHexMesh)
    local ret=${PIPESTATUS[0]}
    if [[ $ret -eq 0 ]]; then
        log_success "snappyHexMesh completed"
    else
        log_error "snappyHexMesh failed (exit code $ret)"
    fi
    return $ret
}

# Run checkMesh with logging
run_checkMesh() {
    local case_dir="${1:-.}"
    check_foam || return 1

    log_info "Running checkMesh in $case_dir"
    (cd "$case_dir" && checkMesh 2>&1 | tee log.checkMesh)
    local ret=${PIPESTATUS[0]}
    if [[ $ret -eq 0 ]]; then
        log_success "checkMesh completed"
    else
        log_error "checkMesh failed (exit code $ret)"
    fi
    return $ret
}

# Run solver with logging
run_solver() {
    local solver="${1:-simpleFoam}"
    local case_dir="${2:-.}"
    local parallel="${3:-false}"
    local np="${4:-4}"
    check_foam || return 1

    local cmd="$solver"
    if [[ "$parallel" == "true" ]]; then
        cmd="mpirun -np $np $solver -parallel"
    fi

    log_info "Running $cmd in $case_dir"
    (cd "$case_dir" && eval $cmd 2>&1 | tee "log.$solver")
    local ret=${PIPESTATUS[0]}
    if [[ $ret -eq 0 ]]; then
        log_success "$solver completed"
    else
        log_error "$solver failed (exit code $ret)"
    fi
    return $ret
}

# Decompose case for parallel run
decompose_case() {
    local case_dir="${1:-.}"
    local force="${2:-false}"
    check_foam || return 1

    local args=""
    [[ "$force" == "true" ]] && args="-force"

    log_info "Running decomposePar $args in $case_dir"
    (cd "$case_dir" && decomposePar $args 2>&1 | tee log.decomposePar)
    local ret=${PIPESTATUS[0]}
    if [[ $ret -eq 0 ]]; then
        log_success "decomposePar completed"
    else
        log_error "decomposePar failed (exit code $ret)"
    fi
    return $ret
}

# Reconstruct case from parallel run
reconstruct_case() {
    local case_dir="${1:-.}"
    local latest="${2:-true}"
    check_foam || return 1

    local args=""
    [[ "$latest" == "true" ]] && args="-latestTime"

    log_info "Running reconstructPar $args in $case_dir"
    (cd "$case_dir" && reconstructPar $args 2>&1 | tee log.reconstructPar)
    local ret=${PIPESTATUS[0]}
    if [[ $ret -eq 0 ]]; then
        log_success "reconstructPar completed"
    else
        log_error "reconstructPar failed (exit code $ret)"
    fi
    return $ret
}

# Convert to VTK for ParaView
convert_to_vtk() {
    local case_dir="${1:-.}"
    check_foam || return 1

    log_info "Running foamToVTK in $case_dir"
    (cd "$case_dir" && foamToVTK 2>&1 | tee log.foamToVTK)
    local ret=${PIPESTATUS[0]}
    if [[ $ret -eq 0 ]]; then
        log_success "foamToVTK completed"
    else
        log_error "foamToVTK failed (exit code $ret)"
    fi
    return $ret
}

# Post-process with function objects
post_process() {
    local func="${1:-sample}"
    local case_dir="${2:-.}"
    local time="${3:-latestTime}"
    check_foam || return 1

    log_info "Running postProcess -func $func -time $time in $case_dir"
    (cd "$case_dir" && postProcess -func "$func" -time "$time" 2>&1 | tee "log.postProcess.$func")
    local ret=${PIPESTATUS[0]}
    if [[ $ret -eq 0 ]]; then
        log_success "postProcess completed"
    else
        log_error "postProcess failed (exit code $ret)"
    fi
    return $ret
}

# Extract residuals from log file
extract_residuals() {
    local log_file="${1:-log.simpleFoam}"
    if [[ ! -f "$log_file" ]]; then
        log_error "Log file not found: $log_file"
        return 1
    fi

    grep -E "Solving for|Initial residual|Final residual" "$log_file" | \
    awk '
    /Solving for/ { field=$3 }
    /Initial residual/ { init=$4; final=$8; print field, init, final }
    '
}

# Monitor log file in real-time
monitor_log() {
    local log_file="${1:-log.simpleFoam}"
    tail -f "$log_file" | grep --line-buffered -E "Time =|Solving for|Initial residual|Final residual|ExecutionTime"
}

# Create standard case structure
create_case_structure() {
    local case_dir="${1:-.}"
    log_info "Creating case structure in $case_dir"
    mkdir -p "$case_dir"/{0,constant,system}
    mkdir -p "$case_dir"/constant/polyMesh
    touch "$case_dir"/0/.gitkeep
    log_success "Case structure created"
}

# Clean case (remove processor dirs, logs, etc.)
clean_case() {
    local case_dir="${1:-.}"
    log_info "Cleaning case in $case_dir"
    (cd "$case_dir" && \
        rm -rf processor* log.* *.log postProcessing VTK *.vtk *.foam 2>/dev/null; \
        find . -name "*~" -delete; \
        find . -name "#*#" -delete)
    log_success "Case cleaned"
}

# Export functions
export -f check_foam run_blockMesh run_snappyHexMesh run_checkMesh
export -f run_solver decompose_case reconstruct_case
export -f convert_to_vtk post_process extract_residuals monitor_log
export -f create_case_structure clean_case

log_info "winFOAM foam_utils.sh loaded. Available functions:"
declare -F | grep -v "^declare -f _" | awk '{print "  " $3}'