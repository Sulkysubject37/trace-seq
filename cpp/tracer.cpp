#include "tracer.hpp"
#include <iostream>
#include <fstream>
#include <chrono> // For std::chrono
#include <ctime>    // For std::time_t, std::tm, std::gmtime
#include <iomanip>  // For std::put_time
#include <filesystem> // For std::filesystem::create_directories
#include "nlohmann/json.hpp"
#include <uuid/uuid.h> // For UUID generation
#include "lineage.hpp"

namespace fs = std::filesystem;

// Helper to get current UTC timestamp in ISO8601 format
std::string get_current_timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t current_time = std::chrono::system_clock::to_time_t(now);
    std::tm* gmt = std::gmtime(&current_time);
    std::stringstream ss;
    ss << std::put_time(gmt, "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

std::string generate_uuid() {
    uuid_t uuid;
    uuid_generate_random(uuid);
    char uuid_str[37];
    uuid_unparse_lower(uuid, uuid_str);
    return uuid_str;
}

// TraceNode constructor
TraceNode::TraceNode() :
    trace_id(generate_uuid()),
    parent("null"),
    timestamp(get_current_timestamp()),
    data_class(""),
    ontology_version("1.0") // Assuming a version for now
{
    // Initialize nested structs
    operation.op_class = "";
    operation.method = "";
    input.shape = "";
    input.checksum = "";
    output.data_class = "";
    output.unit = "";
    output.checksum = "";
    environment.language = "cpp"; // Default to cpp
    environment.tool = "traceseq";
    environment.version = "0.1.0";
}


void TraceNode::save(const std::string& input_file_checksum, const std::string& output_file_checksum, const std::string& output_file_data_class, const fs::path& project_root) {
    // Ensure .traceseq/nodes directory exists
    fs::path nodes_dir = project_root / ".traceseq" / "nodes";
    fs::create_directories(nodes_dir);

    // Save TraceNode to YAML file
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "trace_id" << YAML::Value << trace_id;
    out << YAML::Key << "parent" << YAML::Value << parent;
    out << YAML::Key << "timestamp" << YAML::Value << timestamp;
    out << YAML::Key << "data_class" << YAML::Value << data_class;

    out << YAML::Key << "operation" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "class" << YAML::Value << operation.op_class;
    out << YAML::Key << "method" << YAML::Value << operation.method;
    if (!operation.parameters.empty()) {
        out << YAML::Key << "parameters" << YAML::Value << YAML::BeginMap;
        for (const auto& pair : operation.parameters) {
            out << YAML::Key << pair.first << YAML::Value << pair.second;
        }
        out << YAML::EndMap;
    }
    out << YAML::EndMap; // End operation

    out << YAML::Key << "assumptions" << YAML::Value << YAML::BeginSeq;
    for (const auto& assump : assumptions) {
        out << assump;
    }
    out << YAML::EndSeq; // End assumptions

    out << YAML::Key << "input" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "shape" << YAML::Value << input.shape;
    out << YAML::Key << "checksum" << YAML::Value << input.checksum;
    out << YAML::EndMap; // End input

    out << YAML::Key << "output" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "data_class" << YAML::Value << output_file_data_class; // Use passed data_class
    out << YAML::Key << "unit" << YAML::Value << output.unit;
    out << YAML::Key << "checksum" << YAML::Value << output_file_checksum; // Use passed checksum
    out << YAML::EndMap; // End output

    out << YAML::Key << "environment" << YAML::Value << YAML::BeginMap;
    out << YAML::Key << "language" << YAML::Value << environment.language;
    out << YAML::Key << "tool" << YAML::Value << environment.tool;
    out << YAML::Key << "version" << YAML::Value << environment.version;
    out << YAML::EndMap; // End environment

    out << YAML::Key << "ontology_version" << YAML::Value << ontology_version;
    out << YAML::EndMap; // End TraceNode

    std::ofstream file(nodes_dir / (trace_id + ".yaml"));
    file << out.c_str();
    file.close();

    // Update index.json
    nlohmann::json index_json = load_index(project_root);
    
    // Create an entry for the input_file_checksum pointing to the trace__id
    index_json[input_file_checksum] = trace_id;

    // Create an entry for the output_file_checksum pointing to the trace_id
    // This assumes a 1:1 relationship between output file and a single trace node
    // In a more complex scenario, an output might be influenced by multiple traces
    index_json[output_file_checksum] = trace_id;

    save_index(index_json, project_root);
}

void Ontology::load(const std::string& op_path, const std::string& assump_path) {
    operations = YAML::LoadFile(op_path);
    assumptions = YAML::LoadFile(assump_path);
}

bool Ontology::validate_operation(const std::string& op_class) const {
    return operations["operation_classes"][op_class].IsDefined();
}

bool Ontology::validate_assumption(const std::string& assump_full_str) const {
    std::string assump_class = assump_full_str;
    std::string assump_value = "";
    size_t colon_pos = assump_full_str.find(":");
    if (colon_pos != std::string::npos) {
        assump_class = assump_full_str.substr(0, colon_pos);
        assump_value = assump_full_str.substr(colon_pos + 1);
    }

    if (!assumptions["assumption_classes"][assump_class].IsDefined()) {
        return false;
    }

    // If a value is provided, check against allowed_values
    if (!assump_value.empty()) {
        const YAML::Node& allowed_values_node = assumptions["assumption_classes"][assump_class]["allowed_values"];
        if (allowed_values_node.IsDefined() && allowed_values_node.IsSequence()) {
            bool value_found = false;
            for (const auto& value_item : allowed_values_node) {
                if (value_item.as<std::string>() == assump_value) {
                    value_found = true;
                    break;
                }
            }
            if (!value_found) {
                return false;
            }
        } else {
            // No allowed_values defined, but a value was provided - this might be an error or just not explicitly allowed
            // For now, if no allowed_values, any value is considered invalid.
            return false;
        }
    }
    return true;
}

TraceNode create_trace_node(
    const std::string& parent_id,
    const std::string& input_data_class, // Renamed for clarity: this is the data_class of the input
    const std::string& operation_class,
    const std::string& operation_method,
    const std::vector<std::string>& assumptions
) {
    TraceNode node;
    node.parent = parent_id;
    node.data_class = input_data_class; // Data class of the input to this operation

    node.operation.op_class = operation_class;
    node.operation.method = operation_method;
    // Assuming no parameters for now, or they are handled separately

    node.assumptions = assumptions;

    // Environment and Ontology version are default initialized in constructor
    return node;
}

// Simplified validation for now
bool validate_node(const TraceNode& node, const Ontology& ontology) {
    // Check if operation class is valid
    if (!ontology.validate_operation(node.operation.op_class)) {
        std::cerr << "Validation Error: Invalid operation class: " << node.operation.op_class << std::endl;
        return false;
    }

    // Check if all assumption classes are valid
    for (const auto& assump_full_str : node.assumptions) {
        if (!ontology.validate_assumption(assump_full_str)) {
            std::cerr << "Validation Error: Invalid assumption: " << assump_full_str << std::endl;
            return false;
        }
    }
    
    // Check required fields (basic presence check)
    if (node.trace_id.empty() || node.timestamp.empty() || node.data_class.empty() ||
        node.operation.op_class.empty() || node.operation.method.empty() ||
        node.input.checksum.empty() || node.output.checksum.empty() ||
        node.environment.language.empty() || node.environment.tool.empty() || node.environment.version.empty() ||
        node.ontology_version.empty()) {
        std::cerr << "Validation Error: Missing required field in TraceNode." << std::endl;
        return false;
    }

    return true;
}
