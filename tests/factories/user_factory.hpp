#pragma once

#include "../../app/models/user.hpp"
#include <string>
#include <map>
#include <optional>
#include <random>

/**
 * Factory for creating User instances for testing
 */
class UserFactory {
public:
  /**
   * Create a User with default or overridden attributes
   *
   * @param overrides Map of attributes to override the defaults
   * @return A persisted User instance
   */
  static User create(std::map<std::string, std::any> overrides = {}) {
    static int counter = 0;
    counter++;

    // Default attributes
    std::map<std::string, std::any> attributes = {
      {"email", "user" + std::to_string(counter) + "@example.com"},
      {"password", "password123"},
      {"password_confirmation", "password123"},
      {"name", "Test User " + std::to_string(counter)},
      {"role", "user"}
    };

    // Apply overrides
    for (const auto& [key, value] : overrides) {
      attributes[key] = value;
    }

    // Create and return the user
    auto user = User::create(attributes);

    // Confirm the user by default
    if (!overrides.contains("confirmed_at") && !std::any_cast<std::optional<TimePoint>>(attributes["confirmed_at"]).has_value()) {
      user.setConfirmedAt(TimePoint::now());
      user.save();
    }

    return user;
  }

  /**
   * Create an admin user
   *
   * @param overrides Map of attributes to override the defaults
   * @return A persisted admin User instance
   */
  static User createAdmin(std::map<std::string, std::any> overrides = {}) {
    // Ensure the role is set to admin
    overrides["role"] = "admin";

    return create(overrides);
  }

  /**
   * Create multiple Users with the same attributes
   *
   * @param count Number of users to create
   * @param attributes Attributes to apply to all users
   * @return Vector of persisted User instances
   */
  static std::vector<User> createMany(int count, std::map<std::string, std::any> attributes = {}) {
    std::vector<User> users;
    users.reserve(count);

    for (int i = 0; i < count; i++) {
      users.push_back(create(attributes));
    }

    return users;
  }

  /**
   * Build a User instance without persisting it to the database
   *
   * @param overrides Map of attributes to override the defaults
   * @return An unpersisted User instance
   */
  static User build(std::map<std::string, std::any> overrides = {}) {
    static int counter = 0;
    counter++;

    // Default attributes
    std::map<std::string, std::any> attributes = {
      {"email", "user" + std::to_string(counter) + "@example.com"},
      {"password", "password123"},
      {"password_confirmation", "password123"},
      {"name", "Test User " + std::to_string(counter)},
      {"role", "user"}
    };

    // Apply overrides
    for (const auto& [key, value] : overrides) {
      attributes[key] = value;
    }

    // Build and return the user (without saving)
    return User::new_(attributes);
  }

  /**
   * Generate a random valid email
   */
  static std::string randomEmail() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1000, 9999);

    return "user" + std::to_string(dis(gen)) + "@example.com";
  }
};