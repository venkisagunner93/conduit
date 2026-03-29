

# Struct conduit::Imu



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**Imu**](structconduit_1_1Imu.md)



_Inertial Measurement Unit data (orientation, angular velocity, linear acceleration)._ 

* `#include <imu.hpp>`



Inherits the following classes: [conduit::FixedMessageType](structconduit_1_1FixedMessageType.md)






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  [**Vec3**](structconduit_1_1Vec3.md) | [**angular\_velocity**](#variable-angular_velocity)  <br>_Angular velocity (rad/s)._  |
|  [**Header**](structconduit_1_1Header.md) | [**header**](#variable-header)  <br>_Timestamp and coordinate frame._  |
|  [**Vec3**](structconduit_1_1Vec3.md) | [**linear\_acceleration**](#variable-linear_acceleration)  <br>_Linear acceleration (m/s^2)._  |
|  [**Orientation**](structconduit_1_1Orientation.md) | [**orientation**](#variable-orientation)  <br>_Quaternion orientation estimate._  |


















































































## Protected Functions inherited from conduit::FixedMessageType

See [conduit::FixedMessageType](structconduit_1_1FixedMessageType.md)

| Type | Name |
| ---: | :--- |
|   | [**FixedMessageType**](structconduit_1_1FixedMessageType.md#function-fixedmessagetype) () = default<br> |






## Public Attributes Documentation




### variable angular\_velocity 

_Angular velocity (rad/s)._ 
```C++
Vec3 conduit::Imu::angular_velocity;
```




<hr>



### variable header 

_Timestamp and coordinate frame._ 
```C++
Header conduit::Imu::header;
```




<hr>



### variable linear\_acceleration 

_Linear acceleration (m/s^2)._ 
```C++
Vec3 conduit::Imu::linear_acceleration;
```




<hr>



### variable orientation 

_Quaternion orientation estimate._ 
```C++
Orientation conduit::Imu::orientation;
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_types/include/conduit_types/derived/imu.hpp`

