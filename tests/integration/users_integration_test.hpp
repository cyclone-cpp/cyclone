#pragma once

#include "test_framework.hpp"
#include "../factories/user_factory.hpp"
#include "../factories/post_factory.hpp"
#include "cyclone/testing/integration_test.hpp"

class UsersIntegrationTest : public cyclone::testing::IntegrationTest {
public:
  void SetUp() override {
    DatabaseCleaner::start();
  }

  void TearDown() override {
    DatabaseCleaner::clean();
  }

  void describe_profile_workflow() {
    describe("User profile workflow", [&]() {
      it("allows users to view and edit their profile", [&]() {
        // Arrange - Create a user
        auto user = UserFactory::create({
          {"name", "Integration User"},
          {"email", "integration@example.com"},
          {"password", "password123"}
        });

        // Create some posts for the user
        PostFactory::createManyForUser(user, 3);

        // Start a browser session
        auto session = startSession();

        // Log in
        session.visit("/login");
        session.fillIn("email", "integration@example.com");
        session.fillIn("password", "password123");
        session.click("Log In");

        // Should redirect to dashboard or home page
        expect(session.currentPath()).to_equal("/dashboard");

        // Navigate to profile
        session.visit("/profile");
        expect(session.pageTitle()).to_equal("My Profile");
        expect(session.hasContent("Integration User")).to_be_true();

        // Post titles should be visible
        expect(session.findAll(".post-title").size()).to_equal(3);

        // Click edit profile button
        session.click("Edit Profile");
        expect(session.currentPath()).to_equal("/profile/edit");

        // Update name
        session.fillIn("user[name]", "Updated Integration User");
        session.click("Update Profile");

        // Should redirect back to profile with updated info
        expect(session.currentPath()).to_equal("/profile");
        expect(session.hasFlash("notice")).to_be_true();
        expect(session.hasContent("Updated Integration User")).to_be_true();

        // Log out
        session.click("Log Out");
        expect(session.currentPath()).to_equal("/");
      });

      it("allows users to change their password", [&]() {
        // Arrange - Create a user
        auto user = UserFactory::create({
          {"name", "Password User"},
          {"email", "password@example.com"},
          {"password", "password123"}
        });

        // Start a browser session
        auto session = startSession();

        // Log in
        session.visit("/login");
        session.fillIn("email", "password@example.com");
        session.fillIn("password", "password123");
        session.click("Log In");

        // Navigate to edit profile
        session.visit("/profile/edit");

        // Update password with incorrect current password
        session.fillIn("user[password]", "newpassword456");
        session.fillIn("user[password_confirmation]", "newpassword456");
        session.fillIn("user[current_password]", "wrongpassword");
        session.click("Update Profile");

        // Should show error
        expect(session.hasFlash("alert")).to_be_true();
        expect(session.hasContent("Current password is incorrect")).to_be_true();

        // Try again with correct current password
        session.fillIn("user[password]", "newpassword456");
        session.fillIn("user[password_confirmation]", "newpassword456");
        session.fillIn("user[current_password]", "password123");
        session.click("Update Profile");

        // Should redirect to profile with success message
        expect(session.currentPath()).to_equal("/profile");
        expect(session.hasFlash("notice")).to_be_true();

        // Log out
        session.click("Log Out");

        // Log in with new password
        session.visit("/login");
        session.fillIn("email", "password@example.com");
        session.fillIn("password", "newpassword456");
        session.click("Log In");

        // Should be logged in successfully
        expect(session.currentPath()).to_equal("/dashboard");
      });
    });
  }

  void describe_public_profile() {
    describe("Public user profile", [&]() {
      it("shows only published posts on public profile", [&]() {
        // Arrange - Create a user
        auto user = UserFactory::create({
          {"name", "Public Profile User"}
        });

        // Create published posts
        auto publishedPosts = PostFactory::createMany(2, {
          {"user_id", user.id()},
          {"published", true},
          {"title", "Published Post"}
        });

        // Create unpublished posts
        auto unpublishedPosts = PostFactory::createMany(2, {
          {"user_id", user.id()},
          {"published", false},
          {"title", "Unpublished Post"}
        });

        // Start a browser session
        auto session = startSession();

        // Visit public profile
        session.visit("/users/" + std::to_string(user.id()));

        // Check page content
        expect(session.pageTitle()).to_equal("Public Profile User's Profile");
        expect(session.hasContent("Public Profile User")).to_be_true();

        // Should show published posts
        expect(session.hasContent("Published Post")).to_be_true();

        // Should not show unpublished posts
        expect(session.hasContent("Unpublished Post")).to_be_false();

        // Count post elements
        expect(session.findAll(".post-item").size()).to_equal(2);
      });

      it("handles non-existent user profile", [&]() {
        // Start a browser session
        auto session = startSession();

        // Visit non-existent user profile
        session.visit("/users/99999");

        // Should redirect to home
        expect(session.currentPath()).to_equal("/");
        expect(session.hasFlash("alert")).to_be_true();
        expect(session.hasContent("User not found")).to_be_true();
      });
    });
  }

  void run_tests() override {
    describe_profile_workflow();
    describe_public_profile();
  }
};

// Register the test case with the test runner
REGISTER_TEST_CASE(UsersIntegrationTest);