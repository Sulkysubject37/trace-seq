#ifndef LINEAGE_HPP
#define LINEAGE_HPP

#include <filesystem>
#include "nlohmann/json.hpp"
#include "tracer.hpp"

namespace fs = std::filesystem;

nlohmann::json load_index(const fs::path& project_root);
void save_index(const nlohmann::json& index_json, const fs::path& project_root);
std::vector<TraceNode> resolve_lineage(const std::string& trace_id, const fs::path& project_root);

#endif // LINEAGE_HPP
