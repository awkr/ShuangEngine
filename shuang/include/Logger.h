#pragma once

#include <spdlog/spdlog.h>

#define log_debug(...) spdlog::debug(__VA_ARGS__)
#define log_info(...) spdlog::info(__VA_ARGS__)
#define log_warn(...) spdlog::warn(__VA_ARGS__)
#define log_error(...) spdlog::error(__VA_ARGS__)
#define log_func spdlog::debug(__func__)
