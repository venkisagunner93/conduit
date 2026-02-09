

# Class conduit::internal::RingBufferReader



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**internal**](namespaceconduit_1_1internal.md) **>** [**RingBufferReader**](classconduit_1_1internal_1_1RingBufferReader.md)



_Reader side of the lock-free SPMC ring buffer._ [More...](#detailed-description)

* `#include <ring_buffer.hpp>`





































## Public Functions

| Type | Name |
| ---: | :--- |
|   | [**RingBufferReader**](#function-ringbufferreader) ([**void**](classconduit_1_1internal_1_1Publisher.md) \* region, [**size\_t**](classconduit_1_1internal_1_1Publisher.md) region\_size) <br>_Construct a reader over an existing shared memory region._  |
|  [**int**](classconduit_1_1internal_1_1Publisher.md) | [**claim\_slot**](#function-claim_slot) () <br>_Claim a subscriber slot in the ring buffer._  |
|  [**RingBufferHeader**](structconduit_1_1internal_1_1RingBufferHeader.md) \* | [**header**](#function-header) () <br>_Access the ring buffer header._  |
|  [**void**](classconduit_1_1internal_1_1Publisher.md) | [**release\_slot**](#function-release_slot) ([**int**](classconduit_1_1internal_1_1Publisher.md) slot) <br>_Release a previously claimed subscriber slot._  |
|  std::optional&lt; [**ReadResult**](structconduit_1_1internal_1_1ReadResult.md) &gt; | [**try\_read**](#function-try_read) ([**int**](classconduit_1_1internal_1_1Publisher.md) slot) <br>_Non-blocking read of the next message._  |
|  std::optional&lt; [**ReadResult**](structconduit_1_1internal_1_1ReadResult.md) &gt; | [**wait**](#function-wait) ([**int**](classconduit_1_1internal_1_1Publisher.md) slot) <br>_Block until a message is available (waits forever)._  |
|  std::optional&lt; [**ReadResult**](structconduit_1_1internal_1_1ReadResult.md) &gt; | [**wait\_for**](#function-wait_for) ([**int**](classconduit_1_1internal_1_1Publisher.md) slot, std::chrono::nanoseconds timeout) <br>_Block until a message is available or timeout expires._  |




























## Detailed Description


Multiple readers can exist per topic (up to MAX\_SUBSCRIBERS). Each reader claims a slot via [**claim\_slot()**](classconduit_1_1internal_1_1RingBufferReader.md#function-claim_slot), then reads messages independently. If the writer laps a reader, the reader detects the overwrite via sequence number validation and skips ahead.




**See also:** [**RingBufferWriter**](classconduit_1_1internal_1_1RingBufferWriter.md) 



    
## Public Functions Documentation




### function RingBufferReader 

_Construct a reader over an existing shared memory region._ 
```C++
conduit::internal::RingBufferReader::RingBufferReader (
    void * region,
    size_t region_size
) 
```





**Parameters:**


* `region` Pointer to the shared memory region (must already be initialized). 
* `region_size` Total size of the region in bytes. 




        

<hr>



### function claim\_slot 

_Claim a subscriber slot in the ring buffer._ 
```C++
int conduit::internal::RingBufferReader::claim_slot () 
```





**Returns:**

Slot index (0..MAX\_SUBSCRIBERS-1), or -1 if all slots are taken. 





        

<hr>



### function header 

_Access the ring buffer header._ 
```C++
inline RingBufferHeader * conduit::internal::RingBufferReader::header () 
```





**Returns:**

Pointer to the header in shared memory. 





        

<hr>



### function release\_slot 

_Release a previously claimed subscriber slot._ 
```C++
void conduit::internal::RingBufferReader::release_slot (
    int slot
) 
```





**Parameters:**


* `slot` Slot index to release. 




        

<hr>



### function try\_read 

_Non-blocking read of the next message._ 
```C++
std::optional< ReadResult > conduit::internal::RingBufferReader::try_read (
    int slot
) 
```





**Parameters:**


* `slot` Reader slot index from [**claim\_slot()**](classconduit_1_1internal_1_1RingBufferReader.md#function-claim_slot). 



**Returns:**

The next message, or std::nullopt if no new message is available. 





        

<hr>



### function wait 

_Block until a message is available (waits forever)._ 
```C++
std::optional< ReadResult > conduit::internal::RingBufferReader::wait (
    int slot
) 
```



Uses futex-based signaling for zero CPU usage while idle.




**Parameters:**


* `slot` Reader slot index. 



**Returns:**

The next message, or std::nullopt on spurious wakeup. 





        

<hr>



### function wait\_for 

_Block until a message is available or timeout expires._ 
```C++
std::optional< ReadResult > conduit::internal::RingBufferReader::wait_for (
    int slot,
    std::chrono::nanoseconds timeout
) 
```





**Parameters:**


* `slot` Reader slot index. 
* `timeout` Maximum time to wait. 



**Returns:**

The next message, or std::nullopt on timeout. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/internal/ring_buffer.hpp`

