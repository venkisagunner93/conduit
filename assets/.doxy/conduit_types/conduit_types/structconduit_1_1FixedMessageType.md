

# Struct conduit::FixedMessageType



[**ClassList**](annotated.md) **>** [**conduit**](namespaceconduit.md) **>** [**FixedMessageType**](structconduit_1_1FixedMessageType.md)



_Base class for fixed-size, trivially copyable message types._ [More...](#detailed-description)

* `#include <fixed_message_type.hpp>`





Inherited by the following classes: [conduit::Bool](structconduit_1_1Bool.md),  [conduit::Double](structconduit_1_1Double.md),  [conduit::Imu](structconduit_1_1Imu.md),  [conduit::Int](structconduit_1_1Int.md),  [conduit::Odometry](structconduit_1_1Odometry.md),  [conduit::Orientation](structconduit_1_1Orientation.md),  [conduit::Pose2D](structconduit_1_1Pose2D.md),  [conduit::Pose3D](structconduit_1_1Pose3D.md),  [conduit::Time](structconduit_1_1Time.md),  [conduit::Twist](structconduit_1_1Twist.md),  [conduit::Uint](structconduit_1_1Uint.md),  [conduit::Vec2](structconduit_1_1Vec2.md),  [conduit::Vec3](structconduit_1_1Vec3.md)
























































## Protected Functions

| Type | Name |
| ---: | :--- |
|   | [**FixedMessageType**](#function-fixedmessagetype) () = default<br> |




## Detailed Description


Derive from this to create messages that are transmitted via zero-copy memcpy. The type must be trivially copyable and standard layout.




**See also:** [**validate\_fixed\_message\_type**](namespaceconduit.md#function-validate_fixed_message_type) 



    
## Protected Functions Documentation




### function FixedMessageType 

```C++
conduit::FixedMessageType::FixedMessageType () = default
```




<hr>

------------------------------
The documentation for this class was generated from the following file `packages/conduit_types/include/conduit_types/fixed_message_type.hpp`

