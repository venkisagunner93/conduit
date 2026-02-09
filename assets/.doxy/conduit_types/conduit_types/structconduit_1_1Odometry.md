

# Struct conduit::Odometry



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**Odometry**](structconduit_1_1Odometry.md)



[_**Odometry**_](structconduit_1_1Odometry.md) _message with pose and velocity._

* `#include <odometry.hpp>`



Inherits the following classes: [conduit::FixedMessageType](structconduit_1_1FixedMessageType.md)






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**Vec3**](structconduit_1_1Vec3.md) | [**angular\_velocity**](#variable-angular_velocity)  <br>_Angular velocity (rad/s)._  |
|  char | [**child\_frame**](#variable-child_frame)  <br>_Child coordinate frame (null-terminated)._  |
|  [**Header**](structconduit_1_1Header.md) | [**header**](#variable-header)  <br>_Timestamp and coordinate frame._  |
|  [**Vec3**](structconduit_1_1Vec3.md) | [**linear\_velocity**](#variable-linear_velocity)  <br>_Linear velocity (m/s)._  |
|  [**Pose3D**](structconduit_1_1Pose3D.md) | [**pose**](#variable-pose)  <br>_3D pose estimate._  |


















































































## Protected Functions inherited from conduit::FixedMessageType

See [conduit::FixedMessageType](structconduit_1_1FixedMessageType.md)

| Type | Name |
| ---: | :--- |
|   | [**FixedMessageType**](structconduit_1_1FixedMessageType.md#function-fixedmessagetype) () = default<br> |






## Public Attributes Documentation




### variable angular\_velocity 

_Angular velocity (rad/s)._ 
```C++
Vec3 conduit::Odometry::angular_velocity;
```




<hr>



### variable child\_frame 

_Child coordinate frame (null-terminated)._ 
```C++
char conduit::Odometry::child_frame[64];
```




<hr>



### variable header 

_Timestamp and coordinate frame._ 
```C++
Header conduit::Odometry::header;
```




<hr>



### variable linear\_velocity 

_Linear velocity (m/s)._ 
```C++
Vec3 conduit::Odometry::linear_velocity;
```




<hr>



### variable pose 

_3D pose estimate._ 
```C++
Pose3D conduit::Odometry::pose;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_types/include/conduit_types/derived/odometry.hpp`

