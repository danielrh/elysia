#ifndef _BRAIN_CORE_PLATFORM_HPP_
#define _BRAIN_CORE_PLATFORM_HPP_
#include <cstdint>
#include "core/util/Platform.hpp"
#ifndef BRAIN_CORE_EXPORT

# ifdef _WIN32
#   if defined(STATIC_LINKED)
#     define BRAIN_CORE_EXPORT
#   else
#     if defined(BRAIN_CORE_BUILD)
#       define BRAIN_CORE_EXPORT __declspec(dllexport)
#     else
#       define BRAIN_CORE_EXPORT __declspec(dllimport)
#     endif
#   endif
#   define BRAIN_CORE_PLUGIN_EXPORT __declspec(dllexport)
# else
#   if defined(STATIC_LINKED)
#     define BRAIN_CORE_EXPORT
#     define BRAIN_CORE_PLUGIN_EXPORT
#   else
#     define BRAIN_CORE_EXPORT __attribute__ ((visibility("default")))
#     define BRAIN_CORE_PLUGIN_EXPORT __attribute__ ((visibility("default")))
#   endif
# endif
#endif
namespace Elysia {
    //using Sirikata::String;
    //using Sirikata::Vector3f;
    //using Sirikata::Vector3d;
    //using Sirikata::Vector4f;
    //using Sirikata::Vector4d;
    //using Sirikata::BoundingBox3f3f;
    using std::int64_t;
    using std::uint64_t;
class SimTime {
    std::int64_t time;
public:
    SimTime() {
        time=0;
    }
    std::int64_t getRawTime()const {
        return time;
    }
    bool operator !=(const SimTime&other) const{
        return other.time!=time;
    }
    bool operator ==(const SimTime&other) const{
        return other.time==time;
    }
    SimTime& operator ++() {
        ++time;
        return *this;
    }
};
}
#endif
