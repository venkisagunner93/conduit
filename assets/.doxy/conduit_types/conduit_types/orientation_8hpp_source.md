

# File orientation.hpp

[**File List**](files.md) **>** [**conduit\_types**](dir_2f8116ad873fa9f4ab4bc95e5e2c7e48.md) **>** [**include**](dir_b494fed11488e6772c8fe9a92fd0e861.md) **>** [**conduit\_types**](dir_5c67974ec98d7ff0f95b9e09c6bc682d.md) **>** [**derived**](dir_6c611f3cf964ed6f858fc2fcf2f339e5.md) **>** [**orientation.hpp**](orientation_8hpp.md)

[Go to the documentation of this file](orientation_8hpp.md)


```C++
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

    static Orientation from_yaw(double yaw) {
        Orientation q{};
        q.z = std::sin(yaw * 0.5);
        q.w = std::cos(yaw * 0.5);
        return q;
    }

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

    double to_yaw() const {
        return std::atan2(2.0 * (w * z + x * y), 1.0 - 2.0 * (y * y + z * z));
    }
};

}  // namespace conduit
```


