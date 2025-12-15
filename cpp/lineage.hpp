#ifndef LINEAGE_HPP
#define LINEAGE_HPP

#include <filesystem>
#include "nlohmann/json.hpp"
#include "tracer.hpp"

namespace fs = std::filesystem;

/**
 * @brief Loads the trace index from the '.traceseq/index.json' file.
 *
 * The index maps file checksums to trace node IDs, providing a lookup
 * mechanism for provenance information. If the index file does not exist,
 * an empty JSON object is returned.
 *
 * @param project_root The root directory of the project.
 * @return A `nlohmann::json` object representing the loaded index.
 */
nlohmann::json load_index(const fs::path& project_root);

/**
 * @brief Saves the trace index to the '.traceseq/index.json' file.
 *
 * This function serializes the provided JSON object to the index file,
 * ensuring that the provenance lookup is up-to-date.
 *
 * @param index_json The `nlohmann::json` object representing the index to save.
 * @param project_root The root directory of the project.
 */
void save_index(const nlohmann::json& index_json, const fs::path& project_root);

/**
 * @brief Resolves the full lineage of a trace node.
 *
 * This function reconstructs the entire provenance chain for a given
 * trace node ID by recursively loading parent nodes until the root
 * of the lineage is reached.
 *
 * @param trace_id The ID of the trace node for which to resolve the lineage.
 * @param project_root The root directory of the project.
 * @return A `std::vector` of `TraceNode` objects, ordered from the oldest
 *         (root) to the most recent node in the lineage.
 */
std::vector<TraceNode> resolve_lineage(const std::string& trace_id, const fs::path& project_root);

#endif // LINEAGE_HPP
