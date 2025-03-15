#pragma once

#include "cyclone/model.hpp"
#include "cyclone/engines/fortress/authenticatable.hpp"

class User : public Cyclone::Model<User> {
public:
  static void defineSchema() {
    schema()
      .field<int>("id", {.primaryKey = true, .autoIncrement = true})
      .field<std::string>("email", {.nullable = false, .unique = true})
      .field<std::string>("name", {.nullable = false})
      .field<std::string>("encrypted_password")
      .field<std::string>("reset_password_token")
      .field<TimePoint>("reset_password_sent_at")
      .field<std::string>("confirmation_token")
      .field<TimePoint>("confirmed_at")
      .field<TimePoint>("confirmation_sent_at")
      .field<std::string>("unconfirmed_email")
      .field<int>("failed_attempts", {.default_ = 0})
      .field<std::string>("unlock_token")
      .field<TimePoint>("locked_at")
      .field<std::string>("remember_token")
      .field<TimePoint>("remember_created_at")
      .field<std::string>("role", {.default_ = "user"})
      .timestamps();

    // Define relationships
    hasMany<Post>();
    hasMany<Comment>();
    hasMany<Like>();
  }

  // Validations
  static void defineValidations() {
    validates("name", {.presence = true, .length = {.minimum = 2, .maximum = 50}});
    validates("email", {.presence = true, .email = true, .uniqueness = true});
    validates("password", {.length = {.minimum = 8}, .confirmation = true});
  }

  // Fortress authentication DSL
  FORTRESS_AUTHENTICATABLE({
    .password_length = {8, 128},
    .email_regexp = "^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$",
    .stretches = 12,
    .pepper = "app_pepper_from_env_var"
  });

  FORTRESS_RECOVERABLE({
    .reset_password_within = std::chrono::hours(6)
  });

  FORTRESS_CONFIRMABLE({
    .reconfirmable = true,
    .confirmation_within = std::chrono::days(3)
  });

  FORTRESS_LOCKABLE({
    .maximum_attempts = 5,
    .unlock_strategy = Fortress::UnlockStrategy::Both,
    .unlock_in = std::chrono::hours(1)
  });

  FORTRESS_TRACKABLE();

  FORTRESS_REMEMBERABLE({
    .remember_for = std::chrono::days(14),
    .extend_remember_period = true
  });

  // Authorization helpers
  bool isAdmin() const {
    return role() == "admin";
  }
};