#pragma once

#include "application_controller.hpp"
#include "../models/post.hpp"

class PostsController : public ApplicationController {
public:
  // GET /posts
  Cyclone::Response index() {
    auto posts = Post::published().orderBy("created_at", "DESC").get();
    return render("posts/index", {{"posts", posts}});
  }

  // GET /posts/:id
  Cyclone::Response show(int id) {
    auto post = Post::find(id);

    if (!post) {
      return resourceNotFound("Post not found");
    }

    auto comments = post->comments()
      .orderBy("created_at", "ASC")
      .get();

    return render("posts/show", {
      {"post", *post},
      {"comments", comments}
    });
  }

  // GET /posts/new
  Cyclone::Response newPost() {
    requireLogin();

    auto post = Post::new_();
    return render("posts/new", {{"post", post}});
  }

  // POST /posts
  Cyclone::Response create() {
    requireLogin();

    auto params = request().params.get("post");

    // Create the post with the current user
    auto post = Post::new_(params);
    post.setUserId(currentUser()->id());

    if (post.save()) {
      flash().notice = "Post was successfully created";
      return redirectTo("/posts/" + std::to_string(post.id()));
    } else {
      return render("posts/new", {
        {"post", post},
        {"errors", post.errors()}
      }, 422);
    }
  }

  // GET /posts/:id/edit
  Cyclone::Response edit(int id) {
    requireLogin();

    auto post = Post::find(id);

    if (!post) {
      return resourceNotFound("Post not found");
    }

    // Check if the current user is the author
    if (post->userId() != currentUser()->id() && !currentUser()->isAdmin()) {
      flash().alert = "You are not authorized to edit this post";
      return redirectTo("/posts/" + std::to_string(id));
    }

    return render("posts/edit", {{"post", *post}});
  }

  // PUT/PATCH /posts/:id
  Cyclone::Response update(int id) {
    requireLogin();

    auto post = Post::find(id);

    if (!post) {
      return resourceNotFound("Post not found");
    }

    // Check if the current user is the author
    if (post->userId() != currentUser()->id() && !currentUser()->isAdmin()) {
      flash().alert = "You are not authorized to update this post";
      return redirectTo("/posts/" + std::to_string(id));
    }

    auto params = request().params.get("post");

    if (post->update(params)) {
      flash().notice = "Post was successfully updated";
      return redirectTo("/posts/" + std::to_string(id));
    } else {
      return render("posts/edit", {
        {"post", *post},
        {"errors", post->errors()}
      }, 422);
    }
  }

  // DELETE /posts/:id
  Cyclone::Response destroy(int id) {
    requireLogin();

    auto post = Post::find(id);

    if (!post) {
      return resourceNotFound("Post not found");
    }

    // Check if the current user is the author
    if (post->userId() != currentUser()->id() && !currentUser()->isAdmin()) {
      flash().alert = "You are not authorized to delete this post";
      return redirectTo("/posts/" + std::to_string(id));
    }

    post->destroy();

    flash().notice = "Post was successfully deleted";
    return redirectTo("/posts");
  }

private:
  Cyclone::Response resourceNotFound(const std::string& message) {
    flash().alert = message;
    return redirectTo("/posts", 302);
  }
};