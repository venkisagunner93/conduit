

# Struct conduit::internal::RingBufferHeader



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**internal**](namespaceconduit_1_1internal.md) **>** [**RingBufferHeader**](structconduit_1_1internal_1_1RingBufferHeader.md)



_Shared memory layout for the ring buffer control structure._ [More...](#detailed-description)

* `#include <ring_buffer.hpp>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::atomic&lt; [**uint32\_t**](classconduit_1_1internal_1_1Publisher.md) &gt; | [**futex\_word**](#variable-futex_word)  <br>_Futex word used for subscriber wake signaling._  |
|  [**uint32\_t**](classconduit_1_1internal_1_1Publisher.md) | [**max\_subscribers**](#variable-max_subscribers)  <br>_Maximum reader slots._  |
|  [**uint32\_t**](classconduit_1_1internal_1_1Publisher.md) | [**padding0**](#variable-padding0)  <br>_Padding for alignment._  |
|  [**AlignedAtomicU64**](structconduit_1_1internal_1_1AlignedAtomicU64.md) | [**read\_idx**](#variable-read_idx)  <br>_Per-reader current read index (each on own cache line)._  |
|  [**uint32\_t**](classconduit_1_1internal_1_1Publisher.md) | [**slot\_count**](#variable-slot_count)  <br>_Number of slots._  |
|  [**uint32\_t**](classconduit_1_1internal_1_1Publisher.md) | [**slot\_size**](#variable-slot_size)  <br>_Bytes per slot._  |
|  std::atomic&lt; [**uint32\_t**](classconduit_1_1internal_1_1Publisher.md) &gt; | [**subscriber\_mask**](#variable-subscriber_mask)  <br>_Bitmask of claimed subscriber slots (own cache line)._  |
|  std::atomic&lt; [**uint64\_t**](classconduit_1_1internal_1_1Publisher.md) &gt; | [**write\_idx**](#variable-write_idx)  <br>_Writer's next write index (own cache line to avoid false sharing)._  |












































## Detailed Description


Resides at the start of the shared memory region, followed by the slot data array. Each field group is cache-line-aligned to prevent false sharing between the writer and readers.



```C++
Shared Memory Layout:
┌────────────────────────────────────────┐  offset 0
│  RingBufferHeader                      │
│  ┌──────────────────────────────────┐  │
│  │ config (immutable after init)    │  │
│  │  slot_count, slot_size, etc.     │  │
│  ├──────────────────────────────────┤  │  aligned 64B
│  │ write_idx (writer only)          │  │
│  ├──────────────────────────────────┤  │  aligned 64B
│  │ subscriber_mask + futex_word     │  │
│  ├──────────────────────────────────┤  │  aligned 64B
│  │ read_idx[0..MAX_SUBSCRIBERS-1]   │  │  each aligned 64B
│  └──────────────────────────────────┘  │
├────────────────────────────────────────┤
│  Slot[0]: [hdr 20B | payload ...]     │
│  Slot[1]: [hdr 20B | payload ...]     │
│  ...                                   │
│  Slot[N-1]: [hdr 20B | payload ...]   │
└────────────────────────────────────────┘
```
 


    
## Public Attributes Documentation




### variable futex\_word 

_Futex word used for subscriber wake signaling._ 
```C++
std::atomic<uint32_t> conduit::internal::RingBufferHeader::futex_word;
```




<hr>



### variable max\_subscribers 

_Maximum reader slots._ 
```C++
uint32_t conduit::internal::RingBufferHeader::max_subscribers;
```




<hr>



### variable padding0 

_Padding for alignment._ 
```C++
uint32_t conduit::internal::RingBufferHeader::padding0;
```




<hr>



### variable read\_idx 

_Per-reader current read index (each on own cache line)._ 
```C++
AlignedAtomicU64 conduit::internal::RingBufferHeader::read_idx[MAX_SUBSCRIBERS];
```




<hr>



### variable slot\_count 

_Number of slots._ 
```C++
uint32_t conduit::internal::RingBufferHeader::slot_count;
```




<hr>



### variable slot\_size 

_Bytes per slot._ 
```C++
uint32_t conduit::internal::RingBufferHeader::slot_size;
```




<hr>



### variable subscriber\_mask 

_Bitmask of claimed subscriber slots (own cache line)._ 
```C++
std::atomic<uint32_t> conduit::internal::RingBufferHeader::subscriber_mask;
```




<hr>



### variable write\_idx 

_Writer's next write index (own cache line to avoid false sharing)._ 
```C++
std::atomic<uint64_t> conduit::internal::RingBufferHeader::write_idx;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/internal/ring_buffer.hpp`

