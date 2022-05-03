#include "platform.h"
namespace platform {



const std::string& get_platform()
{
    static std::string platform_name =
#if defined(__linux) || defined(__linux__)
    "Linux";
#elif defined(__WINDOWS__) || defined(_WIN32) || defined(_WIN64)
    "Windows";
#elif __APPLE__ || __MACH__
    "Mac OS X";
#elif defined(__ANDROID__)
    "Android";
#else
    "Unknown";
#endif

    return platform_name;
}

} // end of platform namespace