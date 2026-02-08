

# Namespace conduit



[**Namespace List**](namespaces.md) **>** [**conduit**](namespaceconduit.md)


















## Namespaces

| Type | Name |
| ---: | :--- |
| namespace | [**detail**](namespaceconduit_1_1detail.md) <br> |


## Classes

| Type | Name |
| ---: | :--- |
| struct | [**Bool**](structconduit_1_1Bool.md) <br>_Fixed-size boolean message type._  |
| struct | [**Double**](structconduit_1_1Double.md) <br>_Fixed-size double-precision floating point message type._  |
| struct | [**FixedMessageType**](structconduit_1_1FixedMessageType.md) <br>_Base class for fixed-size, trivially copyable message types._  |
| struct | [**Header**](structconduit_1_1Header.md) <br>_Common message header with timestamp and coordinate frame._  |
| struct | [**Imu**](structconduit_1_1Imu.md) <br>_Inertial Measurement Unit data (orientation, angular velocity, linear acceleration)._  |
| struct | [**Int**](structconduit_1_1Int.md) <br>_Fixed-size signed 64-bit integer message type._  |
| struct | [**Odometry**](structconduit_1_1Odometry.md) <br>[_**Odometry**_](structconduit_1_1Odometry.md) _message with pose and velocity._ |
| struct | [**Orientation**](structconduit_1_1Orientation.md) <br>_Quaternion orientation (x, y, z, w) with Euler angle conversions._  |
| struct | [**Pose2D**](structconduit_1_1Pose2D.md) <br>_2D pose with position and orientation._  |
| struct | [**Pose3D**](structconduit_1_1Pose3D.md) <br>_3D pose with position and orientation._  |
| class | [**ReadBuffer**](classconduit_1_1ReadBuffer.md) <br>_Sequential binary reader for deserializing message fields._  |
| struct | [**Time**](structconduit_1_1Time.md) <br>_Fixed-size timestamp message type._  |
| struct | [**Twist**](structconduit_1_1Twist.md) <br>_Linear and angular velocity in 3D._  |
| struct | [**Uint**](structconduit_1_1Uint.md) <br>_Fixed-size unsigned 64-bit integer message type._  |
| class | [**VariableMessageType**](classconduit_1_1VariableMessageType.md) <br>_Base class for variable-size messages requiring serialization._  |
| struct | [**Vec2**](structconduit_1_1Vec2.md) <br>_Fixed-size 2D vector message type._  |
| struct | [**Vec3**](structconduit_1_1Vec3.md) <br>_Fixed-size 3D vector message type._  |
| class | [**WriteBuffer**](classconduit_1_1WriteBuffer.md) <br>_Sequential binary writer for serializing message fields._  |


## Public Types

| Type | Name |
| ---: | :--- |
| enum  | [**EulerOrder**](#enum-eulerorder)  <br>_Euler angle rotation order._  |




















## Public Functions

| Type | Name |
| ---: | :--- |
|  void | [**set\_frame**](#function-set_frame) (char(&) dst, const char \* src) <br>_Safely copy a frame string into a_ [_**Header**_](structconduit_1_1Header.md) _frame field._ |
|  constexpr void | [**validate\_fixed\_message\_type**](#function-validate_fixed_message_type) () <br>_Compile-time validation that T is a valid fixed message type._  |
|  constexpr void | [**validate\_variable\_message\_type**](#function-validate_variable_message_type) () <br>_Compile-time validation that T is a valid variable message type._  |




























## Public Types Documentation




### enum EulerOrder 

_Euler angle rotation order._ 
```C++
enum conduit::EulerOrder {
    ZYX,
    XYZ
};
```




<hr>
## Public Functions Documentation




### function set\_frame 

_Safely copy a frame string into a_ [_**Header**_](structconduit_1_1Header.md) _frame field._
```C++
inline void conduit::set_frame (
    char(&) dst,
    const char * src
) 
```





**Parameters:**


* `dst` Destination frame array (64 bytes, will be null-terminated). 
* `src` Source null-terminated string. 




        

<hr>



### function validate\_fixed\_message\_type 

_Compile-time validation that T is a valid fixed message type._ 
```C++
template<typename T>
constexpr void conduit::validate_fixed_message_type () 
```



Checks that T derives from [**FixedMessageType**](structconduit_1_1FixedMessageType.md), is trivially copyable, is standard layout, and is non-empty.




**Template parameters:**


* `T` The type to validate. 




        

<hr>



### function validate\_variable\_message\_type 

_Compile-time validation that T is a valid variable message type._ 
```C++
template<typename T>
constexpr void conduit::validate_variable_message_type () 
```



Checks that T derives from [**VariableMessageType**](classconduit_1_1VariableMessageType.md), is not trivially copyable, and provides a static `T deserialize(const uint8_t*, size_t)` method.




**Template parameters:**


* `T` The type to validate. 




        

<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_types/include/conduit_types/buffer.hpp`

