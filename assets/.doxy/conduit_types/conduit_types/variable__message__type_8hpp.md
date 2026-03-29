

# File variable\_message\_type.hpp



[**FileList**](files.md) **>** [**conduit\_types**](dir_2f8116ad873fa9f4ab4bc95e5e2c7e48.md) **>** [**include**](dir_b494fed11488e6772c8fe9a92fd0e861.md) **>** [**conduit\_types**](dir_5c67974ec98d7ff0f95b9e09c6bc682d.md) **>** [**variable\_message\_type.hpp**](variable__message__type_8hpp.md)

[Go to the source code of this file](variable__message__type_8hpp_source.md)

_Base class and compile-time validation for variable-size messages._ [More...](#detailed-description)

* `#include <cstddef>`
* `#include <cstdint>`
* `#include <type_traits>`













## Namespaces

| Type | Name |
| ---: | :--- |
| namespace | [**conduit**](namespaceconduit.md) <br> |
| namespace | [**detail**](namespaceconduit_1_1detail.md) <br> |


## Classes

| Type | Name |
| ---: | :--- |
| class | [**VariableMessageType**](classconduit_1_1VariableMessageType.md) <br>_Base class for variable-size messages requiring serialization._  |


















































## Detailed Description


Rules for variable message types:
* Derive from VariableMessageType
* Implement serialized\_size() and serialize(uint8\_t\*)
* Provide a static deserialize(const uint8\_t\*, size\_t) -&gt; T method
* Must NOT be trivially copyable (enforced as sanity check)






**See also:** validate\_variable\_message\_type, FixedMessageType 



    

------------------------------
The documentation for this class was generated from the following file `packages/conduit_types/include/conduit_types/variable_message_type.hpp`

