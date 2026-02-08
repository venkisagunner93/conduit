

# Struct conduit::TypedMessage

**template &lt;typename T&gt;**



[**ClassList**](annotated.md) **>** [**TypedMessage**](structconduit_1_1TypedMessage.md)



_Typed message with deserialized payload._ [More...](#detailed-description)






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  T | [**data**](#variable-data)  <br>_Deserialized message payload._  |
|  uint64\_t | [**sequence**](#variable-sequence)  <br>_Monotonically increasing message sequence number._  |
|  uint64\_t | [**timestamp\_ns**](#variable-timestamp_ns)  <br>_CLOCK\_MONOTONIC\_RAW timestamp in nanoseconds._  |












































## Detailed Description




**Template parameters:**


* `T` The message type. 




    
## Public Attributes Documentation




### variable data 

_Deserialized message payload._ 
```C++
T conduit::TypedMessage< T >::data;
```




<hr>



### variable sequence 

_Monotonically increasing message sequence number._ 
```C++
uint64_t conduit::TypedMessage< T >::sequence;
```




<hr>



### variable timestamp\_ns 

_CLOCK\_MONOTONIC\_RAW timestamp in nanoseconds._ 
```C++
uint64_t conduit::TypedMessage< T >::timestamp_ns;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/subscriber.hpp`

