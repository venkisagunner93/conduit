

# Class conduit::VariableMessageType



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**VariableMessageType**](classconduit_1_1VariableMessageType.md)



_Base class for variable-size messages requiring serialization._ [More...](#detailed-description)

* `#include <variable_message_type.hpp>`





































## Public Functions

| Type | Name |
| ---: | :--- |
| virtual void | [**serialize**](#function-serialize) (uint8\_t \* buffer) const = 0<br>_Serialize the message into a pre-allocated buffer._  |
| virtual size\_t | [**serialized\_size**](#function-serialized_size) () const = 0<br>_Compute the serialized size in bytes._  |
| virtual  | [**~VariableMessageType**](#function-variablemessagetype) () = default<br> |
























## Protected Functions

| Type | Name |
| ---: | :--- |
|   | [**VariableMessageType**](#function-variablemessagetype) () = default<br> |




## Detailed Description


Derive from this to create messages with dynamic-size fields (e.g. strings). Subclasses must implement [**serialized\_size()**](classconduit_1_1VariableMessageType.md#function-serialized_size), [**serialize()**](classconduit_1_1VariableMessageType.md#function-serialize), and a static deserialize() factory method.




**See also:** [**validate\_variable\_message\_type**](namespaceconduit.md#function-validate_variable_message_type), [**WriteBuffer**](classconduit_1_1WriteBuffer.md), [**ReadBuffer**](classconduit_1_1ReadBuffer.md) 



    
## Public Functions Documentation




### function serialize 

_Serialize the message into a pre-allocated buffer._ 
```C++
virtual void conduit::VariableMessageType::serialize (
    uint8_t * buffer
) const = 0
```





**Parameters:**


* `buffer` Output buffer (must have at least [**serialized\_size()**](classconduit_1_1VariableMessageType.md#function-serialized_size) bytes). 




        

<hr>



### function serialized\_size 

_Compute the serialized size in bytes._ 
```C++
virtual size_t conduit::VariableMessageType::serialized_size () const = 0
```





**Returns:**

Number of bytes needed for [**serialize()**](classconduit_1_1VariableMessageType.md#function-serialize). 





        

<hr>



### function ~VariableMessageType 

```C++
virtual conduit::VariableMessageType::~VariableMessageType () = default
```




<hr>
## Protected Functions Documentation




### function VariableMessageType 

```C++
conduit::VariableMessageType::VariableMessageType () = default
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_types/include/conduit_types/variable_message_type.hpp`

