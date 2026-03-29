

# File ring\_buffer.hpp



[**FileList**](files.md) **>** [**conduit\_core**](dir_e8f2a2a6c23a8405cb81447580d9d2d9.md) **>** [**include**](dir_e280fcca63980b3efd6dff25fbf50072.md) **>** [**conduit\_core**](dir_5b31385b73e3aff376fcc12a1f7e93dd.md) **>** [**internal**](dir_ee09fc54beaf3ba0ab27276c1e0adb4c.md) **>** [**ring\_buffer.hpp**](ring__buffer_8hpp.md)

[Go to the source code of this file](ring__buffer_8hpp_source.md)



* `#include <atomic>`
* `#include <chrono>`
* `#include <cstddef>`
* `#include <cstdint>`
* `#include <optional>`













## Namespaces

| Type | Name |
| ---: | :--- |
| namespace | [**conduit**](namespaceconduit.md) <br> |
| namespace | [**internal**](namespaceconduit_1_1internal.md) <br>_Internal implementation details for conduit._  |


## Classes

| Type | Name |
| ---: | :--- |
| struct | [**AlignedAtomicU64**](structconduit_1_1internal_1_1AlignedAtomicU64.md) <br>_Cache-line-aligned atomic uint64\_t to prevent false sharing._  |
| struct | [**ReadResult**](structconduit_1_1internal_1_1ReadResult.md) <br>_Result of a successful ring buffer read._  |
| struct | [**RingBufferConfig**](structconduit_1_1internal_1_1RingBufferConfig.md) <br>_Ring buffer configuration._  |
| struct | [**RingBufferHeader**](structconduit_1_1internal_1_1RingBufferHeader.md) <br>_Shared memory layout for the ring buffer control structure._  |
| class | [**RingBufferReader**](classconduit_1_1internal_1_1RingBufferReader.md) <br>_Reader side of the lock-free SPMC ring buffer._  |
| class | [**RingBufferWriter**](classconduit_1_1internal_1_1RingBufferWriter.md) <br>_Writer side of the lock-free SPMC ring buffer._  |



















































------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/internal/ring_buffer.hpp`

