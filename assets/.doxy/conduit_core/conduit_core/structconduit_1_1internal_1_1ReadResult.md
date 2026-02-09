

# Struct conduit::internal::ReadResult



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**internal**](namespaceconduit_1_1internal.md) **>** [**ReadResult**](structconduit_1_1internal_1_1ReadResult.md)



_Result of a successful ring buffer read._ 

* `#include <ring_buffer.hpp>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**const**](classconduit_1_1internal_1_1Publisher.md) [**void**](classconduit_1_1internal_1_1Publisher.md) \* | [**data**](#variable-data)  <br>_Pointer to payload within the slot._  |
|  [**uint64\_t**](classconduit_1_1internal_1_1Publisher.md) | [**sequence**](#variable-sequence)  <br>[_**Message**_](structconduit_1_1Message.md) _sequence number._ |
|  [**size\_t**](classconduit_1_1internal_1_1Publisher.md) | [**size**](#variable-size)  <br>_Payload size in bytes._  |
|  [**uint64\_t**](classconduit_1_1internal_1_1Publisher.md) | [**timestamp\_ns**](#variable-timestamp_ns)  <br>_CLOCK\_MONOTONIC\_RAW timestamp in nanoseconds._  |












































## Public Attributes Documentation




### variable data 

_Pointer to payload within the slot._ 
```C++
const void* conduit::internal::ReadResult::data;
```




<hr>



### variable sequence 

[_**Message**_](structconduit_1_1Message.md) _sequence number._
```C++
uint64_t conduit::internal::ReadResult::sequence;
```




<hr>



### variable size 

_Payload size in bytes._ 
```C++
size_t conduit::internal::ReadResult::size;
```




<hr>



### variable timestamp\_ns 

_CLOCK\_MONOTONIC\_RAW timestamp in nanoseconds._ 
```C++
uint64_t conduit::internal::ReadResult::timestamp_ns;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/internal/ring_buffer.hpp`

