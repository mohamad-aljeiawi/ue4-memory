#ifndef LOGGER_H
#define LOGGER_H

namespace Logger
{
    void d(const char *fmt, ...);
    void i(const char *fmt, ...);
    void w(const char *fmt, ...);
    void e(const char *fmt, ...);
}

#endif // LOGGER_H
