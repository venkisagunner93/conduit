

# Struct conduit::Twist



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**Twist**](structconduit_1_1Twist.md)



_Linear and angular velocity in 3D._ 

* `#include <twist.hpp>`



Inherits the following classes: [conduit::FixedMessageType](structconduit_1_1FixedMessageType.md)






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**Vec3**](structconduit_1_1Vec3.md) | [**angular**](#variable-angular)  <br>_Angular velocity (rad/s) in x, y, z._  |
|  [**Header**](structconduit_1_1Header.md) | [**header**](#variable-header)  <br>_Timestamp and coordinate frame._  |
|  [**Vec3**](structconduit_1_1Vec3.md) | [**linear**](#variable-linear)  <br>_Linear velocity (m/s) in x, y, z._  |


















































































## Protected Functions inherited from conduit::FixedMessageType

See [conduit::FixedMessageType](structconduit_1_1FixedMessageType.md)

| Type | Name |
| ---: | :--- |
|   | [**FixedMessageType**](structconduit_1_1FixedMessageType.md#function-fixedmessagetype) () = default<br> |






## Public Attributes Documentation




### variable angular 

_Angular velocity (rad/s) in x, y, z._ 
```C++
Vec3 conduit::Twist::angular;
```




<hr>



### variable header 

_Timestamp and coordinate frame._ 
```C++
Header conduit::Twist::header;
```




<hr>



### variable linear 

_Linear velocity (m/s) in x, y, z._ 
```C++
Vec3 conduit::Twist::linear;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_types/include/conduit_types/derived/twist.hpp`

