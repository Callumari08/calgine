#include "log.h"

#include "calgine_pch.h"

#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Calgine {

std::shared_ptr<spdlog::logger> Log::S_engine_logger;
std::shared_ptr<spdlog::logger> Log::S_app_logger;

void Log::init(std::string app_name)
{
  std::transform(app_name.begin(), app_name.end(), app_name.begin(), ::toupper);

  spdlog::set_pattern("%^[%T] (%n) %v%$");
  S_engine_logger = spdlog::stdout_color_mt("CALGINE");
  S_engine_logger->set_level(spdlog::level::trace);

  S_app_logger = spdlog::stdout_color_mt(app_name);
  S_app_logger->set_level(spdlog::level::trace);
}

} // namespace Calgine