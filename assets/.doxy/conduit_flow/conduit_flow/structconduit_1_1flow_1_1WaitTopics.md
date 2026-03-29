

# Struct conduit::flow::WaitTopics



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**flow**](namespaceconduit_1_1flow.md) **>** [**WaitTopics**](structconduit_1_1flow_1_1WaitTopics.md)



_Wait step: block until specified topics exist in shared memory._ 

* `#include <flow.hpp>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::chrono::milliseconds | [**timeout**](#variable-timeout)   = `{30000}`<br>_Maximum wait time (default 30s)._  |
|  std::vector&lt; std::string &gt; | [**topics**](#variable-topics)  <br>_Topic names to wait for._  |












































## Public Attributes Documentation




### variable timeout 

_Maximum wait time (default 30s)._ 
```C++
std::chrono::milliseconds conduit::flow::WaitTopics::timeout;
```




<hr>



### variable topics 

_Topic names to wait for._ 
```C++
std::vector<std::string> conduit::flow::WaitTopics::topics;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_flow/include/conduit_flow/flow.hpp`

