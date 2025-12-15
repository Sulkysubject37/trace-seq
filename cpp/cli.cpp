#include <iostream>
#include <vector>
#include <filesystem>
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#include <limits.h>
#elif defined(__linux__)
#include <unistd.h> // For readlink
#include <limits.h> // For PATH_MAX
#endif
#include "cxxopts.hpp"
#include "hashing.hpp"
#include "tracer.hpp"
#include "nlohmann/json.hpp" // For load_index and resolve_lineage

namespace fs = std::filesystem;

// Forward declarations for functions implemented in lineage.cpp
nlohmann::json load_index(const fs::path& project_root);
std::vector<TraceNode> resolve_lineage(const std::string& trace_id, const fs::path& project_root);

// Function to get the project root path
fs::path get_project_root_path(const char* argv0) {
    fs::path executable_path;
    // Get the path to the executable
#if defined(__APPLE__)
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        executable_path = fs::path(path);
    } else {
        executable_path = fs::current_path() / argv0;
    }
#elif defined(__linux__)
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count != -1) {
        executable_path = fs::path(std::string(path, count));
    } else {
        executable_path = fs::current_path() / argv0;
    }
#else
    // Generic fallback for other OSes
    executable_path = fs::current_path() / argv0;
#endif

    fs::path current_dir = fs::canonical(executable_path).parent_path();
    while (!current_dir.empty() && current_dir != current_dir.parent_path()) {
        if (fs::exists(current_dir / "prompt.md")) {
            return current_dir;
        }
        current_dir = current_dir.parent_path();
    }
    
    // Fallback if root is not found
    return fs::current_path();
}


// Forward declarations
void annotate(const cxxopts::ParseResult& result, const fs::path& project_root);
void explain(const cxxopts::ParseResult& result, const fs::path& project_root);
void diff(const cxxopts::ParseResult& result, const fs::path& project_root);
void validate(const cxxopts::ParseResult& result, const fs::path& project_root);

int main(int argc, char** argv) {
    fs::path project_root = get_project_root_path(argv[0]);

    cxxopts::Options options("trace-seq", "Minimal Semantic Provenance Tracking");

    options.add_options()
        ("a,annotate", "Annotate a file with a new trace", cxxopts::value<std::string>())
        ("e,explain", "Explain the provenance of a file", cxxopts::value<std::string>())
        ("d,diff", "Diff two files", cxxopts::value<std::vector<std::string>>())
        ("v,validate", "Validate the provenance of a file", cxxopts::value<std::string>())
        ("operation", "Operation class (e.g., normalization, filtering)", cxxopts::value<std::string>())
        ("method", "Operation method (e.g., TPM, DESeq2, GATK_HaplotypeCaller)", cxxopts::value<std::string>())
        ("assumption", "Assumption", cxxopts::value<std::vector<std::string>>())
        ("parent", "Parent trace ID", cxxopts::value<std::string>())
        ("h,help", "Print usage");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }
    if (result.count("annotate") || result.count("explain") || result.count("diff") || result.count("validate")) {
        if (result.count("annotate")) {
            // Ensure required options for annotate are present
            if (!result.count("operation") || !result.count("method")) {
                std::cerr << "Error: --operation and --method are required for annotate command." << std::endl;
                std::cout << options.help() << std::endl;
                return 1;
            }
            annotate(result, project_root);
        } else if (result.count("explain")) {
            explain(result, project_root);
        } else if (result.count("diff")) {
            diff(result, project_root);
        } else if (result.count("validate")) {
            validate(result, project_root);
        }
    } else {
        std::cout << options.help() << std::endl;
        return 1;
    }

    return 0;
}

void annotate(const cxxopts::ParseResult& result, const fs::path& project_root) {
    std::string filepath = result["annotate"].as<std::string>();
    std::string operation_class = result["operation"].as<std::string>();
    std::string operation_method = result["method"].as<std::string>();
    std::vector<std::string> assumptions;
    if (result.count("assumption")) {
        assumptions = result["assumption"].as<std::vector<std::string>>();
    }

    std::string parent_id = "null";
    if (result.count("parent")) {
        parent_id = result["parent"].as<std::string>();
    }

    // 1. Calculate checksum for input file
    std::string input_checksum;
    try {
        input_checksum = sha256_file(filepath);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;
    }

    // 3. Load ontology
    Ontology ontology;
    try {
        // Construct absolute paths to ontology files
        std::string op_ontology_path = (project_root / "core" / "operation_ontology.yaml").string();
        std::string assump_ontology_path = (project_root / "core" / "assumption_ontology.yaml").string();
        ontology.load(op_ontology_path, assump_ontology_path);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error loading ontology: " << e.what() << std::endl;
        return;
    }

    // 4. Validate operation and assumptions
    if (!ontology.validate_operation(operation_class)) {
        std::cerr << "Error: Invalid operation class '" << operation_class << "'" << std::endl;
        return;
    }
    for (const auto& assump : assumptions) {
        if (!ontology.validate_assumption(assump)) { // Use full assumption string for validation
            std::cerr << "Error: Invalid assumption '" << assump << "'" << std::endl;
            return;
        }
    }

    // 5. Create TraceNode
    TraceNode node = create_trace_node(
        parent_id,
        "quantitative_matrix", // Placeholder: input data_class needs to be dynamic
        operation_class,
        operation_method,
        assumptions
    );
    // Set input details
    node.input.checksum = input_checksum;
    node.input.shape = "unknown"; // Placeholder: input shape needs to be dynamic
    node.output.data_class = "quantitative_matrix"; // Placeholder: output data_class needs to be dynamic

    // 6. Save TraceNode
    // Assuming output file is the same as input file for simplicity in annotation
    // In a real pipeline, a new file would be created, and its checksum passed.
    std::string output_checksum = input_checksum; // Placeholder
    std::string output_data_class = "quantitative_matrix"; // Placeholder
    node.save(input_checksum, output_checksum, output_data_class, project_root);

    std::cout << "Successfully annotated " << filepath << " with trace ID: " << node.trace_id << std::endl;
}

void explain(const cxxopts::ParseResult& result, const fs::path& project_root) {
    std::string filepath = result["explain"].as<std::string>();

    // 1. Calculate checksum for input file
    std::string input_checksum;
    try {
        input_checksum = sha256_file(filepath);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;
    }

    // 2. Load index.json to find the trace_id
    nlohmann::json index_json = load_index(project_root); 

    if (!index_json.contains(input_checksum)) {
        std::cout << "No provenance found for file: " << filepath << std::endl;
        return;
    }

    std::string latest_trace_id = index_json[input_checksum].get<std::string>();

    // 3. Resolve lineage
    std::vector<TraceNode> lineage = resolve_lineage(latest_trace_id, project_root);

    // 4. Print lineage details
    std::cout << "Provenance for " << filepath << ":" << std::endl;
    for (size_t i = 0; i < lineage.size(); ++i) {
        const auto& node = lineage[i];
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "Step " << i + 1 << ":" << std::endl;
        std::cout << "  Trace ID: " << node.trace_id << std::endl;
        std::cout << "  Parent ID: " << node.parent << std::endl;
        std::cout << "  Timestamp: " << node.timestamp << std::endl;
        std::cout << "  Input Data Class: " << node.data_class << std::endl;
        std::cout << "  Operation Class: " << node.operation.op_class << std::endl;
        std::cout << "  Operation Method: " << node.operation.method << std::endl;
        std::cout << "  Assumptions:" << std::endl;
        for (const auto& assump : node.assumptions) {
            std::cout << "    - " << assump << std::endl;
        }
        std::cout << "  Input Checksum: " << node.input.checksum << std::endl;
        std::cout << "  Output Checksum: " << node.output.checksum << std::endl;
        std::cout << "  Output Data Class: " << node.output.data_class << std::endl;
        std::cout << "  Environment: " << node.environment.language << "/" << node.environment.tool << " v" << node.environment.version << std::endl;
        std::cout << "  Ontology Version: " << node.ontology_version << std::endl;
    }
    std::cout << "----------------------------------------" << std::endl;
}

void diff(const cxxopts::ParseResult& result, const fs::path& project_root) {
    std::vector<std::string> files = result["diff"].as<std::vector<std::string>>();
    if (files.size() != 2) {
        std::cerr << "Error: diff command requires exactly two filepaths." << std::endl;
        return;
    }
    std::string file_a = files[0];
    std::string file_b = files[1];

    // 1. Calculate checksums
    std::string checksum_a, checksum_b;
    try {
        checksum_a = sha256_file(file_a);
        checksum_b = sha256_file(file_b);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;
    }

    // 2. Load index.json
    nlohmann::json index_json = load_index(project_root);

    if (!index_json.contains(checksum_a)) {
        std::cout << "No provenance found for file A: " << file_a << std::endl;
        return;
    }
    if (!index_json.contains(checksum_b)) {
        std::cout << "No provenance found for file B: " << file_b << std::endl;
        return;
    }

    std::string trace_id_a = index_json[checksum_a].get<std::string>();
    std::string trace_id_b = index_json[checksum_b].get<std::string>();

    // 3. Resolve lineages
    std::vector<TraceNode> lineage_a = resolve_lineage(trace_id_a, project_root);
    std::vector<TraceNode> lineage_b = resolve_lineage(trace_id_b, project_root);

    std::cout << "--- Diffing Provenance ---" << std::endl;
    std::cout << "File A: " << file_a << std::endl;
    std::cout << "File B: " << file_b << std::endl;

    if (lineage_a.size() != lineage_b.size()) {
        std::cout << "Difference: Lineage lengths differ (File A: " << lineage_a.size() << ", File B: " << lineage_b.size() << ")" << std::endl;
    }

    size_t min_size = std::min(lineage_a.size(), lineage_b.size());
    for (size_t i = 0; i < min_size; ++i) {
        const auto& node_a = lineage_a[i];
        const auto& node_b = lineage_b[i];

        std::cout << "\nStep " << i + 1 << ":" << std::endl;
        bool step_diff = false;

        if (node_a.operation.op_class != node_b.operation.op_class) {
            std::cout << "  - Operation Class: A='" << node_a.operation.op_class << "', B='" << node_b.operation.op_class << "'" << std::endl;
            step_diff = true;
        }
        if (node_a.operation.method != node_b.operation.method) {
            std::cout << "  - Operation Method: A='" << node_a.operation.method << "', B='" << node_b.operation.method << "'" << std::endl;
            step_diff = true;
        }
        if (node_a.assumptions != node_b.assumptions) {
            std::cout << "  - Assumptions Differ" << std::endl;
            step_diff = true;
        }
        // Add more comparisons as needed (e.g., input/output data_class, environment)

        if (!step_diff) {
            std::cout << "  (No semantic differences at this step)" << std::endl;
        }
    }

    if (lineage_a.size() > min_size) {
        std::cout << "\nFile A has additional steps beyond step " << min_size << std::endl;
    }
    if (lineage_b.size() > min_size) {
        std::cout << "\nFile B has additional steps beyond step " << min_size << std::endl;
    }
    std::cout << "----------------------------------------" << std::endl;
}

void validate(const cxxopts::ParseResult& result, const fs::path& project_root) {
    std::string filepath = result["validate"].as<std::string>();

    // 1. Calculate checksum for input file
    std::string input_checksum;
    try {
        input_checksum = sha256_file(filepath);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;
    }

    // 2. Load index.json to find the trace_id
    nlohmann::json index_json = load_index(project_root);

    if (!index_json.contains(input_checksum)) {
        std::cout << "No provenance found for file: " << filepath << std::endl;
        return;
    }

    std::string latest_trace_id = index_json[input_checksum].get<std::string>();

    // 3. Load ontology
    Ontology ontology;
    try {
        // Construct absolute paths to ontology files
        std::string op_ontology_path = (project_root / "core" / "operation_ontology.yaml").string();
        std::string assump_ontology_path = (project_root / "core" / "assumption_ontology.yaml").string();
        ontology.load(op_ontology_path, assump_ontology_path);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error loading ontology: " << e.what() << std::endl;
        return;
    }

    // 4. Resolve lineage
    std::vector<TraceNode> lineage = resolve_lineage(latest_trace_id, project_root);

    if (lineage.empty()) {
        std::cout << "No lineage found for file: " << filepath << std::endl;
        return;
    }

    bool all_valid = true;
    std::string prev_trace_id = ""; // To check parent-child link

    for (size_t i = 0; i < lineage.size(); ++i) {
        const auto& node = lineage[i];
        std::cout << "Validating Step " << i + 1 << " (Trace ID: " << node.trace_id << "): ";

        // 5. Validate node against ontology and schema
        if (!validate_node(node, ontology)) {
            all_valid = false;
            std::cout << "[FAILED]" << std::endl;
            // Error messages are printed by validate_node
        } else {
            std::cout << "[PASSED]" << std::endl;
        }

        // 6. Check lineage integrity
        if (i > 0) {
            if (node.parent != prev_trace_id) {
                all_valid = false;
                std::cout << "  [FAILED] Lineage integrity check: Parent ID mismatch. Expected '"
                          << prev_trace_id << "', got '" << node.parent << "'" << std::endl;
            }
        }
        prev_trace_id = node.trace_id;
    }

    if (all_valid) {
        std::cout << "\nValidation successful for " << filepath << ": All trace nodes and lineage are valid." << std::endl;
    } else {
        std::cout << "\nValidation failed for " << filepath << ": Issues found in trace nodes or lineage." << std::endl;
    }
}
