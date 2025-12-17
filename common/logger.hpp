#pragma once

#include <syslog.h>
#include <string>

class Logger {
public:
    static void init() {
        openlog("CarDeliveryServer", LOG_PID | LOG_CONS, LOG_USER);
    }

    static void log_info(const std::string& message) {
        syslog(LOG_INFO, "%s", message.c_str());
    }

    static void log_error(const std::string& message) {
        syslog(LOG_ERR, "%s", message.c_str());
    }

    static void log_warning(const std::string& message) {
        syslog(LOG_WARNING, "%s", message.c_str());
    }

    static void log_debug(const std::string& message) {
        syslog(LOG_DEBUG, "%s", message.c_str());
    }

    static void cleanup() {
        closelog();
    }
};