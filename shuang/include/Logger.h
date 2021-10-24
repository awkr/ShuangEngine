#pragma once

#include <spdlog/spdlog.h>

#define logDebug(...) spdlog::debug(__VA_ARGS__);
#define logInfo(...) spdlog::info(__VA_ARGS__);
#define logWarn(...) spdlog::warn(__VA_ARGS__);
#define logError(...) spdlog::error(__VA_ARGS__);
