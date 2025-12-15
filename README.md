# TRACE-SEQ

**Minimal Semantic Provenance Tracking for Bioinformatics Pipelines**

TRACE-SEQ is a system designed for tracking the semantic provenance of data within bioinformatics pipelines. It aims to provide a lightweight, cross-platform, and user-friendly solution for understanding and verifying data transformations. Unlike systems that merely log code execution, TRACE-SEQ focuses on capturing the *meaning* of each data manipulation step, ensuring a higher level of auditability and reproducibility.

## Key Features

TRACE-SEQ's design is driven by the principle of **Semantic Provenance**, capturing the *meaning* behind data transformations. This core concept enables several powerful features:

*   **Fixed, Extensible Ontology:** All operations and assumptions are defined within a clear, extensible ontology. This ensures consistency in provenance records and allows for standardized semantic interpretation across different analyses.
*   **State-Based Provenance via Cryptographic Hashes:** The semantic state of a dataset at each step is recorded and linked using cryptographic hashes (SHA256). This creates an immutable and verifiable lineage, allowing users to trace data origins and transformations with high confidence, directly contributing to **Reproducibility**.
*   **Cross-Language Integration:** TRACE-SEQ's core logic is implemented in high-performance C++, providing robust and efficient provenance tracking. This core is exposed through user-friendly interfaces in Python and R, catering to the diverse ecosystem of bioinformatics tools and users.
*   **Human-Readable Provenance Records:** Provenance information is stored in human-readable YAML files, making it easy to inspect, audit, and understand the history of any data artifact without specialized tools, thereby enhancing **Auditability**.
*   **Detailed Lineage Reconstruction:** The system can reconstruct the complete lineage of any derived data file, showing all operations, assumptions, and parent-child relationships in the transformation chain. This enables clear understanding of data derivation and facilitates **Semantic Comparability** between datasets.

## Getting Started

To get TRACE-SEQ up and running on your system, follow these steps:

### Documentation

For more detailed information on TRACE-SEQ's concepts, ontology, and reproducibility aspects, please refer to the `docs` directory:

*   [`docs/concepts.md`](docs/concepts.md): Deep dive into Semantic Provenance and why TRACE-SEQ is needed.
*   [`docs/ontology.md`](docs/ontology.md): Explanation of TRACE-SEQ's fixed ontology for operations and assumptions.
*   [`docs/reproducibility.md`](docs/reproducibility.md): How TRACE-SEQ enhances scientific reproducibility.


### 1. Clone the Repository

First, clone the TRACE-SEQ repository to your local machine:

```bash
git clone https://github.com/your-username/trace-seq.git
cd trace-seq
```

### 2. Set up Python Virtual Environment

It's highly recommended to use a Python virtual environment to manage dependencies:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install pybind11 nlohmann-json
```

### 3. Build C++ Core and Python Bindings

The C++ core and its Python bindings are built using CMake:

```bash
mkdir -p cpp/build
cd cpp/build
cmake -DCMAKE_PREFIX_PATH="$(pwd)/../../.venv/lib/python3.14/site-packages/pybind11/share/cmake/pybind11" ..
make
```
*(Note: Adjust `python3.14` to your Python version if different)*

After a successful build, copy the generated Python module to the `python/traceseq` directory:

```bash
cp cpp/build/traceseq_py.cpython-314-darwin.so ../../python/traceseq/
```
*(Note: The `.so` file name might vary based on your OS and Python version)*

### 4. R Package Setup

For R functionality, ensure you have the necessary packages installed:

```R
install.packages(c("yaml", "jsonlite", "digest", "rprojroot", "uuid", "whisker", "testthat", "knitr"), repos = "http://cran.us.r-project.org")
```

## Usage

TRACE-SEQ provides interfaces for C++ (CLI), Python, and R.

### C++ Command-Line Interface (CLI)

The `traceseq` executable (located at `cpp/build/traceseq`) allows direct interaction:

```bash
# Annotate a file
./cpp/build/traceseq --annotate /path/to/data.tsv \
    --operation="normalization" --method="TPM" \
    --assumption="library_preparation:polyA_selected" \
    --parent="<optional_parent_trace_id>"

# Explain provenance
./cpp/build/traceseq --explain /path/to/data.tsv

# Diff provenance of two files
./cpp/build/traceseq --diff /path/to/file1.tsv /path/to/file2.tsv

# Validate provenance
./cpp/build/traceseq --validate /path/to/data.tsv
```

### Python Library

After building the Python bindings, you can use TRACE-SEQ directly in your Python scripts:

```python
import traceseq
import os

# Create a dummy file
with open("my_data.txt", "w") as f:
    f.write("Some data content.")

# Annotate the file
trace_id = traceseq.annotate(
    "my_data.txt",
    operation="preprocessing",
    method="cleanup",
    assumptions=["data_quality:high"],
    parent_id="null"
)
print(f"Annotated with trace ID: {trace_id}")

# Explain lineage
lineage = traceseq.explain("my_data.txt")
for step in lineage:
    print(f"Step: {step.operation.op_class} - {step.operation.method}, ID: {step.trace_id}")

# Clean up
os.remove("my_data.txt")
```

### R Library

Utilize TRACE-SEQ functions within your R scripts or interactive sessions:

```R
# Source the R initialization file
source("r/init.R")

# Create a dummy file
test_file <- "my_r_data.txt"
writeLines("R data content.", test_file)

# Annotate the file
trace_id <- annotate_r(
  filepath = test_file,
  operation_class = "analysis",
  operation_method = "descriptive_stats",
  assumptions = list("data_distribution:normal"),
  project_root = project_root # 'project_root' comes from init.R
)
print(paste("Annotated with trace ID:", trace_id))

# Resolve lineage
lineage <- resolve_lineage_r(filepath = test_file, project_root = project_root)
for (i in seq_along(lineage)) {
  node <- lineage[[i]]
  print(paste0("Step ", i, ": ", node$operation$class, " - ", node$operation$method, ", ID: ", node$trace_id))
}

# Generate a report (feature of R library)
# generate_lineage_report(lineage, output_file = "my_report.md")

# Clean up
file.remove(test_file)
```

## Project Structure

```
.
├── core/                        # Core ontology definitions (YAML files)
├── cpp/                         # C++ core library, CLI, and pybind11 bindings
│   ├── build/                   # CMake build directory (executables, Python module)
│   ├── bindings.cpp             # pybind11 bindings for Python
│   ├── CMakeLists.txt           # CMake build configuration
│   ├── cli.cpp                  # C++ command-line interface implementation
│   ├── hashing.cpp/hpp          # SHA256 hashing utility
│   ├── lineage.cpp/hpp          # Functions for managing trace lineage
│   └── tracer.cpp/hpp           # Core TraceNode and Ontology classes
├── docs/                        # Project documentation (concepts, FAQ, etc.)
├── examples/                    # Example usage scripts and data
│   └── rnaseq/                  # RNA-seq pipeline examples
├── python/                      # Python interface and utility scripts
│   ├── traceseq/                # Python package
│   │   ├── __init__.py          # Imports core functionality
│   │   └── api.py               # High-level Python API
│   └── tests/                   # Python unit tests
├── r/                           # R interface and utility scripts
│   ├── init.R                   # R initialization script (defines project_root, sources other R files)
│   ├── tests/                   # R unit tests
│   ├── trace_annotate.R         # R function for annotating files
│   ├── trace_load.R             # R functions for loading trace nodes and index
│   ├── trace_ontology.R         # R functions for loading and validating ontologies
│   ├── trace_report.R           # R function for generating provenance reports
│   └── trace_validate.R         # R functions for validating trace nodes and lineage
└── README.md                    # This README file
```

## Contributing

We welcome contributions to TRACE-SEQ! Please see our `CONTRIBUTING.md` (to be added) for more details on how to get involved.

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.