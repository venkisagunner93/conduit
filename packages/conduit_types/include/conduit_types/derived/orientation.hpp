#pragma once

#include "conduit_types/fixed_message_type.hpp"
#include "conduit_types/primitives/vec3.hpp"

#include <cmath>

namespace conduit {

enum class EulerOrder { ZYX, XYZ };

struct Orientation : FixedMessageType {
    double x;
    double y;
    double z;
    double w;

    // Euler angles (roll, pitch, yaw) to quaternion
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

    // Yaw-only rotation (convenience for 2D)
    static Orientation from_yaw(double yaw) {
        Orientation q{};
        q.z = std::sin(yaw * 0.5);
        q.w = std::cos(yaw * 0.5);
        return q;
    }

    // Quaternion to Euler angles → Vec3{roll, pitch, yaw}
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

    // Extract yaw (convenience for 2D)
    double to_yaw() const {
        return std::atan2(2.0 * (w * z + x * y), 1.0 - 2.0 * (y * y + z * z));
    }
};

}  // namespace conduit
