#pragma once

#include "cyclone/mailer/base_mailer.hpp"
#include <string>
#include <map>

/**
 * Base class for all application mailers
 * Inherits from Cyclone's BaseMailer
 */
class ApplicationMailer : public cyclone::BaseMailer {
public:
    ApplicationMailer() {
        // Set default configuration for all mailers in the application
        setLayout("application");
        setDefaultFrom("notifications@example.com");

        // Set default data available to all email templates
        setDefaultData("site_name", "Your Application");
        setDefaultData("site_url", "https://example.com");
        setDefaultData("support_email", "support@example.com");

        // Configure asset paths for email templates
        setAssetHost("https://cdn.example.com");

        // Configure default email headers
        addDefaultHeader("X-Priority", "3");
        addDefaultHeader("X-Mailer", "Cyclone Mailer");
    }

protected:
    /**
     * Helper method to format URLs for email templates
     */
    std::string url(const std::string& path) const {
        return getDefaultData("site_url") + path;
    }

    /**
     * Helper method to truncate text for email summaries
     */
    std::string truncate(const std::string& text, size_t length = 100) const {
        if (text.length() <= length) {
            return text;
        }

        return text.substr(0, length - 3) + "...";
    }
};