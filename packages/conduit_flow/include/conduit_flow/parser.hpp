#pragma once

#include "conduit_flow/flow.hpp"
#include <string>

namespace conduit::flow {

/// @brief Parse a .flow.yaml file into a FlowConfig.
/// @param path Filesystem path to the YAML file.
/// @return Parsed flow configuration.
/// @throws std::runtime_error If the file cannot be read or parsed.
FlowConfig parse_file(const std::string& path);

/// @brief Parse a YAML string into a FlowConfig (useful for testing).
/// @param yaml YAML content as a string.
/// @return Parsed flow configuration.
/// @throws std::runtime_error If the YAML is invalid.
FlowConfig parse_string(const std::string& yaml);

}  // namespace conduit::flow
