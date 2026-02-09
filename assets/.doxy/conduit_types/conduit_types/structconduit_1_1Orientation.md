

# Struct conduit::Orientation



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**Orientation**](structconduit_1_1Orientation.md)



_Quaternion orientation (x, y, z, w) with Euler angle conversions._ [More...](#detailed-description)

* `#include <orientation.hpp>`



Inherits the following classes: [conduit::FixedMessageType](structconduit_1_1FixedMessageType.md)






















## Public Attributes

| Type | Name |
| ---: | :--- |
|  double | [**w**](#variable-w)  <br>_Quaternion W (scalar) component._  |
|  double | [**x**](#variable-x)  <br>_Quaternion X component._  |
|  double | [**y**](#variable-y)  <br>_Quaternion Y component._  |
|  double | [**z**](#variable-z)  <br>_Quaternion Z component._  |
































## Public Functions

| Type | Name |
| ---: | :--- |
|  [**Vec3**](structconduit_1_1Vec3.md) | [**to\_euler**](#function-to_euler) (EulerOrder order=EulerOrder::ZYX) const<br>_Convert quaternion to Euler angles._  |
|  double | [**to\_yaw**](#function-to_yaw) () const<br>_Extract the yaw angle from the quaternion (convenience for 2D)._  |




## Public Static Functions

| Type | Name |
| ---: | :--- |
|  [**Orientation**](structconduit_1_1Orientation.md) | [**from\_euler**](#function-from_euler) (double roll, double pitch, double yaw, EulerOrder order=EulerOrder::ZYX) <br>_Create a quaternion from Euler angles (roll, pitch, yaw)._  |
|  [**Orientation**](structconduit_1_1Orientation.md) | [**from\_yaw**](#function-from_yaw) (double yaw) <br>_Create a quaternion from a yaw-only rotation (convenience for 2D)._  |














































## Protected Functions inherited from conduit::FixedMessageType

See [conduit::FixedMessageType](structconduit_1_1FixedMessageType.md)

| Type | Name |
| ---: | :--- |
|   | [**FixedMessageType**](structconduit_1_1FixedMessageType.md#function-fixedmessagetype) () = default<br> |






## Detailed Description


Represents a 3D rotation as a unit quaternion. Provides factory methods for constructing from Euler angles and extracting them back. 


    
## Public Attributes Documentation




### variable w 

_Quaternion W (scalar) component._ 
```C++
double conduit::Orientation::w;
```




<hr>



### variable x 

_Quaternion X component._ 
```C++
double conduit::Orientation::x;
```




<hr>



### variable y 

_Quaternion Y component._ 
```C++
double conduit::Orientation::y;
```




<hr>



### variable z 

_Quaternion Z component._ 
```C++
double conduit::Orientation::z;
```




<hr>
## Public Functions Documentation




### function to\_euler 

_Convert quaternion to Euler angles._ 
```C++
inline Vec3 conduit::Orientation::to_euler (
    EulerOrder order=EulerOrder::ZYX
) const
```





**Parameters:**


* `order` Euler angle convention (default ZYX). 



**Returns:**

[**Vec3**](structconduit_1_1Vec3.md) with x=roll, y=pitch, z=yaw in radians. 





        

<hr>



### function to\_yaw 

_Extract the yaw angle from the quaternion (convenience for 2D)._ 
```C++
inline double conduit::Orientation::to_yaw () const
```





**Returns:**

Yaw angle in radians. 





        

<hr>
## Public Static Functions Documentation




### function from\_euler 

_Create a quaternion from Euler angles (roll, pitch, yaw)._ 
```C++
static inline Orientation conduit::Orientation::from_euler (
    double roll,
    double pitch,
    double yaw,
    EulerOrder order=EulerOrder::ZYX
) 
```





**Parameters:**


* `roll` Rotation about X axis in radians. 
* `pitch` Rotation about Y axis in radians. 
* `yaw` Rotation about Z axis in radians. 
* `order` Euler angle convention (default ZYX). 



**Returns:**

Unit quaternion representing the rotation. 





        

<hr>



### function from\_yaw 

_Create a quaternion from a yaw-only rotation (convenience for 2D)._ 
```C++
static inline Orientation conduit::Orientation::from_yaw (
    double yaw
) 
```





**Parameters:**


* `yaw` Rotation about Z axis in radians. 



**Returns:**

Unit quaternion representing the rotation. 





        

<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_types/include/conduit_types/derived/orientation.hpp`

