LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := cping_memory
LOCAL_CFLAGS := -Wno-error=format-security -fvisibility=hidden -ffunction-sections -fdata-sections -w
LOCAL_CPPFLAGS := -std=c++17
LOCAL_LDLIBS := -llog

# Include paths
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/utils

# Source files
FILE_LIST := $(wildcard $(LOCAL_PATH)/src/*.cpp)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

include $(BUILD_EXECUTABLE)



# LOCAL_PATH := $(call my-dir)

# include $(CLEAR_VARS)
# LOCAL_MODULE := cping
# LOCAL_CFLAGS := -Wno-error=format-security -fvisibility=hidden -ffunction-sections -fdata-sections -w
# LOCAL_CPPFLAGS := -std=c++17
# LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2 -lOpenSLES

# # Include paths
# LOCAL_C_INCLUDES := \
#     $(LOCAL_PATH)/include \
#     $(LOCAL_PATH)/include/utils \
#     $(LOCAL_PATH)/include/sdl \
#     $(LOCAL_PATH)/include/sdl/SDL3 \
#     $(LOCAL_PATH)/include/sdl/build_config

# # Source files
# FILE_LIST := $(wildcard $(LOCAL_PATH)/src/*.cpp)
# LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

# LOCAL_STATIC_LIBRARIES := SDL3

# include $(BUILD_EXECUTABLE)

# # SDL3 prebuilt static library
# include $(CLEAR_VARS)
# LOCAL_MODULE := SDL3
# LOCAL_SRC_FILES := lib/sdl/$(TARGET_ARCH_ABI)/libSDL3.a
# include $(PREBUILT_STATIC_LIBRARY)