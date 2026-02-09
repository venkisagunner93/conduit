

# File fixed\_message\_type.hpp



[**FileList**](files.md) **>** [**conduit\_types**](dir_2f8116ad873fa9f4ab4bc95e5e2c7e48.md) **>** [**include**](dir_b494fed11488e6772c8fe9a92fd0e861.md) **>** [**conduit\_types**](dir_5c67974ec98d7ff0f95b9e09c6bc682d.md) **>** [**fixed\_message\_type.hpp**](fixed__message__type_8hpp.md)

[Go to the source code of this file](fixed__message__type_8hpp_source.md)

_Base class and compile-time validation for fixed-size messages._ [More...](#detailed-description)

* `#include <cstddef>`
* `#include <type_traits>`













## Namespaces

| Type | Name |
| ---: | :--- |
| namespace | [**conduit**](namespaceconduit.md) <br> |


## Classes

| Type | Name |
| ---: | :--- |
| struct | [**FixedMessageType**](structconduit_1_1FixedMessageType.md) <br>_Base class for fixed-size, trivially copyable message types._  |


















































## Detailed Description


Rules for fixed message types:
* Derive from FixedMessageType
* Must be trivially copyable (no pointers, no std::string, no virtuals)
* Must be standard layout
* Published via memcpy — no serialization needed






**See also:** validate\_fixed\_message\_type, VariableMessageType 



    

------------------------------
The documentation for this class was generated from the following file `packages/conduit_types/include/conduit_types/fixed_message_type.hpp`

