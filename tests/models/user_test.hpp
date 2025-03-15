#pragma once

#include "test_framework.hpp"
#include "../../app/models/user.hpp"
#include "cyclone/fortress/test_helpers.hpp"

class UserTest : public TestCase {
public:
  void SetUp() override {
    // Clear database before each test
    User::deleteAll();
  }

  void TearDown() override {
    // Clean up after tests
    User::deleteAll();
  }

  void describe_validations() {
    describe("validations", [&]() {
      it("validates presence of name", [&]() {
        auto user = User::create({
          {"email", "test@example.com"},
          {"password", "password123"},
          {"password_confirmation", "password123"},
          {"name", ""}
        });

        expect(user.isValid()).to_be_false();
        expect(user.errors().on("name")).to_contain("can't be blank");
      });

      it("validates length of name", [&]() {
        // Test minimum length
        auto user1 = User::create({
          {"email", "test@example.com"},
          {"password", "password123"},
          {"password_confirmation", "password123"},
          {"name", "A"}
        });

        expect(user1.isValid()).to_be_false();
        expect(user1.errors().on("name")).to_contain("is too short (minimum is 2 characters)");

        // Test maximum length
        std::string longName(51, 'A');
        auto user2 = User::create({
          {"email", "test@example.com"},
          {"password", "password123"},
          {"password_confirmation", "password123"},
          {"name", longName}
        });

        expect(user2.isValid()).to_be_false();
        expect(user2.errors().on("name")).to_contain("is too long (maximum is 50 characters)");
      });

      it("validates email format", [&]() {
        auto user = User::create({
          {"email", "not_an_email"},
          {"password", "password123"},
          {"password_confirmation", "password123"},
          {"name", "Test User"}
        });

        expect(user.isValid()).to_be_false();
        expect(user.errors().on("email")).to_contain("is not a valid email");
      });

      it("validates uniqueness of email", [&]() {
        // Create first user
        auto user1 = User::create({
          {"email", "duplicate@example.com"},
          {"password", "password123"},
          {"password_confirmation", "password123"},
          {"name", "First User"}
        });

        expect(user1.isValid()).to_be_true();

        // Try to create another user with the same email
        auto user2 = User::create({
          {"email", "duplicate@example.com"},
          {"password", "password123"},
          {"password_confirmation", "password123"},
          {"name", "Second User"}
        });

        expect(user2.isValid()).to_be_false();
        expect(user2.errors().on("email")).to_contain("has already been taken");
      });

      it("validates password length", [&]() {
        auto user = User::create({
          {"email", "test@example.com"},
          {"password", "short"},
          {"password_confirmation", "short"},
          {"name", "Test User"}
        });

        expect(user.isValid()).to_be_false();
        expect(user.errors().on("password")).to_contain("is too short (minimum is 8 characters)");
      });

      it("validates password confirmation", [&]() {
        auto user = User::create({
          {"email", "test@example.com"},
          {"password", "password123"},
          {"password_confirmation", "different_password"},
          {"name", "Test User"}
        });

        expect(user.isValid()).to_be_false();
        expect(user.errors().on("password_confirmation")).to_contain("doesn't match Password");
      });
    });
  }

  void describe_authentication() {
    describe("authenticatable", [&]() {
      it("encrypts password on creation", [&]() {
        auto user = User::create({
          {"email", "test@example.com"},
          {"password", "password123"},
          {"password_confirmation", "password123"},
          {"name", "Test User"}
        });

        expect(user.isValid()).to_be_true();
        expect(user.encrypted_password()).not_to_be_empty();
        expect(user.encrypted_password()).not_to_equal("password123");
      });

      it("validates with correct password", [&]() {
        auto user = User::create({
          {"email", "test@example.com"},
          {"password", "password123"},
          {"password_confirmation", "password123"},
          {"name", "Test User"}
        });

        bool valid = Fortress::Test::validatePassword(user, "password123");
        expect(valid).to_be_true();
      });

      it("fails validation with incorrect password", [&]() {
        auto user = User::create({
          {"email", "test@example.com"},
          {"password", "password123"},
          {"password_confirmation", "password123"},
          {"name", "Test User"}
        });

        bool valid = Fortress::Test::validatePassword(user, "wrong_password");
        expect(valid).to_be_false();
      });
    });

    describe("recoverable", [&]() {
      it("can generate reset password token", [&]() {
        auto user = User::create({
          {"email", "test@example.com"},
          {"password", "password123"},
          {"password_confirmation", "password123"},
          {"name", "Test User"}
        });

        Fortress::Test::generateResetPasswordToken(user);

        expect(user.reset_password_token()).not_to_be_empty();
        expect(user.reset_password_sent_at()).not_to_be_null();
      });

      it("can reset password", [&]() {
        auto user = User::create({
          {"email", "test@example.com"},
          {"password", "password123"},
          {"password_confirmation", "password123"},
          {"name", "Test User"}
        });

        std::string token = Fortress::Test::generateResetPasswordToken(user);

        bool result = Fortress::Test::resetPassword(user, token, "new_password123", "new_password123");
        expect(result).to_be_true();

        // Verify new password works
        bool valid = Fortress::Test::validatePassword(user, "new_password123");
        expect(valid).to_be_true();
      });
    });

    describe("confirmable", [&]() {
      it("generates confirmation token on creation", [&]() {
        auto user = User::create({
          {"email", "test@example.com"},
          {"password", "password123"},
          {"password_confirmation", "password123"},
          {"name", "Test User"}
        });

        expect(user.confirmation_token()).not_to_be_empty();
        expect(user.confirmation_sent_at()).not_to_be_null();
        expect(user.confirmed_at()).to_be_null();
      });

      it("can be confirmed", [&]() {
        auto user = User::create({
          {"email", "test@example.com"},
          {"password", "password123"},
          {"password_confirmation", "password123"},
          {"name", "Test User"}
        });

        std::string token = user.confirmation_token();
        bool result = Fortress::Test::confirmUser(user, token);

        expect(result).to_be_true();
        expect(user.confirmed_at()).not_to_be_null();
      });
    });

    describe("lockable", [&]() {
      it("increments failed attempts on authentication failure", [&]() {
        auto user = User::create({
          {"email", "test@example.com"},
          {"password", "password123"},
          {"password_confirmation", "password123"},
          {"name", "Test User"}
        });

        Fortress::Test::failedLoginAttempt(user);
        expect(user.failed_attempts()).to_equal(1);
      });

      it("locks account after maximum attempts", [&]() {
        auto user = User::create({
          {"email", "test@example.com"},
          {"password", "password123"},
          {"password_confirmation", "password123"},
          {"name", "Test User"}
        });

        // Fail 5 times (the maximum)
        for (int i = 0; i < 5; i++) {
          Fortress::Test::failedLoginAttempt(user);
        }

        expect(user.locked_at()).not_to_be_null();
        expect(user.unlock_token()).not_to_be_empty();
      });

      it("can be unlocked with token", [&]() {
        auto user = User::create({
          {"email", "test@example.com"},
          {"password", "password123"},
          {"password_confirmation", "password123"},
          {"name", "Test User"}
        });

        // Lock the account
        for (int i = 0; i < 5; i++) {
          Fortress::Test::failedLoginAttempt(user);
        }

        std::string token = user.unlock_token();
        bool result = Fortress::Test::unlockAccount(user, token);

        expect(result).to_be_true();
        expect(user.locked_at()).to_be_null();
        expect(user.failed_attempts()).to_equal(0);
      });
    });
  }

  void describe_authorization() {
    describe("authorization", [&]() {
      it("recognizes admin role", [&]() {
        auto user = User::create({
          {"email", "admin@example.com"},
          {"password", "password123"},
          {"password_confirmation", "password123"},
          {"name", "Admin User"},
          {"role", "admin"}
        });

        expect(user.isAdmin()).to_be_true();
      });

      it("defaults to user role", [&]() {
        auto user = User::create({
          {"email", "user@example.com"},
          {"password", "password123"},
          {"password_confirmation", "password123"},
          {"name", "Regular User"}
        });

        expect(user.role()).to_equal("user");
        expect(user.isAdmin()).to_be_false();
      });
    });
  }

  void run_tests() override {
    describe_validations();
    describe_authentication();
    describe_authorization();
  }
};

// Register the test case with the test runner
REGISTER_TEST_CASE(UserTest);