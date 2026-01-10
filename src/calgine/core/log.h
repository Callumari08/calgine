#pragma once

#include <spdlog/logger.h>

class Log
{
public:
  static void init(std::string app_name);

  inline static std::shared_ptr<spdlog::logger>& get_engine_logger() { return S_engine_logger; }
  inline static std::shared_ptr<spdlog::logger>& get_app_logger() { return S_app_logger; }

private:
  static std::shared_ptr<spdlog::logger> S_engine_logger;
  static std::shared_ptr<spdlog::logger> S_app_logger;
};