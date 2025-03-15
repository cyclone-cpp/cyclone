#pragma once

#include "cyclone/job.hpp"
#include "../models/user.hpp"

class ApplicationJob : public Cyclone::Job {
public:
    // Common job configuration
    bool shouldRetry(const std::exception& e) const override {
        // Don't retry if it's a permanent error
        return !isPermanentError(e);
    }

    int maxRetries() const override {
        return 3; // Default max retries for all jobs
    }

    std::chrono::seconds retryDelay(int attempt) const override {
        // Exponential backoff: 1 minute, 5 minutes, 25 minutes
        return std::chrono::seconds(60 * std::pow(5, attempt - 1));
    }

    // Helper methods for all jobs
    std::string truncate(const std::string& str, size_t length) const {
        if (str.length() <= length) {
            return str;
        }

        return str.substr(0, length - 3) + "...";
    }

protected:
    bool isPermanentError(const std::exception& e) const {
        // Check if the exception message contains indicators of permanent failure
        std::string message = e.what();
        return message.find("not found") != std::string::npos ||
               message.find("invalid id") != std::string::npos ||
               message.find("unauthorized") != std::string::npos ||
               message.find("permission denied") != std::string::npos;
    }
};