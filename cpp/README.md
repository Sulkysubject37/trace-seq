# C++ Core Library and CLI

This directory contains the core C++ library for `traceseq` and its command-line interface (CLI).

## Features

*   **Hashing:** Calculates SHA256 checksums of files.
*   **Trace Node Management:**
    *   Creates and manages `TraceNode` objects, representing individual steps in a provenance chain.
    *   Stores trace nodes as YAML files in a hidden `.traceseq/nodes` directory.
*   **Ontology Validation:** Validates operations and assumptions against defined YAML ontologies.
*   **Provenance Tracking:**
    *   Maintains an `index.json` file in the `.traceseq` directory to map file checksums to trace IDs.
    *   Resolves the full lineage of a file by traversing parent trace IDs.

## Command-Line Interface (CLI)

The `traceseq` executable provides the following commands:

*   **`--annotate <filepath>`**: Annotates a file with a new trace node.
    *   Requires `--operation` and `--method`.
    *   Optional: `--assumption` (can be specified multiple times), `--parent` (trace ID of the parent node).
*   **`--explain <filepath>`**: Explains the provenance chain of a file.
*   **`--diff <filepath_a> <filepath_b>`**: Diffs the provenance chains of two files. (Note: Current implementation is simplified and only compares certain aspects).
*   **`--validate <filepath>`**: Validates the provenance chain of a file against the ontologies.

## Build Instructions

To build the C++ core and CLI, navigate to the `cpp/build` directory and run the following commands:

```bash
mkdir -p cpp/build
cd cpp/build
cmake ..
make
```

**Prerequisites:**

*   CMake (version 3.10 or higher)
*   A C++17 compliant compiler (e.g., Clang, GCC)
*   `yaml-cpp` library
*   `nlohmann/json` library
*   `OpenSSL` library (for SHA256 hashing)
*   `cxxopts` library
*   `GTest` (for running tests)
*   `uuid` library

**Note:** If you are using Homebrew on macOS, most of these dependencies can be installed via `brew install`.

## Usage Example

```bash
# Assuming you are in the project root directory
# Build the executable (if not already built)
# cd cpp/build && cmake .. && make && cd ../..

# Annotate a raw data file
./cpp/build/traceseq --annotate examples/rnaseq/GSE43335_smrna_locus.txt \
    --operation="annotation" \
    --method="GEO" \
    --assumption="data_source:GEO"

# Explain the provenance
./cpp/build/traceseq --explain examples/rnaseq/GSE43335_smrna_locus.txt
```