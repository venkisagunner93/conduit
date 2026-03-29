

# Class conduit::ReadBuffer



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**ReadBuffer**](classconduit_1_1ReadBuffer.md)



_Sequential binary reader for deserializing message fields._ [More...](#detailed-description)

* `#include <buffer.hpp>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**ReadBuffer**](#function-readbuffer) (const uint8\_t \* data, size\_t size) <br>_Construct a read buffer over the given data._  |
|  T | [**read**](#function-read) () <br>_Read the next value from the buffer._  |




























## Detailed Description


Reads values contiguously from a byte buffer. Strings are read as length-prefixed (uint32\_t). Trivially copyable types are read via memcpy.




**See also:** [**WriteBuffer**](classconduit_1_1WriteBuffer.md) 



    
## Public Functions Documentation




### function ReadBuffer 

_Construct a read buffer over the given data._ 
```C++
inline conduit::ReadBuffer::ReadBuffer (
    const uint8_t * data,
    size_t size
) 
```





**Parameters:**


* `data` Pointer to the input buffer. 
* `size` Total buffer size in bytes (used for bounds context). 




        

<hr>



### function read 

_Read the next value from the buffer._ 
```C++
template<typename T>
inline T conduit::ReadBuffer::read () 
```



For std::string, reads a uint32\_t length prefix followed by that many characters. For trivially copyable types, reads sizeof(T) bytes.




**Template parameters:**


* `T` Type to read (std::string or trivially copyable). 



**Returns:**

The deserialized value. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_types/include/conduit_types/buffer.hpp`

