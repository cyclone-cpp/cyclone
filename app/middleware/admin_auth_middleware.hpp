#pragma once

#include "cyclone/middleware.hpp"
#include "../models/user.hpp"

class AdminAuthMiddleware : public Cyclone::Middleware {
public:
    Cyclone::Response process(const Cyclone::Request& request, Cyclone::MiddlewareNext next) override {
        // Check if user is logged in
        if (!request.session.has("user_id")) {
            // User is not logged in, redirect to login page
            Cyclone::Response response = Cyclone::Response::redirect("/login");
            response.flash["alert"] = "Please log in to access the admin area";
            return response;
        }

        // Check if user is an admin
        auto userId = request.session.get<int>("user_id");
        auto user = User::find(userId);

        if (!user || !user->isAdmin()) {
            // User is not an admin, redirect to homepage
            Cyclone::Response response = Cyclone::Response::redirect("/");
            response.flash["alert"] = "You do not have permission to access the admin area";
            return response;
        }

        // User is an admin, continue to the next middleware or controller
        return next(request);
    }
};