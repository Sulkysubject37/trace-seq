#ifndef TRACER_HPP
#define TRACER_HPP

#include <filesystem>
#include <string>
#include <vector>
#include <map>
#include "yaml-cpp/yaml.h"

// Forward declaration for UUID generation
std::string generate_uuid();

class TraceNode {
public:
    struct Operation {
        std::string op_class;
        std::string method;
        std::map<std::string, std::string> parameters; // Simplified: could be YAML::Node for full flexibility
    };

    struct Input {
        std::string shape;
        std::string checksum;
    };

    struct Output {
        std::string data_class;
        std::string unit;
        std::string checksum;
    };

    struct Environment {
        std::string language;
        std::string tool;
        std::string version;
    };

    std::string trace_id;
    std::string parent;
    std::string timestamp;
    std::string data_class;
    Operation operation;
    std::vector<std::string> assumptions;
    Input input;
    Output output;
    Environment environment;
    std::string ontology_version;

    // Constructor
    TraceNode();

    // Save method declaration
    void save(const std::string& input_file_checksum, const std::string& output_file_checksum, const std::string& output_file_data_class, const std::filesystem::path& project_root);
};

class Ontology {
public:
    YAML::Node operations;
    YAML::Node assumptions;

    void load(const std::string& op_path, const std::string& assump_path);
    bool validate_operation(const std::string& op_class) const;
    bool validate_assumption(const std::string& assump_class_value) const; // Modified to handle "class:value"
};

// Functions to create and validate TraceNodes
TraceNode create_trace_node(
    const std::string& parent_id,
    const std::string& data_class, // This is the input data_class
    const std::string& operation_class,
    const std::string& operation_method,
    const std::vector<std::string>& assumptions
);

bool validate_node(const TraceNode& node, const Ontology& ontology);

#endif // TRACER_HPP
