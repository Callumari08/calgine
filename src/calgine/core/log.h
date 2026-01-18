#pragma once

#include <spdlog/logger.h>
#include "calgine_api.h"

namespace Calgine {

class CALGINE_API Log
{
public:
  static void init(std::string app_name);

  inline static std::shared_ptr<spdlog::logger>& get_engine_logger() { return engine_logger; }
  inline static std::shared_ptr<spdlog::logger>& get_app_logger() { return app_logger; }

private:
  static std::shared_ptr<spdlog::logger> engine_logger;
  static std::shared_ptr<spdlog::logger> app_logger;
};

} // namespace Calgine