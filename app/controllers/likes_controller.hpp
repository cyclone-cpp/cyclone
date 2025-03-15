#pragma once

#include "application_controller.hpp"
#include "../models/post.hpp"
#include "../models/comment.hpp"
#include "../models/like.hpp"
#include "../jobs/notification_job.hpp"

class LikesController : public ApplicationController {
public:
  // POST /posts/:id/like
  // Creates a like for a post
  Cyclone::Response create(int id) {
    requireLogin();

    auto post = Post::find(id);

    if (!post) {
      return jsonResponse({{"error", "Post not found"}}, 404);
    }

    // Check if user already liked this post
    auto existingLike = Like::forPost(id).byUser(currentUser()->id()).first();

    if (existingLike) {
      return jsonResponse({{"error", "You have already liked this post"}}, 422);
    }

    // Create the like
    auto like = Like::new_({
      {"user_id", currentUser()->id()},
      {"likeable_id", id},
      {"likeable_type", "Post"}
    });

    if (like.save()) {
      // Enqueue notification job if the like is not from the post author
      if (post->userId() != currentUser()->id()) {
        JobQueue::enqueue<NotificationJob>({
          .type = NotificationJob::Type::NewLike,
          .user_id = post->userId(),
          .source_user_id = currentUser()->id(),
          .post_id = id
        });
      }

      // Return updated like count
      return jsonResponse({
        {"like_count", post->likeCount()},
        {"message", "Post liked successfully"}
      });
    } else {
      return jsonResponse({{"error", "Failed to like post"}}, 422);
    }
  }

  // DELETE /posts/:id/like
  // Removes a like from a post
  Cyclone::Response destroy(int id) {
    requireLogin();

    auto post = Post::find(id);

    if (!post) {
      return jsonResponse({{"error", "Post not found"}}, 404);
    }

    // Find the like
    auto like = Like::forPost(id).byUser(currentUser()->id()).first();

    if (!like) {
      return jsonResponse({{"error", "You have not liked this post"}}, 422);
    }

    // Remove the like
    like->destroy();

    // Return updated like count
    return jsonResponse({
      {"like_count", post->likeCount()},
      {"message", "Post unliked successfully"}
    });
  }

  // POST /comments/:id/like
  // Creates a like for a comment
  Cyclone::Response createForComment(int id) {
    requireLogin();

    auto comment = Comment::find(id);

    if (!comment) {
      return jsonResponse({{"error", "Comment not found"}}, 404);
    }

    // Check if user already liked this comment
    auto existingLike = Like::forComment(id).byUser(currentUser()->id()).first();

    if (existingLike) {
      return jsonResponse({{"error", "You have already liked this comment"}}, 422);
    }

    // Create the like
    auto like = Like::new_({
      {"user_id", currentUser()->id()},
      {"likeable_id", id},
      {"likeable_type", "Comment"}
    });

    if (like.save()) {
      // Enqueue notification job if the like is not from the comment author
      if (comment->userId() != currentUser()->id()) {
        JobQueue::enqueue<NotificationJob>({
          .type = NotificationJob::Type::NewLike,
          .user_id = comment->userId(),
          .source_user_id = currentUser()->id(),
          .comment_id = id
        });
      }

      // Return updated like count
      return jsonResponse({
        {"like_count", comment->likeCount()},
        {"message", "Comment liked successfully"}
      });
    } else {
      return jsonResponse({{"error", "Failed to like comment"}}, 422);
    }
  }

  // DELETE /comments/:id/like
  // Removes a like from a comment
  Cyclone::Response destroyForComment(int id) {
    requireLogin();

    auto comment = Comment::find(id);

    if (!comment) {
      return jsonResponse({{"error", "Comment not found"}}, 404);
    }

    // Find the like
    auto like = Like::forComment(id).byUser(currentUser()->id()).first();

    if (!like) {
      return jsonResponse({{"error", "You have not liked this comment"}}, 422);
    }

    // Remove the like
    like->destroy();

    // Return updated like count
    return jsonResponse({
      {"like_count", comment->likeCount()},
      {"message", "Comment unliked successfully"}
    });
  }

private:
  Cyclone::Response jsonResponse(const Cyclone::Json& data, int status = 200) {
    auto response = Cyclone::Response::json(data, status);

    // Set appropriate headers
    response.headers["Content-Type"] = "application/json";

    return response;
  }
};