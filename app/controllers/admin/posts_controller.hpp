#pragma once

#include "../application_controller.hpp"
#include "../../models/post.hpp"

namespace Admin {

class PostsController : public ApplicationController {
public:
  // GET /admin/posts
  Cyclone::Response index() {
    requireAdmin();

    // Get query parameters for filtering
    auto status = request().params.get<std::string>("status", "all");
    auto author = request().params.get<int>("author_id", 0);
    auto search = request().params.get<std::string>("search", "");
    auto page = request().params.get<int>("page", 1);
    auto perPage = request().params.get<int>("per_page", 20);

    // Build query
    auto query = Post::query();

    // Apply filters
    if (status == "published") {
      query = query.where("published", true);
    } else if (status == "draft") {
      query = query.where("published", false);
    }

    if (author > 0) {
      query = query.where("user_id", author);
    }

    if (!search.empty()) {
      query = query.where([search](auto& q) {
        q.where("title", "LIKE", "%" + search + "%")
         .orWhere("content", "LIKE", "%" + search + "%");
      });
    }

    // Order by newest first
    query = query.orderBy("created_at", "DESC");

    // Paginate results
    auto paginator = query.paginate(page, perPage);

    // Get authors for filter dropdown
    auto authors = User::join<Post>()
      .select("users.id, users.name")
      .groupBy("users.id")
      .orderBy("users.name", "ASC")
      .get();

    return render("admin/posts/index", {
      {"posts", paginator.items()},
      {"pagination", paginator},
      {"status", status},
      {"author_id", author},
      {"search", search},
      {"authors", authors}
    });
  }

  // GET /admin/posts/:id
  Cyclone::Response show(int id) {
    requireAdmin();

    auto post = Post::find(id);

    if (!post) {
      flash().alert = "Post not found";
      return redirectTo("/admin/posts");
    }

    return render("admin/posts/show", {
      {"post", *post}
    });
  }

  // GET /admin/posts/new
  Cyclone::Response newPost() {
    requireAdmin();

    auto post = Post::new_();
    auto users = User::orderBy("name", "ASC").get();

    return render("admin/posts/new", {
      {"post", post},
      {"users", users}
    });
  }

  // POST /admin/posts
  Cyclone::Response create() {
    requireAdmin();

    auto params = request().params.get("post");

    // Create the post
    auto post = Post::new_(params);

    if (post.save()) {
      flash().notice = "Post was successfully created";
      return redirectTo("/admin/posts/" + std::to_string(post.id()));
    } else {
      auto users = User::orderBy("name", "ASC").get();

      return render("admin/posts/new", {
        {"post", post},
        {"users", users},
        {"errors", post.errors()}
      }, 422);
    }
  }

  // GET /admin/posts/:id/edit
  Cyclone::Response edit(int id) {
    requireAdmin();

    auto post = Post::find(id);

    if (!post) {
      flash().alert = "Post not found";
      return redirectTo("/admin/posts");
    }

    auto users = User::orderBy("name", "ASC").get();

    return render("admin/posts/edit", {
      {"post", *post},
      {"users", users}
    });
  }

  // PUT/PATCH /admin/posts/:id
  Cyclone::Response update(int id) {
    requireAdmin();

    auto post = Post::find(id);

    if (!post) {
      flash().alert = "Post not found";
      return redirectTo("/admin/posts");
    }

    auto params = request().params.get("post");

    // Check if we're publishing a previously unpublished post
    bool wasPublished = post->published();

    if (post->update(params)) {
      // If the post was just published, send a notification
      if (!wasPublished && post->published()) {
        JobQueue::enqueue<NotificationJob>({
          .type = NotificationJob::Type::PostPublished,
          .user_id = post->userId(),
          .source_user_id = currentUser()->id(),
          .post_id = post->id()
        });
      }

      flash().notice = "Post was successfully updated";
      return redirectTo("/admin/posts/" + std::to_string(id));
    } else {
      auto users = User::orderBy("name", "ASC").get();

      return render("admin/posts/edit", {
        {"post", *post},
        {"users", users},
        {"errors", post->errors()}
      }, 422);
    }
  }

  // DELETE /admin/posts/:id
  Cyclone::Response destroy(int id) {
    requireAdmin();

    auto post = Post::find(id);

    if (!post) {
      flash().alert = "Post not found";
      return redirectTo("/admin/posts");
    }

    post->destroy();

    flash().notice = "Post was successfully deleted";
    return redirectTo("/admin/posts");
  }
};

} // namespace Admin