

# Struct conduit::Header



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**Header**](structconduit_1_1Header.md)



_Common message header with timestamp and coordinate frame._ 

* `#include <header.hpp>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  char | [**frame**](#variable-frame)  <br>_Coordinate frame identifier (null-terminated)._  |
|  uint64\_t | [**timestamp\_ns**](#variable-timestamp_ns)  <br>_Timestamp in nanoseconds._  |












































## Public Attributes Documentation




### variable frame 

_Coordinate frame identifier (null-terminated)._ 
```C++
char conduit::Header::frame[64];
```




<hr>



### variable timestamp\_ns 

_Timestamp in nanoseconds._ 
```C++
uint64_t conduit::Header::timestamp_ns;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_types/include/conduit_types/header.hpp`

