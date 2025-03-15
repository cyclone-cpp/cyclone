#pragma once

#include "application_job.hpp"
#include "../models/user.hpp"
#include "../models/post.hpp"
#include "../models/comment.hpp"

class NotificationJob : public ApplicationJob {
public:
  // Notification types
  enum class Type {
    NewComment,
    NewLike,
    MentionedInComment,
    PostPublished
  };

  // Serializable parameters
  struct Params {
    Type type;
    int user_id;         // Recipient user ID
    int source_user_id;  // User who triggered the notification
    int post_id = 0;     // Optional post ID
    int comment_id = 0;  // Optional comment ID

    // Serialization methods required by the job system
    template <typename Archive>
    void serialize(Archive& ar) {
      ar & static_cast<int>(type);
      ar & user_id;
      ar & source_user_id;
      ar & post_id;
      ar & comment_id;
    }
  };

  // Constructor accepts parameters
  explicit NotificationJob(const Params& params) : params_(params) {}

  // Define job behavior
  void perform() override {
    // Load user from database
    auto user = User::find(params_.user_id);
    if (!user) {
      Logger::error("NotificationJob: User not found, id={}", params_.user_id);
      return;
    }

    auto sourceUser = User::find(params_.source_user_id);
    if (!sourceUser) {
      Logger::error("NotificationJob: Source user not found, id={}", params_.source_user_id);
      return;
    }

    // Create notification based on type
    switch (params_.type) {
      case Type::NewComment:
        sendNewCommentNotification(*user, *sourceUser);
        break;
      case Type::NewLike:
        sendNewLikeNotification(*user, *sourceUser);
        break;
      case Type::MentionedInComment:
        sendMentionNotification(*user, *sourceUser);
        break;
      case Type::PostPublished:
        sendPostPublishedNotification(*user);
        break;
    }

    // Record metrics
    std::string typeStr;
    switch (params_.type) {
      case Type::NewComment: typeStr = "new_comment"; break;
      case Type::NewLike: typeStr = "new_like"; break;
      case Type::MentionedInComment: typeStr = "mentioned"; break;
      case Type::PostPublished: typeStr = "post_published"; break;
    }

    Metrics::increment("notifications." + typeStr + ".sent");
  }

  // Job system interface methods
  std::string queueName() const override { return "notifications"; }
  int priority() const override { return 5; }

private:
  Params params_;

  void sendNewCommentNotification(const User& user, const User& sourceUser) {
    if (params_.post_id == 0 || params_.comment_id == 0) {
      Logger::error("NotificationJob: Missing post_id or comment_id for NewComment notification");
      return;
    }

    auto post = Post::find(params_.post_id);
    if (!post) {
      Logger::error("NotificationJob: Post not found, id={}", params_.post_id);
      return;
    }

    auto comment = Comment::find(params_.comment_id);
    if (!comment) {
      Logger::error("NotificationJob: Comment not found, id={}", params_.comment_id);
      return;
    }

    // Send email notification
    auto mailer = ServiceContainer::resolve<MailerService>();
    mailer->sendTemplate(
      user.email(),
      "New comment on your post",
      "comment_notification",
      {
        {"user_name", user.name()},
        {"commenter_name", sourceUser.name()},
        {"post_title", post->title()},
        {"comment_excerpt", truncate(comment->content(), 100)},
        {"post_url", "/posts/" + std::to_string(post->id())}
      }
    );
  }

  void sendNewLikeNotification(const User& user, const User& sourceUser) {
    std::string contentType;
    std::string contentExcerpt;
    std::string url;

    if (params_.post_id > 0) {
      auto post = Post::find(params_.post_id);
      if (!post) {
        Logger::error("NotificationJob: Post not found, id={}", params_.post_id);
        return;
      }

      contentType = "post";
      contentExcerpt = truncate(post->title(), 100);
      url = "/posts/" + std::to_string(post->id());
    } else if (params_.comment_id > 0) {
      auto comment = Comment::find(params_.comment_id);
      if (!comment) {
        Logger::error("NotificationJob: Comment not found, id={}", params_.comment_id);
        return;
      }

      auto post = Post::find(comment->postId());
      if (!post) {
        Logger::error("NotificationJob: Post not found for comment, post_id={}", comment->postId());
        return;
      }

      contentType = "comment";
      contentExcerpt = truncate(comment->content(), 100);
      url = "/posts/" + std::to_string(post->id()) + "#comment-" + std::to_string(comment->id());
    } else {
      Logger::error("NotificationJob: Missing post_id or comment_id for NewLike notification");
      return;
    }

    // Send email notification
    auto mailer = ServiceContainer::resolve<MailerService>();
    mailer->sendTemplate(
      user.email(),
      "Someone liked your " + contentType,
      "like_notification",
      {
        {"user_name", user.name()},
        {"liker_name", sourceUser.name()},
        {"content_type", contentType},
        {"content_excerpt", contentExcerpt},
        {"url", url}
      }
    );
  }

  void sendMentionNotification(const User& user, const User& sourceUser) {
    if (params_.comment_id == 0) {
      Logger::error("NotificationJob: Missing comment_id for MentionedInComment notification");
      return;
    }

    auto comment = Comment::find(params_.comment_id);
    if (!comment) {
      Logger::error("NotificationJob: Comment not found, id={}", params_.comment_id);
      return;
    }

    auto post = Post::find(comment->postId());
    if (!post) {
      Logger::error("NotificationJob: Post not found for comment, post_id={}", comment->postId());
      return;
    }

    // Send email notification
    auto mailer = ServiceContainer::resolve<MailerService>();
    mailer->sendTemplate(
      user.email(),
      "You were mentioned in a comment",
      "mention_notification",
      {
        {"user_name", user.name()},
        {"mentioner_name", sourceUser.name()},
        {"post_title", post->title()},
        {"comment_excerpt", truncate(comment->content(), 100)},
        {"post_url", "/posts/" + std::to_string(post->id()) + "#comment-" + std::to_string(comment->id())}
      }
    );
  }

  void sendPostPublishedNotification(const User& user) {
    if (params_.post_id == 0) {
      Logger::error("NotificationJob: Missing post_id for PostPublished notification");
      return;
    }

    auto post = Post::find(params_.post_id);
    if (!post) {
      Logger::error("NotificationJob: Post not found, id={}", params_.post_id);
      return;
    }

    // Send email notification
    auto mailer = ServiceContainer::resolve<MailerService>();
    mailer->sendTemplate(
      user.email(),
      "Your post has been published!",
      "post_published_notification",
      {
        {"user_name", user.name()},
        {"post_title", post->title()},
        {"post_excerpt", truncate(post->content(), 150)},
        {"post_url", "/posts/" + std::to_string(post->id())}
      }
    );
  }
};