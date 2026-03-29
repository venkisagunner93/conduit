

# Struct conduit::internal::AlignedAtomicU64



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**internal**](namespaceconduit_1_1internal.md) **>** [**AlignedAtomicU64**](structconduit_1_1internal_1_1AlignedAtomicU64.md)



_Cache-line-aligned atomic uint64\_t to prevent false sharing._ 

* `#include <ring_buffer.hpp>`





















## Public Attributes

| Type | Name |
| ---: | :--- |
|  std::atomic&lt; [**uint64\_t**](classconduit_1_1internal_1_1Publisher.md) &gt; | [**value**](#variable-value)  <br> |












































## Public Attributes Documentation




### variable value 

```C++
std::atomic<uint64_t> conduit::internal::AlignedAtomicU64::value;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/internal/ring_buffer.hpp`

