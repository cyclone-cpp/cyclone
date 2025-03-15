#pragma once

#include "application_mailer.hpp"
#include "../models/user.hpp"
#include "../models/post.hpp"
#include "../models/comment.hpp"

/**
 * Mailer for sending user notifications
 * Inherits from ApplicationMailer
 */
class NotificationMailer : public ApplicationMailer {
public:
  NotificationMailer() {
    // Override default settings from ApplicationMailer
    setLayout("notification");
    setDefaultFrom("notifications@example.com", "Notification Center");
  }

  /**
   * Send notification about a new comment
   */
  void newComment(const User& user, const User& commenter, const Post& post, const Comment& comment) {
    // Set mail recipients
    to(user.email(), user.name());

    // Set mail subject
    subject("New comment on your post");

    // Set template variables
    setData("user_name", user.name());
    setData("commenter_name", commenter.name());
    setData("post_title", post.title());
    setData("comment_excerpt", truncate(comment.content(), 100));
    setData("post_url", url("/posts/" + std::to_string(post.id())));

    // Set unique template variables
    setData("comment_id", std::to_string(comment.id()));

    // Send using the template
    template_("notifications/new_comment");
  }

  /**
   * Send notification about a new like
   */
  void newLike(const User& user, const User& liker, const std::string& contentType,
               const std::string& contentExcerpt, const std::string& url) {
    to(user.email(), user.name());
    subject("Someone liked your " + contentType);

    setData("user_name", user.name());
    setData("liker_name", liker.name());
    setData("content_type", contentType);
    setData("content_excerpt", contentExcerpt);
    setData("url", url);

    template_("notifications/new_like");
  }

  /**
   * Send notification about being mentioned in a comment
   */
  void mentioned(const User& user, const User& mentioner, const Post& post, const Comment& comment) {
    to(user.email(), user.name());
    subject("You were mentioned in a comment");

    setData("user_name", user.name());
    setData("mentioner_name", mentioner.name());
    setData("post_title", post.title());
    setData("comment_excerpt", truncate(comment.content(), 100));
    setData("post_url", url("/posts/" + std::to_string(post.id()) +
                            "#comment-" + std::to_string(comment.id())));

    template_("notifications/mentioned");
  }

  /**
   * Send notification about post being published
   */
  void postPublished(const User& user, const Post& post) {
    to(user.email(), user.name());
    subject("Your post has been published!");

    setData("user_name", user.name());
    setData("post_title", post.title());
    setData("post_excerpt", truncate(post.content(), 150));
    setData("post_url", url("/posts/" + std::to_string(post.id())));

    template_("notifications/post_published");
  }
};