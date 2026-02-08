

# Struct conduit::Pose2D



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**Pose2D**](structconduit_1_1Pose2D.md)



_2D pose with position and orientation._ 

* `#include <pose2d.hpp>`



Inherits the following classes: [conduit::FixedMessageType](structconduit_1_1FixedMessageType.md)






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**Header**](structconduit_1_1Header.md) | [**header**](#variable-header)  <br>_Timestamp and coordinate frame._  |
|  [**Orientation**](structconduit_1_1Orientation.md) | [**orientation**](#variable-orientation)  <br>_Quaternion orientation (typically yaw-only for 2D)._  |
|  [**Vec2**](structconduit_1_1Vec2.md) | [**position**](#variable-position)  <br>_2D position (x, y)._  |


















































































## Protected Functions inherited from conduit::FixedMessageType

See [conduit::FixedMessageType](structconduit_1_1FixedMessageType.md)

| Type | Name |
| ---: | :--- |
|   | [**FixedMessageType**](structconduit_1_1FixedMessageType.md#function-fixedmessagetype) () = default<br> |






## Public Attributes Documentation




### variable header 

_Timestamp and coordinate frame._ 
```C++
Header conduit::Pose2D::header;
```




<hr>



### variable orientation 

_Quaternion orientation (typically yaw-only for 2D)._ 
```C++
Orientation conduit::Pose2D::orientation;
```




<hr>



### variable position 

_2D position (x, y)._ 
```C++
Vec2 conduit::Pose2D::position;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_types/include/conduit_types/derived/pose2d.hpp`

