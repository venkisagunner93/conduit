

# Struct conduit::PublisherOptions



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**PublisherOptions**](structconduit_1_1PublisherOptions.md)



_Configuration for topic publisher and ring buffer sizing._ 

* `#include <publisher.hpp>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  uint32\_t | [**depth**](#variable-depth)   = `16`<br>_Number of message slots in the ring buffer (must be power of 2)._  |
|  uint32\_t | [**max\_message\_size**](#variable-max_message_size)   = `4096`<br>_Maximum payload size in bytes. Messages exceeding this are rejected._  |












































## Public Attributes Documentation




### variable depth 

_Number of message slots in the ring buffer (must be power of 2)._ 
```C++
uint32_t conduit::PublisherOptions::depth;
```




<hr>



### variable max\_message\_size 

_Maximum payload size in bytes. Messages exceeding this are rejected._ 
```C++
uint32_t conduit::PublisherOptions::max_message_size;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/publisher.hpp`

