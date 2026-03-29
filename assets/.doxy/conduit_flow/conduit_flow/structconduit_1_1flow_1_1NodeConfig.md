

# Struct conduit::flow::NodeConfig



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**flow**](namespaceconduit_1_1flow.md) **>** [**NodeConfig**](structconduit_1_1flow_1_1NodeConfig.md)



_Configuration for a single node in a flow._ 

* `#include <flow.hpp>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::vector&lt; std::string &gt; | [**args**](#variable-args)  <br>_Command-line arguments._  |
|  std::map&lt; std::string, std::string &gt; | [**env**](#variable-env)  <br>_Environment variable overrides._  |
|  std::string | [**exec**](#variable-exec)  <br>_Executable path or command._  |
|  std::string | [**name**](#variable-name)  <br>_Logical node name._  |
|  std::string | [**working\_dir**](#variable-working_dir)  <br>_Working directory for the process._  |












































## Public Attributes Documentation




### variable args 

_Command-line arguments._ 
```C++
std::vector<std::string> conduit::flow::NodeConfig::args;
```




<hr>



### variable env 

_Environment variable overrides._ 
```C++
std::map<std::string, std::string> conduit::flow::NodeConfig::env;
```




<hr>



### variable exec 

_Executable path or command._ 
```C++
std::string conduit::flow::NodeConfig::exec;
```




<hr>



### variable name 

_Logical node name._ 
```C++
std::string conduit::flow::NodeConfig::name;
```




<hr>



### variable working\_dir 

_Working directory for the process._ 
```C++
std::string conduit::flow::NodeConfig::working_dir;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_flow/include/conduit_flow/flow.hpp`

