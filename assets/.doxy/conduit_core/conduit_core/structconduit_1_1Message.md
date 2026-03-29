

# Struct conduit::Message



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**Message**](structconduit_1_1Message.md)



_Raw message received from a topic._ [More...](#detailed-description)

* `#include <subscriber.hpp>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  const void \* | [**data**](#variable-data)  <br>_Pointer to payload in shared memory (transient)._  |
|  uint64\_t | [**sequence**](#variable-sequence)  <br>_Monotonically increasing message sequence number._  |
|  size\_t | [**size**](#variable-size)  <br>_Payload size in bytes._  |
|  uint64\_t | [**timestamp\_ns**](#variable-timestamp_ns)  <br>_CLOCK\_MONOTONIC\_RAW timestamp in nanoseconds._  |












































## Detailed Description


Contains a pointer into shared memory that is only valid until the next call to take()/wait()/wait\_for() on the same subscriber. 


    
## Public Attributes Documentation




### variable data 

_Pointer to payload in shared memory (transient)._ 
```C++
const void* conduit::Message::data;
```




<hr>



### variable sequence 

_Monotonically increasing message sequence number._ 
```C++
uint64_t conduit::Message::sequence;
```




<hr>



### variable size 

_Payload size in bytes._ 
```C++
size_t conduit::Message::size;
```




<hr>



### variable timestamp\_ns 

_CLOCK\_MONOTONIC\_RAW timestamp in nanoseconds._ 
```C++
uint64_t conduit::Message::timestamp_ns;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/subscriber.hpp`

