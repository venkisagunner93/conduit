

# File flow.hpp

[**File List**](files.md) **>** [**conduit\_flow**](dir_0d13c9fe7659a255034ddc49adfce1eb.md) **>** [**include**](dir_fb8309341c6f8659fa21828ed1417524.md) **>** [**conduit\_flow**](dir_288fb1ab69c9ae4a94f91c5befae3aa0.md) **>** [**flow.hpp**](flow_8hpp.md)

[Go to the documentation of this file](flow_8hpp.md)


```C++
#pragma once

#include <chrono>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace conduit::flow {

struct NodeConfig {
    std::string name;                            
    std::string exec;                            
    std::vector<std::string> args;               
    std::map<std::string, std::string> env;      
    std::string working_dir;                     
};

struct WaitDuration {
    std::chrono::milliseconds duration;  
};

struct WaitTopics {
    std::vector<std::string> topics;             
    std::chrono::milliseconds timeout{30000};    
};

struct Group {
    std::vector<NodeConfig> nodes;  
};

using Step = std::variant<NodeConfig, WaitDuration, WaitTopics, Group>;

struct FlowConfig {
    std::vector<Step> startup;   
    std::vector<Step> shutdown;  
};

}  // namespace conduit::flow
```


