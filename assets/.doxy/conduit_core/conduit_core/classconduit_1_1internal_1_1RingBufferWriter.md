

# Class conduit::internal::RingBufferWriter



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**internal**](namespaceconduit_1_1internal.md) **>** [**RingBufferWriter**](classconduit_1_1internal_1_1RingBufferWriter.md)



_Writer side of the lock-free SPMC ring buffer._ [More...](#detailed-description)

* `#include <ring_buffer.hpp>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**RingBufferWriter**](#function-ringbufferwriter) ([**void**](classconduit_1_1internal_1_1Publisher.md) \* region, [**size\_t**](classconduit_1_1internal_1_1Publisher.md) region\_size, [**const**](classconduit_1_1internal_1_1Publisher.md) [**RingBufferConfig**](structconduit_1_1internal_1_1RingBufferConfig.md) & config) <br>_Construct a writer over a shared memory region._  |
|  [**RingBufferHeader**](structconduit_1_1internal_1_1RingBufferHeader.md) \* | [**header**](#function-header) () <br>_Access the ring buffer header._  |
|  [**void**](classconduit_1_1internal_1_1Publisher.md) | [**initialize**](#function-initialize) () <br>_Initialize the ring buffer header in shared memory._  |
|  [**bool**](classconduit_1_1internal_1_1Publisher.md) | [**try\_write**](#function-try_write) ([**const**](classconduit_1_1internal_1_1Publisher.md) [**void**](classconduit_1_1internal_1_1Publisher.md) \* data, [**size\_t**](classconduit_1_1internal_1_1Publisher.md) len) <br>_Write a message to the next slot in the ring buffer._  |




























## Detailed Description


There is exactly one writer per topic. The writer initializes the shared memory header, then writes messages into slots in a circular pattern. After each write, subscribers are woken via futex.




**See also:** [**RingBufferReader**](classconduit_1_1internal_1_1RingBufferReader.md) 



    
## Public Functions Documentation




### function RingBufferWriter 

_Construct a writer over a shared memory region._ 
```C++
conduit::internal::RingBufferWriter::RingBufferWriter (
    void * region,
    size_t region_size,
    const  RingBufferConfig & config
) 
```





**Parameters:**


* `region` Pointer to the shared memory region. 
* `region_size` Total size of the region in bytes. 
* `config` Ring buffer configuration (slot\_count must be power of 2). 




        

<hr>



### function header 

_Access the ring buffer header._ 
```C++
inline RingBufferHeader * conduit::internal::RingBufferWriter::header () 
```





**Returns:**

Pointer to the header in shared memory. 





        

<hr>



### function initialize 

_Initialize the ring buffer header in shared memory._ 
```C++
void conduit::internal::RingBufferWriter::initialize () 
```




<hr>



### function try\_write 

_Write a message to the next slot in the ring buffer._ 
```C++
bool conduit::internal::RingBufferWriter::try_write (
    const  void * data,
    size_t len
) 
```



Automatically timestamps the message with CLOCK\_MONOTONIC\_RAW, increments the sequence number, and wakes waiting subscribers.




**Parameters:**


* `data` Pointer to the payload. 
* `len` Payload size in bytes. 



**Returns:**

true if written, false if len exceeds the slot's payload capacity. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/internal/ring_buffer.hpp`

