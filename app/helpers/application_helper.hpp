#pragma once

#include "cyclone/helper.hpp"
#include "../models/post.hpp"
#include "../models/comment.hpp"
#include "../models/like.hpp"

class ApplicationHelper : public Cyclone::Helper {
public:
  // Format a date with the specified format string
  std::string formatDate(const Cyclone::TimePoint& date, const std::string& format) {
    auto time = std::chrono::system_clock::to_time_t(date);
    std::tm tm = *std::localtime(&time);

    char buffer[100];
    std::strftime(buffer, sizeof(buffer), format.c_str(), &tm);

    return std::string(buffer);
  }

  // Get a human-readable time ago string
  std::string timeAgo(const Cyclone::TimePoint& date) {
    auto now = std::chrono::system_clock::now();
    auto diff = now - date;

    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(diff).count();
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(diff).count();
    auto hours = std::chrono::duration_cast<std::chrono::hours>(diff).count();
    auto days = std::chrono::duration_cast<std::chrono::days>(diff).count();

    if (seconds < 60) {
      return "just now";
    } else if (minutes < 60) {
      return std::to_string(minutes) + (minutes == 1 ? " minute ago" : " minutes ago");
    } else if (hours < 24) {
      return std::to_string(hours) + (hours == 1 ? " hour ago" : " hours ago");
    } else if (days < 7) {
      return std::to_string(days) + (days == 1 ? " day ago" : " days ago");
    } else {
      return formatDate(date, "%B %d, %Y");
    }
  }

  // Truncate a string to the specified length
  std::string truncate(const std::string& str, size_t length, const std::string& omission = "...") {
    if (str.length() <= length) {
      return str;
    }

    return str.substr(0, length - omission.length()) + omission;
  }

  // Convert markdown to HTML
  std::string markdown(const std::string& content) {
    // This would use a markdown library to convert the content to HTML
    // For simplicity, we'll just return the content as-is
    return content;
  }

  // Check if the current user has liked a post
  bool hasLiked(const Post& post) {
    if (!currentUser()) {
      return false;
    }

    return Like::forPost(post.id()).byUser(currentUser()->id()).exists();
  }

  // Check if the current user has liked a comment
  bool hasLikedComment(const Comment& comment) {
    if (!currentUser()) {
      return false;
    }

    return Like::forComment(comment.id()).byUser(currentUser()->id()).exists();
  }

  // Get the current user from the controller
  std::optional<User> currentUser() {
    return controller()->currentUser();
  }

  // Check if a user is logged in
  bool loggedIn() {
    return controller()->loggedIn();
  }
};