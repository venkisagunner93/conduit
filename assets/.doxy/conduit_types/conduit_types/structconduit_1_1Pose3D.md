

# Struct conduit::Pose3D



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**Pose3D**](structconduit_1_1Pose3D.md)



_3D pose with position and orientation._ 

* `#include <pose3d.hpp>`



Inherits the following classes: [conduit::FixedMessageType](structconduit_1_1FixedMessageType.md)






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**Header**](structconduit_1_1Header.md) | [**header**](#variable-header)  <br>_Timestamp and coordinate frame._  |
|  [**Orientation**](structconduit_1_1Orientation.md) | [**orientation**](#variable-orientation)  <br>_Quaternion orientation._  |
|  [**Vec3**](structconduit_1_1Vec3.md) | [**position**](#variable-position)  <br>_3D position (x, y, z)._  |


















































































## Protected Functions inherited from conduit::FixedMessageType

See [conduit::FixedMessageType](structconduit_1_1FixedMessageType.md)

| Type | Name |
| ---: | :--- |
|   | [**FixedMessageType**](structconduit_1_1FixedMessageType.md#function-fixedmessagetype) () = default<br> |






## Public Attributes Documentation




### variable header 

_Timestamp and coordinate frame._ 
```C++
Header conduit::Pose3D::header;
```




<hr>



### variable orientation 

_Quaternion orientation._ 
```C++
Orientation conduit::Pose3D::orientation;
```




<hr>



### variable position 

_3D position (x, y, z)._ 
```C++
Vec3 conduit::Pose3D::position;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_types/include/conduit_types/derived/pose3d.hpp`

