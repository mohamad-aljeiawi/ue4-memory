APP_ABI := arm64-v8a
APP_PLATFORM := android-24
APP_STL := c++_static
APP_CPPFLAGS := -std=c++20 -fexceptions -frtti -Wall -Wextra
APP_OPTIM := release
APP_LDFLAGS += -latomic