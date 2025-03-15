#pragma once

#include "cyclone/routing.hpp"
#include "../app/controllers/pages_controller.hpp"
#include "../app/controllers/posts_controller.hpp"
#include "../app/controllers/comments_controller.hpp"
#include "../app/controllers/likes_controller.hpp"
#include "../app/controllers/users_controller.hpp"
#include "../app/controllers/admin/dashboard_controller.hpp"
#include "../app/controllers/admin/posts_controller.hpp"
#include "../app/controllers/admin/users_controller.hpp"
#include "../app/middleware/admin_auth_middleware.hpp"

using namespace Cyclone;

void configureRoutes(Router& router) {
  // Home page
  router.get("/", &PagesController::home);

  // Static pages
  router.get("/about", &PagesController::about);
  router.get("/privacy", &PagesController::privacy);
  router.get("/terms", &PagesController::terms);
  router.get("/contact", &PagesController::contact);

  // Fortress authentication routes are automatically mounted at the root path
  // by the Fortress engine in the application configuration

  // Authenticated routes
  router.authenticate([](Router& r) {
    // Posts CRUD (only for logged-in users)
    r.resources("posts", PostsController, [](ResourceRouter& r) {
      // Comments (nested under posts)
      r.resources("comments", CommentsController);

      // Likes for posts
      r.post("/:id/like", &LikesController::create);
      r.delete("/:id/like", &LikesController::destroy);
    });

    // Comments standalone routes (for AJAX operations)
    r.post("/comments/:id/like", &LikesController::createForComment);
    r.delete("/comments/:id/like", &LikesController::destroyForComment);

    // User profile
    r.get("/profile", &UsersController::profile);
    r.put("/profile", &UsersController::updateProfile);
  });

  // Admin routes
  router.namespace("admin", [](Router& r) {
    // Admin authentication
    r.use(AdminAuthMiddleware);

    // Admin dashboard
    r.get("/", &Admin::DashboardController::index);

    // Admin CRUD for posts
    r.resources("posts", Admin::PostsController);

    // Admin CRUD for users
    r.resources("users", Admin::UsersController);

    // Admin CRUD for comments
    r.resources("comments", Admin::CommentsController);
  });

  // API endpoints
  router.namespace("api", [](Router& r) {
    r.namespace("v1", [](Router& r) {
      r.post("/token", &Api::V1::TokensController::create);

      // Authenticated API endpoints
      r.authenticate({.type = "token"}, [](Router& r) {
        r.resources("posts", Api::V1::PostsController);
        r.resources("comments", Api::V1::CommentsController);
        r.get("/users/me", &Api::V1::UsersController::currentUser);
      });
    });
  });

  // Error pages
  router.notFound(&ErrorsController::notFound);
  router.internalError(&ErrorsController::internalError);
  router.forbidden(&ErrorsController::forbidden);
  router.unauthorized(&ErrorsController::unauthorized);
}