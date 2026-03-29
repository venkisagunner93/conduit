

# Class conduit::WriteBuffer



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**WriteBuffer**](classconduit_1_1WriteBuffer.md)



_Sequential binary writer for serializing message fields._ [More...](#detailed-description)

* `#include <buffer.hpp>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**WriteBuffer**](#function-writebuffer) (uint8\_t \* data) <br>_Construct a write buffer starting at the given pointer._  |
|  void | [**write**](#function-write-12) (const std::string & s) <br>_Write a length-prefixed string._  |
|  void | [**write**](#function-write-22) (const T & val) <br>_Write a trivially copyable value._  |


## Public Static Functions

| Type | Name |
| ---: | :--- |
|  size\_t | [**size\_of**](#function-size_of-12) (const std::string & s) <br>_Compute the serialized size of a string (4-byte length prefix + content)._  |
|  constexpr size\_t | [**size\_of**](#function-size_of-22) (const T &) <br>_Compute the serialized size of a trivially copyable value._  |


























## Detailed Description


Writes values contiguously into a pre-allocated byte buffer. Strings are length-prefixed (uint32\_t). Trivially copyable types are written via memcpy.




**See also:** [**ReadBuffer**](classconduit_1_1ReadBuffer.md) 



    
## Public Functions Documentation




### function WriteBuffer 

_Construct a write buffer starting at the given pointer._ 
```C++
inline explicit conduit::WriteBuffer::WriteBuffer (
    uint8_t * data
) 
```





**Parameters:**


* `data` Pointer to the output buffer. 




        

<hr>



### function write [1/2]

_Write a length-prefixed string._ 
```C++
inline void conduit::WriteBuffer::write (
    const std::string & s
) 
```





**Parameters:**


* `s` The string to write. 




        

<hr>



### function write [2/2]

_Write a trivially copyable value._ 
```C++
template<typename T, std::enable_if_t< std::is_trivially_copyable_v< T >, int >>
inline void conduit::WriteBuffer::write (
    const T & val
) 
```





**Template parameters:**


* `T` Value type (must be trivially copyable). 



**Parameters:**


* `val` The value to write. 




        

<hr>
## Public Static Functions Documentation




### function size\_of [1/2]

_Compute the serialized size of a string (4-byte length prefix + content)._ 
```C++
static inline size_t conduit::WriteBuffer::size_of (
    const std::string & s
) 
```





**Parameters:**


* `s` The string. 



**Returns:**

Size in bytes. 





        

<hr>



### function size\_of [2/2]

_Compute the serialized size of a trivially copyable value._ 
```C++
template<typename T, std::enable_if_t< std::is_trivially_copyable_v< T >, int >>
static inline constexpr size_t conduit::WriteBuffer::size_of (
    const T &
) 
```





**Template parameters:**


* `T` Value type. 



**Returns:**

sizeof(T). 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_types/include/conduit_types/buffer.hpp`

