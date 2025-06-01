#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <memory>

// Global variables
static pid_t target_pid;
static uintptr_t lib_base = 0;
static bool is_running = false;

#include "utils/logger.h"
#include "utils/process.h"
#include "utils/structs.h"
#include "utils/memory.h"
#include "utils/offset.h"
#include "utils/utils.h"
#include "utils/ue4.h"
#include "utils/socket_server.h"
#include "utils/frame_time.h"



#endif // MAIN_H