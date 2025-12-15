#include "tracer.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include "nlohmann/json.hpp"

namespace fs = std::filesystem;

// Function to load the index.json
nlohmann::json load_index(const fs::path& project_root) {
    nlohmann::json index_json;
    fs::path index_path = project_root / ".traceseq" / "index.json";
    if (fs::exists(index_path)) {
        std::ifstream index_file(index_path);
        if (index_file.is_open()) {
            index_file >> index_json;
            index_file.close();
        }
    }
    return index_json;
}

// Function to save the index.json
void save_index(const nlohmann::json& index_json, const fs::path& project_root) {
    fs::path trace_dir = project_root / ".traceseq";
    fs::create_directories(trace_dir);
    std::ofstream output_index_file(trace_dir / "index.json");
    output_index_file << std::setw(4) << index_json << std::endl;
    output_index_file.close();
}

// Map YAML node to TraceNode object
TraceNode yaml_to_tracenode(const YAML::Node& yaml_node) {
    TraceNode node;
    node.trace_id = yaml_node["trace_id"].as<std::string>();
    node.parent = yaml_node["parent"].as<std::string>();
    node.timestamp = yaml_node["timestamp"].as<std::string>();
    node.data_class = yaml_node["data_class"].as<std::string>();

    node.operation.op_class = yaml_node["operation"]["class"].as<std::string>();
    node.operation.method = yaml_node["operation"]["method"].as<std::string>();
    if (yaml_node["operation"]["parameters"].IsDefined()) {
        for (YAML::const_iterator it = yaml_node["operation"]["parameters"].begin(); it != yaml_node["operation"]["parameters"].end(); ++it) {
            node.operation.parameters[it->first.as<std::string>()] = it->second.as<std::string>();
        }
    }

    if (yaml_node["assumptions"].IsDefined()) {
        for (const auto& assump : yaml_node["assumptions"]) {
            node.assumptions.push_back(assump.as<std::string>());
        }
    }

    node.input.shape = yaml_node["input"]["shape"].as<std::string>();
    node.input.checksum = yaml_node["input"]["checksum"].as<std::string>();

    node.output.data_class = yaml_node["output"]["data_class"].as<std::string>();
    node.output.unit = yaml_node["output"]["unit"].as<std::string>();
    node.output.checksum = yaml_node["output"]["checksum"].as<std::string>();

    node.environment.language = yaml_node["environment"]["language"].as<std::string>();
    node.environment.tool = yaml_node["environment"]["tool"].as<std::string>();
    node.environment.version = yaml_node["environment"]["version"].as<std::string>();

    node.ontology_version = yaml_node["ontology_version"].as<std::string>();

    return node;
}


TraceNode load_node(const std::string& trace_id, const fs::path& project_root) {
    fs::path file_path = project_root / ".traceseq" / "nodes" / (trace_id + ".yaml");
    if (!fs::exists(file_path)) {
        throw std::runtime_error("Trace node file not found: " + file_path.string());
    }
    YAML::Node yaml_node = YAML::LoadFile(file_path.string());
    return yaml_to_tracenode(yaml_node);
}

std::vector<TraceNode> resolve_lineage(const std::string& trace_id, const fs::path& project_root) {
    std::vector<TraceNode> lineage;
    std::string current_trace_id = trace_id;

    while (current_trace_id != "null" && !current_trace_id.empty()) {
        try {
            TraceNode node = load_node(current_trace_id, project_root);
            lineage.insert(lineage.begin(), node); // Add to the front to maintain order from root to current
            current_trace_id = node.parent;
        } catch (const std::runtime_error& e) {
            std::cerr << "Error resolving lineage: " << e.what() << std::endl;
            break;
        }
    }
    return lineage;
}
