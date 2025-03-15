#pragma once

#include "application_controller.hpp"
#include "../models/post.hpp"

class PagesController : public ApplicationController {
public:
    // GET /
    Cyclone::Response home() {
        // Fetch recent posts for the homepage
        auto posts = Post::published()
          .orderBy("created_at", "DESC")
          .limit(5)
          .get();

        return render("pages/home", {
          {"posts", posts},
          {"title", "Welcome"}
        });
    }

    // GET /about
    Cyclone::Response about() {
        return render("pages/about", {
          {"title", "About Us"}
        });
    }

    // GET /privacy
    Cyclone::Response privacy() {
        return render("pages/privacy", {
          {"title", "Privacy Policy"}
        });
    }

    // GET /terms
    Cyclone::Response terms() {
        return render("pages/terms", {
          {"title", "Terms of Service"}
        });
    }

    // GET /contact
    Cyclone::Response contact() {
        return render("pages/contact", {
          {"title", "Contact Us"}
        });
    }
};