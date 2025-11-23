//
// Created by Козельский Игорь on 06.11.2025.
//

#ifndef LOCALPM_LOGGER_H
#define LOCALPM_LOGGER_H

#include <memory>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string_view>

#ifndef CONSOLE_LOG
#define CONSOLE_LOG 0
#endif

namespace project::log {

// Вызывать один раз (например, в main)
inline void init(std::string_view app_name = "localpm",
				 std::string_view logdir = "logs/",
				 std::string_view logfile = "localpm.log",
				 spdlog::level::level_enum level = spdlog::level::info) {
	// консоль + ротирующий файл

	std::vector<spdlog::sink_ptr> sinks;

	auto rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
		std::string(logfile), 5 * 1024 * 1024, 3); // 5MB, 3 файла
	sinks.push_back(rotating);

	if (CONSOLE_LOG) {
		auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		sinks.push_back(console);
	}

	auto logger = std::make_shared<spdlog::logger>(std::string(app_name),
												   sinks.begin(), sinks.end());
	logger->set_level(level);
	logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");

	spdlog::set_default_logger(logger);
	spdlog::flush_on(spdlog::level::info);
}

// Удобные макросы (компилируются в ноль при более высоком SPDLOG_ACTIVE_LEVEL)
#define LOG_TRACE(...) SPDLOG_TRACE(__VA_ARGS__)
#define LOG_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#define LOG_INFO(...) SPDLOG_INFO(__VA_ARGS__)
#define LOG_WARN(...) SPDLOG_WARN(__VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)
#define LOG_CRIT(...) SPDLOG_CRITICAL(__VA_ARGS__)

} // namespace project::log
#endif // LOCALPM2_LOGGER_H
