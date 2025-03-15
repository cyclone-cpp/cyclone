#pragma once

#include "application_controller.hpp"
#include "../models/user.hpp"
#include "../models/post.hpp"

class UsersController : public ApplicationController {
public:
  // GET /profile
  Cyclone::Response profile() {
    requireLogin();

    // Get user's posts
    auto posts = Post::where("user_id", currentUser()->id())
      .orderBy("created_at", "DESC")
      .get();

    // Get user's comments
    auto comments = Comment::where("user_id", currentUser()->id())
      .orderBy("created_at", "DESC")
      .limit(10)
      .get();

    return render("users/profile", {
      {"user", *currentUser()},
      {"posts", posts},
      {"comments", comments},
      {"title", "My Profile"}
    });
  }

  // GET /profile/edit
  Cyclone::Response editProfile() {
    requireLogin();

    return render("users/edit_profile", {
      {"user", *currentUser()},
      {"title", "Edit Profile"}
    });
  }

  // PUT /profile
  Cyclone::Response updateProfile() {
    requireLogin();

    auto params = request().params.get("user");
    auto user = currentUser();

    // Validate current password if password is being changed
    if (params.contains("password") && !params.get<std::string>("password").empty()) {
      if (!params.contains("current_password") ||
          !user->authenticate(params.get<std::string>("current_password"))) {
        flash().alert = "Current password is incorrect";

        return render("users/edit_profile", {
          {"user", *user},
          {"title", "Edit Profile"}
        }, 422);
      }
    }

    // Only allow updating certain fields
    Cyclone::Json filteredParams;

    if (params.contains("name")) {
      filteredParams["name"] = params.get<std::string>("name");
    }

    if (params.contains("password") && !params.get<std::string>("password").empty()) {
      filteredParams["password"] = params.get<std::string>("password");
      filteredParams["password_confirmation"] = params.get<std::string>("password_confirmation");
    }

    if (user->update(filteredParams)) {
      flash().notice = "Profile successfully updated";
      return redirectTo("/profile");
    } else {
      return render("users/edit_profile", {
        {"user", *user},
        {"errors", user->errors()},
        {"title", "Edit Profile"}
      }, 422);
    }
  }

  // GET /users/:id (public profile)
  Cyclone::Response show(int id) {
    auto user = User::find(id);

    if (!user) {
      flash().alert = "User not found";
      return redirectTo("/");
    }

    // Get user's public posts
    auto posts = Post::where("user_id", id)
      .where("published", true)
      .orderBy("created_at", "DESC")
      .get();

    return render("users/show", {
      {"user", *user},
      {"posts", posts},
      {"title", user->name() + "'s Profile"}
    });
  }
};