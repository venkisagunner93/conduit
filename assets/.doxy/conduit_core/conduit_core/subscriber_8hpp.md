

# File subscriber.hpp



[**FileList**](files.md) **>** [**conduit\_core**](dir_e8f2a2a6c23a8405cb81447580d9d2d9.md) **>** [**include**](dir_e280fcca63980b3efd6dff25fbf50072.md) **>** [**conduit\_core**](dir_5b31385b73e3aff376fcc12a1f7e93dd.md) **>** [**subscriber.hpp**](subscriber_8hpp.md)

[Go to the source code of this file](subscriber_8hpp_source.md)



* `#include <chrono>`
* `#include <cstdint>`
* `#include <cstring>`
* `#include <memory>`
* `#include <optional>`
* `#include <string>`
* `#include <type_traits>`
* `#include "conduit_core/internal/ring_buffer.hpp"`
* `#include "conduit_core/internal/shm_region.hpp"`
* `#include <conduit_types/fixed_message_type.hpp>`
* `#include <conduit_types/variable_message_type.hpp>`













## Namespaces

| Type | Name |
| ---: | :--- |
| namespace | [**conduit**](namespaceconduit.md) <br> |
| namespace | [**internal**](namespaceconduit_1_1internal.md) <br>_Internal implementation details for conduit._  |


## Classes

| Type | Name |
| ---: | :--- |
| struct | [**Message**](structconduit_1_1Message.md) <br>_Raw message received from a topic._  |
| struct | [**SubscriberOptions**](structconduit_1_1SubscriberOptions.md) <br>_Configuration for topic subscriber._  |
| class | [**Subscriber**](classconduit_1_1internal_1_1Subscriber.md) <br>_Low-level subscriber that reads raw bytes from a shared memory ring buffer._  |



















































------------------------------
The documentation for this class was generated from the following file `packages/conduit_core/include/conduit_core/subscriber.hpp`

