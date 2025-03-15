#pragma once

#include "cyclone/services/mailer_service.hpp"
#include <vector>
#include <string>
#include <map>

/**
 * Mock implementation of MailerService for testing
 */
class MailerServiceMock : public MailerService {
public:
    struct SentEmail {
        std::string recipient;
        std::string subject;
        std::string body;
        std::string templateName;
        std::map<std::string, std::string> templateData;
    };

    std::vector<SentEmail> sentEmails;

    // Record email instead of sending it
    bool send(const std::string& to, const std::string& subject, const std::string& body) override {
        SentEmail email;
        email.recipient = to;
        email.subject = subject;
        email.body = body;

        sentEmails.push_back(email);
        return true;
    }

    // Record template email instead of sending it
    bool sendTemplate(
      const std::string& to,
      const std::string& subject,
      const std::string& templateName,
      const std::map<std::string, std::string>& templateData
    ) override {
        SentEmail email;
        email.recipient = to;
        email.subject = subject;
        email.templateName = templateName;
        email.templateData = templateData;

        sentEmails.push_back(email);
        return true;
    }

    // Reset recorded emails
    void reset() {
        sentEmails.clear();
    }
};