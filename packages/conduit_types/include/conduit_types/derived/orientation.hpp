#pragma once

#include "conduit_types/fixed_message_type.hpp"
#include "conduit_types/primitives/vec3.hpp"

#include <cmath>

namespace conduit {

/// @brief Euler angle rotation order.
enum class EulerOrder { ZYX, XYZ };

/// @brief Quaternion orientation (x, y, z, w) with Euler angle conversions.
///
/// Represents a 3D rotation as a unit quaternion. Provides factory methods
/// for constructing from Euler angles and extracting them back.
struct Orientation : FixedMessageType {
    double x;  ///< Quaternion X component.
    double y;  ///< Quaternion Y component.
    double z;  ///< Quaternion Z component.
    double w;  ///< Quaternion W (scalar) component.

    /// @brief Create a quaternion from Euler angles (roll, pitch, yaw).
    /// @param roll Rotation about X axis in radians.
    /// @param pitch Rotation about Y axis in radians.
    /// @param yaw Rotation about Z axis in radians.
    /// @param order Euler angle convention (default ZYX).
    /// @return Unit quaternion representing the rotation.
    static Orientation from_euler(double roll, double pitch, double yaw,
                                  EulerOrder order = EulerOrder::ZYX) {
        const double cr = std::cos(roll * 0.5), sr = std::sin(roll * 0.5);
        const double cp = std::cos(pitch * 0.5), sp = std::sin(pitch * 0.5);
        const double cy = std::cos(yaw * 0.5), sy = std::sin(yaw * 0.5);

        Orientation q{};
        if (order == EulerOrder::ZYX) {
            q.w = cr * cp * cy + sr * sp * sy;
            q.x = sr * cp * cy - cr * sp * sy;
            q.y = cr * sp * cy + sr * cp * sy;
            q.z = cr * cp * sy - sr * sp * cy;
        } else {  // XYZ
            q.w = cr * cp * cy - sr * sp * sy;
            q.x = sr * cp * cy + cr * sp * sy;
            q.y = cr * sp * cy - sr * cp * sy;
            q.z = cr * cp * sy + sr * sp * cy;
        }
        return q;
    }

    /// @brief Create a quaternion from a yaw-only rotation (convenience for 2D).
    /// @param yaw Rotation about Z axis in radians.
    /// @return Unit quaternion representing the rotation.
    static Orientation from_yaw(double yaw) {
        Orientation q{};
        q.z = std::sin(yaw * 0.5);
        q.w = std::cos(yaw * 0.5);
        return q;
    }

    /// @brief Convert quaternion to Euler angles.
    /// @param order Euler angle convention (default ZYX).
    /// @return Vec3 with x=roll, y=pitch, z=yaw in radians.
    Vec3 to_euler(EulerOrder order = EulerOrder::ZYX) const {
        Vec3 e{};
        if (order == EulerOrder::ZYX) {
            e.x = std::atan2(2.0 * (w * x + y * z), 1.0 - 2.0 * (x * x + y * y));
            double sp = 2.0 * (w * y - z * x);
            e.y = std::asin(sp < -1.0 ? -1.0 : (sp > 1.0 ? 1.0 : sp));
            e.z = std::atan2(2.0 * (w * z + x * y), 1.0 - 2.0 * (y * y + z * z));
        } else {  // XYZ
            e.x = std::atan2(2.0 * (w * x - y * z), 1.0 - 2.0 * (x * x + y * y));
            double sp = 2.0 * (w * y + x * z);
            e.y = std::asin(sp < -1.0 ? -1.0 : (sp > 1.0 ? 1.0 : sp));
            e.z = std::atan2(2.0 * (w * z - x * y), 1.0 - 2.0 * (y * y + z * z));
        }
        return e;
    }

    /// @brief Extract the yaw angle from the quaternion (convenience for 2D).
    /// @return Yaw angle in radians.
    double to_yaw() const {
        return std::atan2(2.0 * (w * z + x * y), 1.0 - 2.0 * (y * y + z * z));
    }
};

}  // namespace conduit
