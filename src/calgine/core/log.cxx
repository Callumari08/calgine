#include "log.h"

#include "calgine_pch.h"

#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Calgine {

std::shared_ptr<spdlog::logger> Log::engine_logger;
std::shared_ptr<spdlog::logger> Log::app_logger;

void Log::init(std::string app_name)
{
  std::transform(app_name.begin(), app_name.end(), app_name.begin(), ::toupper);

  spdlog::set_pattern("%^[%T] (%n) %v%$");
  engine_logger = spdlog::stdout_color_mt("CALGINE");
  engine_logger->set_level(spdlog::level::trace);

  app_logger = spdlog::stdout_color_mt(app_name);
  app_logger->set_level(spdlog::level::trace);
}

} // namespace Calgine