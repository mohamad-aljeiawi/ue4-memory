#ifndef MAIN_H
#define MAIN_H

#include "debug/logger.h"
#include "types/offset.h"
#include "types/structs.h"
#include "types/structs_data.h"
#include "utils/utils.h"

#include "utils/process.h"
#include "utils/memory.h"
#include "utils/ue4.h"
#include "utils/socket_server.h"
#include "utils/socket_client.h"
#include "utils/touch.h"

#include "native/a_native_window_creator.h"
#include "native/imgui_renderer.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_android.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include <sstream>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>
#include <thread>
#ifdef __ANDROID__
#include <pthread.h>
#include <sys/resource.h>
#endif

#endif // MAIN_H