#pragma once

#include "../../app/models/post.hpp"
#include "user_factory.hpp"
#include <string>
#include <map>
#include <optional>
#include <random>
#include <vector>

/**
 * Factory for creating Post instances for testing
 */
class PostFactory {
public:
  /**
   * Create a Post with default or overridden attributes
   *
   * @param overrides Map of attributes to override the defaults
   * @return A persisted Post instance
   */
  static Post create(std::map<std::string, std::any> overrides = {}) {
    static int counter = 0;
    counter++;

    // Create a user if user_id not provided
    if (!overrides.contains("user_id")) {
      auto user = UserFactory::create();
      overrides["user_id"] = user.id();
    }

    // Default attributes
    std::map<std::string, std::any> attributes = {
      {"title", "Test Post " + std::to_string(counter)},
      {"content", generateLoremIpsum(3, 5)},
      {"published", false}
    };

    // Apply overrides
    for (const auto& [key, value] : overrides) {
      attributes[key] = value;
    }

    // Create and return the post
    return Post::create(attributes);
  }

  /**
   * Create a published Post
   *
   * @param overrides Map of attributes to override the defaults
   * @return A persisted published Post instance
   */
  static Post createPublished(std::map<std::string, std::any> overrides = {}) {
    // Ensure post is published
    overrides["published"] = true;

    // Set published_at time if not provided
    if (!overrides.contains("published_at")) {
      overrides["published_at"] = TimePoint::now() - std::chrono::hours(1);
    }

    return create(overrides);
  }

  /**
   * Create multiple Posts with the same attributes
   *
   * @param count Number of posts to create
   * @param attributes Attributes to apply to all posts
   * @return Vector of persisted Post instances
   */
  static std::vector<Post> createMany(int count, std::map<std::string, std::any> attributes = {}) {
    std::vector<Post> posts;
    posts.reserve(count);

    for (int i = 0; i < count; i++) {
      posts.push_back(create(attributes));
    }

    return posts;
  }

  /**
   * Create multiple Posts for the same user
   *
   * @param user The user who will own the posts
   * @param count Number of posts to create
   * @param attributes Additional attributes to apply
   * @return Vector of persisted Post instances
   */
  static std::vector<Post> createManyForUser(const User& user, int count, std::map<std::string, std::any> attributes = {}) {
    attributes["user_id"] = user.id();
    return createMany(count, attributes);
  }

  /**
   * Build a Post instance without persisting it to the database
   *
   * @param overrides Map of attributes to override the defaults
   * @return An unpersisted Post instance
   */
  static Post build(std::map<std::string, std::any> overrides = {}) {
    static int counter = 0;
    counter++;

    // Create a user if user_id not provided
    if (!overrides.contains("user_id")) {
      auto user = UserFactory::create();
      overrides["user_id"] = user.id();
    }

    // Default attributes
    std::map<std::string, std::any> attributes = {
      {"title", "Test Post " + std::to_string(counter)},
      {"content", generateLoremIpsum(3, 5)},
      {"published", false}
    };

    // Apply overrides
    for (const auto& [key, value] : overrides) {
      attributes[key] = value;
    }

    // Build and return the post (without saving)
    return Post::new_(attributes);
  }

private:
  /**
   * Generate lorem ipsum text for post content
   *
   * @param paragraphs Number of paragraphs to generate
   * @param sentencesPerParagraph Number of sentences per paragraph
   * @return Generated lorem ipsum text
   */
  static std::string generateLoremIpsum(int paragraphs, int sentencesPerParagraph) {
    static const std::vector<std::string> sentences = {
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit.",
      "Nullam auctor, nisl eget ultricies tincidunt, nisl nisl aliquam nisl, eget aliquam nisl nisl eget nisl.",
      "Fusce euismod, nisl eget ultricies tincidunt, nisl nisl aliquam nisl, eget aliquam nisl nisl eget nisl.",
      "Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.",
      "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.",
      "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.",
      "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.",
      "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium.",
      "Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit.",
      "Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit."
    };

    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::string result;

    for (int p = 0; p < paragraphs; p++) {
      for (int s = 0; s < sentencesPerParagraph; s++) {
        std::uniform_int_distribution<> dis(0, sentences.size() - 1);
        result += sentences[dis(gen)] + " ";
      }
      result += "\n\n";
    }

    return result;
  }
};