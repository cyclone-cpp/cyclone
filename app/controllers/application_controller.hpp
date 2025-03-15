#pragma once

#include "cyclone/controller.hpp"

class ApplicationController : public Cyclone::Controller {
protected:
    // Before action callbacks
    void beforeAction(const Cyclone::Action& action) override {
        // Set default layout
        setLayout("application");

        // Set common view variables
        setViewVar("app_name", "Cyclone App");
        setViewVar("current_year", std::to_string(currentYear()));

        // Set current user if authenticated
        if (auto user = currentUser()) {
            setViewVar("current_user", *user);
        }
    }

    // Helper methods for controllers
    std::optional<User> currentUser() {
        if (loggedIn()) {
            auto userId = session().get<int>("user_id");
            return User::find(userId);
        }
        return std::nullopt;
    }

    bool loggedIn() const {
        return session().has("user_id");
    }

    void requireLogin() {
        if (!loggedIn()) {
            flash().alert = "Please log in to access this page";
            redirectTo("/login");
        }
    }

    void requireAdmin() {
        if (auto user = currentUser()) {
            if (!user->isAdmin()) {
                flash().alert = "You are not authorized to access this page";
                redirectTo("/");
            }
        } else {
            requireLogin();
        }
    }

private:
    int currentYear() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto tm = std::localtime(&time);
        return tm->tm_year + 1900;
    }
};