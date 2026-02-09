
# Class Hierarchy

This inheritance list is sorted roughly, but not completely, alphabetically:


* **class** [**conduit::ReadBuffer**](classconduit_1_1ReadBuffer.md) _Sequential binary reader for deserializing message fields._ 
* **class** [**conduit::VariableMessageType**](classconduit_1_1VariableMessageType.md) _Base class for variable-size messages requiring serialization._ 
* **class** [**conduit::WriteBuffer**](classconduit_1_1WriteBuffer.md) _Sequential binary writer for serializing message fields._ 
* **struct** [**conduit::FixedMessageType**](structconduit_1_1FixedMessageType.md) _Base class for fixed-size, trivially copyable message types._     
    * **struct** [**conduit::Bool**](structconduit_1_1Bool.md) _Fixed-size boolean message type._ 
    * **struct** [**conduit::Double**](structconduit_1_1Double.md) _Fixed-size double-precision floating point message type._ 
    * **struct** [**conduit::Imu**](structconduit_1_1Imu.md) _Inertial Measurement Unit data (orientation, angular velocity, linear acceleration)._ 
    * **struct** [**conduit::Int**](structconduit_1_1Int.md) _Fixed-size signed 64-bit integer message type._ 
    * **struct** [**conduit::Odometry**](structconduit_1_1Odometry.md) [_**Odometry**_](structconduit_1_1Odometry.md) _message with pose and velocity._
    * **struct** [**conduit::Orientation**](structconduit_1_1Orientation.md) _Quaternion orientation (x, y, z, w) with Euler angle conversions._ 
    * **struct** [**conduit::Pose2D**](structconduit_1_1Pose2D.md) _2D pose with position and orientation._ 
    * **struct** [**conduit::Pose3D**](structconduit_1_1Pose3D.md) _3D pose with position and orientation._ 
    * **struct** [**conduit::Time**](structconduit_1_1Time.md) _Fixed-size timestamp message type._ 
    * **struct** [**conduit::Twist**](structconduit_1_1Twist.md) _Linear and angular velocity in 3D._ 
    * **struct** [**conduit::Uint**](structconduit_1_1Uint.md) _Fixed-size unsigned 64-bit integer message type._ 
    * **struct** [**conduit::Vec2**](structconduit_1_1Vec2.md) _Fixed-size 2D vector message type._ 
    * **struct** [**conduit::Vec3**](structconduit_1_1Vec3.md) _Fixed-size 3D vector message type._ 
* **struct** [**conduit::Header**](structconduit_1_1Header.md) _Common message header with timestamp and coordinate frame._ 

