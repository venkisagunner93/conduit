#pragma once

#include "conduit_flow/flow.hpp"
#include <string>

namespace conduit::flow {

// Parse a .flow.yaml file
FlowConfig parse_file(const std::string& path);

// Parse YAML string (for testing)
FlowConfig parse_string(const std::string& yaml);

}  // namespace conduit::flow
