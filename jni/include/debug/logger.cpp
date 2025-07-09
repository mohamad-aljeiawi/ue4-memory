#include "debug/logger.h"
#include <android/log.h>
#include <cstdarg>

#define TAG "UE4_MEMORY_LOGGER"

namespace Logger
{

    void d(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        __android_log_vprint(ANDROID_LOG_DEBUG, TAG, fmt, args);
        va_end(args);
    }

    void i(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        __android_log_vprint(ANDROID_LOG_INFO, TAG, fmt, args);
        va_end(args);
    }

    void w(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        __android_log_vprint(ANDROID_LOG_WARN, TAG, fmt, args);
        va_end(args);
    }

    void e(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        __android_log_vprint(ANDROID_LOG_ERROR, TAG, fmt, args);
        va_end(args);
    }

}
