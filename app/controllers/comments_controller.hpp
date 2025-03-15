#pragma once

#include "application_controller.hpp"
#include "../models/post.hpp"
#include "../models/comment.hpp"
#include "../jobs/notification_job.hpp"

class CommentsController : public ApplicationController {
public:
  // POST /posts/:post_id/comments
  Cyclone::Response create(int postId) {
    requireLogin();

    auto post = Post::find(postId);

    if (!post) {
      flash().alert = "Post not found";
      return redirectTo("/posts");
    }

    auto params = request().params.get("comment");

    // Create the comment
    auto comment = Comment::new_(params);
    comment.setUserId(currentUser()->id());
    comment.setPostId(postId);

    if (comment.save()) {
      // Extract mentions from comment content
      auto mentions = extractMentions(comment.content());

      // Send notification to post author if they didn't write the comment
      if (post->userId() != currentUser()->id()) {
        JobQueue::enqueue<NotificationJob>({
          .type = NotificationJob::Type::NewComment,
          .user_id = post->userId(),
          .source_user_id = currentUser()->id(),
          .post_id = postId,
          .comment_id = comment.id()
        });
      }

      // Send notifications for mentions
      for (const auto& mentionedUser : mentions) {
        // Don't notify the comment author or post author (they already get a notification)
        if (mentionedUser.id() != currentUser()->id() && mentionedUser.id() != post->userId()) {
          JobQueue::enqueue<NotificationJob>({
            .type = NotificationJob::Type::MentionedInComment,
            .user_id = mentionedUser.id(),
            .source_user_id = currentUser()->id(),
            .post_id = postId,
            .comment_id = comment.id()
          });
        }
      }

      flash().notice = "Comment was successfully created";

      // Check if this is an AJAX request
      if (request().isAjax()) {
        return jsonResponse({
          {"id", comment.id()},
          {"content", comment.content()},
          {"user", {
            {"id", currentUser()->id()},
            {"name", currentUser()->name()}
          }},
          {"created_at", comment.createdAt().toString()},
          {"html", renderPartial("comments/_comment", {{"comment", comment}})}
        });
      } else {
        return redirectTo("/posts/" + std::to_string(postId) + "#comment-" + std::to_string(comment.id()));
      }
    } else {
      flash().alert = "Failed to create comment";

      if (request().isAjax()) {
        return jsonResponse({{"errors", comment.errors()}}, 422);
      } else {
        return redirectTo("/posts/" + std::to_string(postId));
      }
    }
  }

  // PATCH/PUT /posts/:post_id/comments/:id
  Cyclone::Response update(int postId, int id) {
    requireLogin();

    auto post = Post::find(postId);

    if (!post) {
      return resourceNotFound("Post not found");
    }

    auto comment = Comment::find(id);

    if (!comment) {
      return resourceNotFound("Comment not found");
    }

    // Check if the current user is the comment author or an admin
    if (comment->userId() != currentUser()->id() && !currentUser()->isAdmin()) {
      flash().alert = "You are not authorized to update this comment";
      return redirectTo("/posts/" + std::to_string(postId));
    }

    auto params = request().params.get("comment");

    // Get the old content for mention comparison
    auto oldContent = comment->content();

    if (comment->update(params)) {
      // Find new mentions that weren't in the original comment
      auto oldMentions = extractMentions(oldContent);
      auto newMentions = extractMentions(comment->content());

      // Send notifications for new mentions
      for (const auto& mentionedUser : newMentions) {
        // Check if this user wasn't mentioned before
        if (std::find_if(oldMentions.begin(), oldMentions.end(),
            [&mentionedUser](const User& u) { return u.id() == mentionedUser.id(); }) == oldMentions.end()) {
          // Don't notify the comment author
          if (mentionedUser.id() != currentUser()->id()) {
            JobQueue::enqueue<NotificationJob>({
              .type = NotificationJob::Type::MentionedInComment,
              .user_id = mentionedUser.id(),
              .source_user_id = currentUser()->id(),
              .post_id = postId,
              .comment_id = id
            });
          }
        }
      }

      flash().notice = "Comment was successfully updated";

      if (request().isAjax()) {
        return jsonResponse({
          {"id", comment->id()},
          {"content", comment->content()},
          {"updated_at", comment->updatedAt().toString()},
          {"html", renderPartial("comments/_comment", {{"comment", *comment}})}
        });
      } else {
        return redirectTo("/posts/" + std::to_string(postId) + "#comment-" + std::to_string(id));
      }
    } else {
      flash().alert = "Failed to update comment";

      if (request().isAjax()) {
        return jsonResponse({{"errors", comment->errors()}}, 422);
      } else {
        return redirectTo("/posts/" + std::to_string(postId) + "#comment-" + std::to_string(id));
      }
    }
  }

  // DELETE /posts/:post_id/comments/:id
  Cyclone::Response destroy(int postId, int id) {
    requireLogin();

    auto post = Post::find(postId);

    if (!post) {
      return resourceNotFound("Post not found");
    }

    auto comment = Comment::find(id);

    if (!comment) {
      return resourceNotFound("Comment not found");
    }

    // Check if the current user is the comment author or an admin
    if (comment->userId() != currentUser()->id() && !currentUser()->isAdmin()) {
      flash().alert = "You are not authorized to delete this comment";
      return redirectTo("/posts/" + std::to_string(postId));
    }

    comment->destroy();

    flash().notice = "Comment was successfully deleted";

    if (request().isAjax()) {
      return jsonResponse({
        {"id", id},
        {"message", "Comment was successfully deleted"}
      });
    } else {
      return redirectTo("/posts/" + std::to_string(postId));
    }
  }

private:
  Cyclone::Response resourceNotFound(const std::string& message) {
    flash().alert = message;

    if (request().isAjax()) {
      return jsonResponse({{"error", message}}, 404);
    } else {
      return redirectTo("/posts", 302);
    }
  }

  Cyclone::Response jsonResponse(const Cyclone::Json& data, int status = 200) {
    auto response = Cyclone::Response::json(data, status);
    response.headers["Content-Type"] = "application/json";
    return response;
  }

  // Extract mentions from comment content (usernames starting with @)
  std::vector<User> extractMentions(const std::string& content) {
    std::vector<User> mentions;
    std::regex mentionRegex("@([a-zA-Z0-9_]+)");

    std::sregex_iterator it(content.begin(), content.end(), mentionRegex);
    std::sregex_iterator end;

    std::unordered_set<std::string> processedUsernames;

    while (it != end) {
      std::string username = it->str(1);

      // Skip if we've already processed this username
      if (processedUsernames.find(username) != processedUsernames.end()) {
        ++it;
        continue;
      }

      processedUsernames.insert(username);

      // Find user by username
      auto user = User::findBy("name", username);
      if (user) {
        mentions.push_back(*user);
      }

      ++it;
    }

    return mentions;
  }
};