

# Struct conduit::internal::RingBufferConfig



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**internal**](namespaceconduit_1_1internal.md) **>** [**RingBufferConfig**](structconduit_1_1internal_1_1RingBufferConfig.md)



_Ring buffer configuration._ 

* `#include <ring_buffer.hpp>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**uint32\_t**](classconduit_1_1internal_1_1Publisher.md) | [**slot\_count**](#variable-slot_count)  <br>_Number of slots (must be power of 2)._  |
|  [**uint32\_t**](classconduit_1_1internal_1_1Publisher.md) | [**slot\_size**](#variable-slot_size)  <br>_Bytes per slot (including slot header)._  |












































## Public Attributes Documentation




### variable slot\_count 

_Number of slots (must be power of 2)._ 
```C++
uint32_t conduit::internal::RingBufferConfig::slot_count;
```




<hr>



### variable slot\_size 

_Bytes per slot (including slot header)._ 
```C++
uint32_t conduit::internal::RingBufferConfig::slot_size;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/internal/ring_buffer.hpp`

