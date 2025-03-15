#pragma once

#include "../application_controller.hpp"
#include "../../models/user.hpp"
#include "../../models/post.hpp"
#include "../../models/comment.hpp"

namespace Admin {

    class DashboardController : public ApplicationController {
    public:
        // GET /admin
        Cyclone::Response index() {
            requireAdmin();

            // Get counts for dashboard
            auto userCount = User::count();
            auto postCount = Post::count();
            auto publishedPostCount = Post::published().count();
            auto commentCount = Comment::count();

            // Get recent activity
            auto recentUsers = User::orderBy("created_at", "DESC").limit(5).get();
            auto recentPosts = Post::orderBy("created_at", "DESC").limit(5).get();
            auto recentComments = Comment::orderBy("created_at", "DESC").limit(5).get();

            // Render dashboard view with stats
            return render("admin/dashboard/index", {
              {"user_count", userCount},
              {"post_count", postCount},
              {"published_post_count", publishedPostCount},
              {"comment_count", commentCount},
              {"recent_users", recentUsers},
              {"recent_posts", recentPosts},
              {"recent_comments", recentComments}
            });
        }
    };

} // namespace Admin