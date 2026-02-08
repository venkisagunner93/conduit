

# Namespace conduit::flow



[**Namespace List**](namespaces.md) **>** [**conduit**](namespaceconduit.md) **>** [**flow**](namespaceconduit_1_1flow.md)




















## Classes

| Type | Name |
| ---: | :--- |
| class | [**Executor**](classconduit_1_1flow_1_1Executor.md) <br>_Executes a flow configuration (startup and shutdown sequences)._  |
| struct | [**ExecutorOptions**](structconduit_1_1flow_1_1ExecutorOptions.md) <br>_Options for the flow executor._  |
| struct | [**FlowConfig**](structconduit_1_1flow_1_1FlowConfig.md) <br>_Complete flow configuration with startup and shutdown sequences._  |
| struct | [**Group**](structconduit_1_1flow_1_1Group.md) <br>_Parallel group: start multiple nodes simultaneously._  |
| struct | [**NodeConfig**](structconduit_1_1flow_1_1NodeConfig.md) <br>_Configuration for a single node in a flow._  |
| struct | [**WaitDuration**](structconduit_1_1flow_1_1WaitDuration.md) <br>_Wait step: pause for a fixed duration._  |
| struct | [**WaitTopics**](structconduit_1_1flow_1_1WaitTopics.md) <br>_Wait step: block until specified topics exist in shared memory._  |


## Public Types

| Type | Name |
| ---: | :--- |
| typedef std::variant&lt; [**NodeConfig**](structconduit_1_1flow_1_1NodeConfig.md), [**WaitDuration**](structconduit_1_1flow_1_1WaitDuration.md), [**WaitTopics**](structconduit_1_1flow_1_1WaitTopics.md), [**Group**](structconduit_1_1flow_1_1Group.md) &gt; | [**Step**](#typedef-step)  <br>_A single step in a flow sequence._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**FlowConfig**](structconduit_1_1flow_1_1FlowConfig.md) | [**parse\_file**](#function-parse_file) (const std::string & path) <br>_Parse a .flow.yaml file into a_ [_**FlowConfig**_](structconduit_1_1flow_1_1FlowConfig.md) _._ |
|  [**FlowConfig**](structconduit_1_1flow_1_1FlowConfig.md) | [**parse\_string**](#function-parse_string) (const std::string & yaml) <br>_Parse a YAML string into a_ [_**FlowConfig**_](structconduit_1_1flow_1_1FlowConfig.md) _(useful for testing)._ |




























## Public Types Documentation




### typedef Step 

_A single step in a flow sequence._ 
```C++
using conduit::flow::Step = typedef std::variant<NodeConfig, WaitDuration, WaitTopics, Group>;
```




<hr>
## Public Functions Documentation




### function parse\_file 

_Parse a .flow.yaml file into a_ [_**FlowConfig**_](structconduit_1_1flow_1_1FlowConfig.md) _._
```C++
FlowConfig conduit::flow::parse_file (
    const std::string & path
) 
```





**Parameters:**


* `path` Filesystem path to the YAML file. 



**Returns:**

Parsed flow configuration. 




**Exception:**


* `std::runtime_error` If the file cannot be read or parsed. 




        

<hr>



### function parse\_string 

_Parse a YAML string into a_ [_**FlowConfig**_](structconduit_1_1flow_1_1FlowConfig.md) _(useful for testing)._
```C++
FlowConfig conduit::flow::parse_string (
    const std::string & yaml
) 
```





**Parameters:**


* `yaml` YAML content as a string. 



**Returns:**

Parsed flow configuration. 




**Exception:**


* `std::runtime_error` If the YAML is invalid. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_flow/include/conduit_flow/executor.hpp`

