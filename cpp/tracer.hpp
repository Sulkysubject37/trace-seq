#ifndef TRACER_HPP
#define TRACER_HPP

#include <string>
#include <vector>
#include "yaml-cpp/yaml.h"

class TraceNode {
public:
    std::string trace_id;
    std::string parent;
    std::string timestamp;
    std::string data_class;
    // ... other fields ...

    void save() const;
};

class Ontology {
public:
    YAML::Node operations;
    YAML::Node assumptions;

    void load(const std::string& op_path, const std::string& assump_path);
    bool validate_operation(const std::string& op_class);
    bool validate_assumption(const std::string& assump_class);
};

TraceNode create_trace_node(
    const std::string& parent_id,
    const std::string& data_class,
    const std::string& operation_class,
    const std::string& operation_method,
    const std::vector<std::string>& assumptions
);

bool validate_node(const TraceNode& node);

#endif // TRACER_HPP
