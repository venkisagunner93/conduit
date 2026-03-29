

# Struct conduit::flow::FlowConfig



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**flow**](namespaceconduit_1_1flow.md) **>** [**FlowConfig**](structconduit_1_1flow_1_1FlowConfig.md)



_Complete flow configuration with startup and shutdown sequences._ 

* `#include <flow.hpp>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::vector&lt; Step &gt; | [**shutdown**](#variable-shutdown)  <br>_Ordered shutdown steps (default: reverse of startup)._  |
|  std::vector&lt; Step &gt; | [**startup**](#variable-startup)  <br>_Ordered startup steps._  |












































## Public Attributes Documentation




### variable shutdown 

_Ordered shutdown steps (default: reverse of startup)._ 
```C++
std::vector<Step> conduit::flow::FlowConfig::shutdown;
```




<hr>



### variable startup 

_Ordered startup steps._ 
```C++
std::vector<Step> conduit::flow::FlowConfig::startup;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_flow/include/conduit_flow/flow.hpp`

