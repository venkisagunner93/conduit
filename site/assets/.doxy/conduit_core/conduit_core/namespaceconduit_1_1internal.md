

# Namespace conduit::internal



[**Namespace List**](namespaces.md) **>** [**conduit**](namespaceconduit.md) **>** [**internal**](namespaceconduit_1_1internal.md)



_Internal implementation details for conduit._ 
















## Classes

| Type | Name |
| ---: | :--- |
| struct | [**AlignedAtomicU64**](structconduit_1_1internal_1_1AlignedAtomicU64.md) <br>_Cache-line-aligned atomic uint64\_t to prevent false sharing._  |
| class | [**Publisher**](classconduit_1_1internal_1_1Publisher.md) <br>_Low-level publisher that writes raw bytes to a shared memory ring buffer._  |
| struct | [**ReadResult**](structconduit_1_1internal_1_1ReadResult.md) <br>_Result of a successful ring buffer read._  |
| struct | [**RingBufferConfig**](structconduit_1_1internal_1_1RingBufferConfig.md) <br>_Ring buffer configuration._  |
| struct | [**RingBufferHeader**](structconduit_1_1internal_1_1RingBufferHeader.md) <br>_Shared memory layout for the ring buffer control structure._  |
| class | [**RingBufferReader**](classconduit_1_1internal_1_1RingBufferReader.md) <br>_Reader side of the lock-free SPMC ring buffer._  |
| class | [**RingBufferWriter**](classconduit_1_1internal_1_1RingBufferWriter.md) <br>_Writer side of the lock-free SPMC ring buffer._  |
| class | [**ShmRegion**](classconduit_1_1internal_1_1ShmRegion.md) <br>_RAII wrapper for a POSIX shared memory region._  |
| class | [**Subscriber**](classconduit_1_1internal_1_1Subscriber.md) <br>_Low-level subscriber that reads raw bytes from a shared memory ring buffer._  |






## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**constexpr**](classconduit_1_1internal_1_1Publisher.md) [**size\_t**](classconduit_1_1internal_1_1Publisher.md) | [**CACHE\_LINE\_SIZE**](#variable-cache_line_size)   = `64`<br>_CPU cache line size used for alignment to prevent false sharing._  |
|  [**constexpr**](classconduit_1_1internal_1_1Publisher.md) [**size\_t**](classconduit_1_1internal_1_1Publisher.md) | [**MAX\_SUBSCRIBERS**](#variable-max_subscribers)   = `16`<br>_Maximum number of concurrent subscriber reader slots._  |
|  [**constexpr**](classconduit_1_1internal_1_1Publisher.md) [**size\_t**](classconduit_1_1internal_1_1Publisher.md) | [**SLOT\_HEADER\_SIZE**](#variable-slot_header_size)   = `[**sizeof**](classconduit_1_1internal_1_1Publisher.md)([**uint32\_t**](classconduit_1_1internal_1_1Publisher.md)) + [**sizeof**](classconduit_1_1internal_1_1Publisher.md)([**uint64\_t**](classconduit_1_1internal_1_1Publisher.md)) + [**sizeof**](classconduit_1_1internal_1_1Publisher.md)([**uint64\_t**](classconduit_1_1internal_1_1Publisher.md))`<br>_Size of each slot's header in bytes._  |
















## Public Functions

| Type | Name |
| ---: | :--- |
|  [**size\_t**](classconduit_1_1internal_1_1Publisher.md) | [**calculate\_region\_size**](#function-calculate_region_size) ([**const**](classconduit_1_1internal_1_1Publisher.md) [**RingBufferConfig**](structconduit_1_1internal_1_1RingBufferConfig.md) & config) <br>_Calculate total shared memory region size for the given config._  |
|  [**bool**](classconduit_1_1internal_1_1Publisher.md) | [**futex\_wait**](#function-futex_wait) (std::atomic&lt; [**uint32\_t**](classconduit_1_1internal_1_1Publisher.md) &gt; \* futex\_word, [**uint32\_t**](classconduit_1_1internal_1_1Publisher.md) expected\_value, std::optional&lt; std::chrono::nanoseconds &gt; timeout=std::nullopt) <br>_Wait until the futex word changes from the expected value._  |
|  [**int**](classconduit_1_1internal_1_1Publisher.md) | [**futex\_wake**](#function-futex_wake) (std::atomic&lt; [**uint32\_t**](classconduit_1_1internal_1_1Publisher.md) &gt; \* futex\_word, [**int**](classconduit_1_1internal_1_1Publisher.md) count=1) <br>_Wake up to_ `count` _threads waiting on the futex word._ |
|  [**int**](classconduit_1_1internal_1_1Publisher.md) | [**futex\_wake\_all**](#function-futex_wake_all) (std::atomic&lt; [**uint32\_t**](classconduit_1_1internal_1_1Publisher.md) &gt; \* futex\_word) <br>_Wake all threads waiting on the futex word._  |
|  [**uint64\_t**](classconduit_1_1internal_1_1Publisher.md) | [**get\_timestamp\_ns**](#function-get_timestamp_ns) () <br>_Get the current timestamp in nanoseconds from CLOCK\_MONOTONIC\_RAW._  |
|  [**bool**](classconduit_1_1internal_1_1Publisher.md) | [**is\_power\_of\_two**](#function-is_power_of_two) ([**uint32\_t**](classconduit_1_1internal_1_1Publisher.md) n) <br>_Check if n is a power of two._  |




























## Public Attributes Documentation




### variable CACHE\_LINE\_SIZE 

_CPU cache line size used for alignment to prevent false sharing._ 
```C++
constexpr size_t conduit::internal::CACHE_LINE_SIZE;
```




<hr>



### variable MAX\_SUBSCRIBERS 

_Maximum number of concurrent subscriber reader slots._ 
```C++
constexpr size_t conduit::internal::MAX_SUBSCRIBERS;
```




<hr>



### variable SLOT\_HEADER\_SIZE 

_Size of each slot's header in bytes._ 
```C++
constexpr size_t conduit::internal::SLOT_HEADER_SIZE;
```



Slot header layout: 
```C++
┌────────────┬──────────────┬────────────────┐
│ size (4B)  │ sequence (8B)│ timestamp (8B)  │  = 20 bytes
└────────────┴──────────────┴────────────────┘
```
 


        

<hr>
## Public Functions Documentation




### function calculate\_region\_size 

_Calculate total shared memory region size for the given config._ 
```C++
inline size_t conduit::internal::calculate_region_size (
    const  RingBufferConfig & config
) 
```





**Parameters:**


* `config` Ring buffer configuration. 



**Returns:**

Total size in bytes (header + all slots). 





        

<hr>



### function futex\_wait 

_Wait until the futex word changes from the expected value._ 
```C++
bool conduit::internal::futex_wait (
    std::atomic< uint32_t > * futex_word,
    uint32_t expected_value,
    std::optional< std::chrono::nanoseconds > timeout=std::nullopt
) 
```



Wraps the Linux `futex(FUTEX_WAIT)` syscall. The calling thread sleeps (consuming zero CPU) until another thread calls [**futex\_wake()**](namespaceconduit_1_1internal.md#function-futex_wake) on the same word, or the optional timeout expires. A spurious wakeup is possible if the futex word has already changed by the time the syscall executes.




**Parameters:**


* `futex_word` Pointer to the atomic futex word in shared memory. 
* `expected_value` The value that triggered the wait; if the current value differs, the call returns immediately. 
* `timeout` Optional maximum wait duration. std::nullopt means wait forever. 



**Returns:**

true if woken by [**futex\_wake()**](namespaceconduit_1_1internal.md#function-futex_wake), false on timeout. 





        

<hr>



### function futex\_wake 

_Wake up to_ `count` _threads waiting on the futex word._
```C++
int conduit::internal::futex_wake (
    std::atomic< uint32_t > * futex_word,
    int count=1
) 
```



Wraps the Linux `futex(FUTEX_WAKE)` syscall.




**Parameters:**


* `futex_word` Pointer to the atomic futex word. 
* `count` Maximum number of waiters to wake (default 1). 



**Returns:**

Number of waiters actually woken. 





        

<hr>



### function futex\_wake\_all 

_Wake all threads waiting on the futex word._ 
```C++
int conduit::internal::futex_wake_all (
    std::atomic< uint32_t > * futex_word
) 
```





**Parameters:**


* `futex_word` Pointer to the atomic futex word. 



**Returns:**

Number of waiters actually woken. 





        

<hr>



### function get\_timestamp\_ns 

_Get the current timestamp in nanoseconds from CLOCK\_MONOTONIC\_RAW._ 
```C++
uint64_t conduit::internal::get_timestamp_ns () 
```





**Returns:**

Nanoseconds since an arbitrary epoch (monotonic, not wall-clock). 





        

<hr>



### function is\_power\_of\_two 

_Check if n is a power of two._ 
```C++
inline bool conduit::internal::is_power_of_two (
    uint32_t n
) 
```





**Parameters:**


* `n` Value to check. 



**Returns:**

true if n is a power of two. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/internal/futex.hpp`

