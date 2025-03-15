#pragma once

#include "test_framework.hpp"
#include "../../app/controllers/users_controller.hpp"
#include "../factories/user_factory.hpp"
#include "../factories/post_factory.hpp"
#include "../factories/comment_factory.hpp"
#include "cyclone/testing/controller_test.hpp"

class UsersControllerTest : public cyclone::testing::ControllerTest {
public:
  void SetUp() override {
    DatabaseCleaner::start();
  }

  void TearDown() override {
    DatabaseCleaner::clean();
  }

  void describe_profile() {
    describe("GET /profile", [&]() {
      it("redirects to login when not authenticated", [&]() {
        // Act
        auto response = get("/profile");

        // Assert
        expect(response.status).to_equal(302);
        expect(response.redirectLocation()).to_contain("/login");
      });

      it("renders profile page for authenticated user", [&]() {
        // Arrange
        auto user = UserFactory::create();
        auto posts = PostFactory::createManyForUser(user, 3);
        auto comments = CommentFactory::createManyForUser(user, 5);

        // Act
        auto response = get("/profile", {}, loginAs(user));

        // Assert
        expect(response.status).to_equal(200);
        expect(response.body()).to_contain(user.name());
        expect(response.renderParams()["title"]).to_equal("My Profile");

        // Check that posts are included in render params
        auto renderedPosts = response.renderParams()["posts"].get<std::vector<Post>>();
        expect(renderedPosts.size()).to_equal(3);

        // Check that comments are included in render params
        auto renderedComments = response.renderParams()["comments"].get<std::vector<Comment>>();
        expect(renderedComments.size()).to_equal(5);
      });
    });
  }

  void describe_editProfile() {
    describe("GET /profile/edit", [&]() {
      it("redirects to login when not authenticated", [&]() {
        // Act
        auto response = get("/profile/edit");

        // Assert
        expect(response.status).to_equal(302);
        expect(response.redirectLocation()).to_contain("/login");
      });

      it("renders edit profile form for authenticated user", [&]() {
        // Arrange
        auto user = UserFactory::create();

        // Act
        auto response = get("/profile/edit", {}, loginAs(user));

        // Assert
        expect(response.status).to_equal(200);
        expect(response.body()).to_contain("Edit Profile");
        expect(response.body()).to_contain(user.name());
        expect(response.renderParams()["title"]).to_equal("Edit Profile");
      });
    });
  }

  void describe_updateProfile() {
    describe("PUT /profile", [&]() {
      it("redirects to login when not authenticated", [&]() {
        // Act
        auto response = put("/profile", {{"user", {{"name", "New Name"}}}});

        // Assert
        expect(response.status).to_equal(302);
        expect(response.redirectLocation()).to_contain("/login");
      });

      it("updates user name successfully", [&]() {
        // Arrange
        auto user = UserFactory::create();

        // Act
        auto response = put("/profile", {
          {"user", {
            {"name", "Updated Name"}
          }}
        }, loginAs(user));

        // Assert
        expect(response.status).to_equal(302);
        expect(response.redirectLocation()).to_equal("/profile");
        expect(response.flash()["notice"]).to_contain("Profile successfully updated");

        // Reload user from database and check name
        auto updatedUser = User::find(user.id());
        expect(updatedUser->name()).to_equal("Updated Name");
      });

      it("requires current password when changing password", [&]() {
        // Arrange
        auto user = UserFactory::create();

        // Act - try to update password without providing current password
        auto response = put("/profile", {
          {"user", {
            {"password", "newpassword123"},
            {"password_confirmation", "newpassword123"}
          }}
        }, loginAs(user));

        // Assert
        expect(response.status).to_equal(422);
        expect(response.flash()["alert"]).to_contain("Current password is incorrect");
      });

      it("updates password when current password is correct", [&]() {
        // Arrange
        auto user = UserFactory::create();
        std::string originalPassword = "password123"; // Default from factory

        // Act
        auto response = put("/profile", {
          {"user", {
            {"password", "newpassword123"},
            {"password_confirmation", "newpassword123"},
            {"current_password", originalPassword}
          }}
        }, loginAs(user));

        // Assert
        expect(response.status).to_equal(302);
        expect(response.redirectLocation()).to_equal("/profile");
        expect(response.flash()["notice"]).to_contain("Profile successfully updated");

        // Verify new password works
        auto updatedUser = User::find(user.id());
        bool validPassword = Fortress::Test::validatePassword(*updatedUser, "newpassword123");
        expect(validPassword).to_be_true();
      });

      it("rejects update with validation errors", [&]() {
        // Arrange
        auto user = UserFactory::create();

        // Act - empty name should fail validation
        auto response = put("/profile", {
          {"user", {
            {"name", ""}
          }}
        }, loginAs(user));

        // Assert
        expect(response.status).to_equal(422);
        expect(response.renderParams()["errors"].get<Cyclone::Errors>().on("name")).to_contain("can't be blank");
      });
    });
  }

  void describe_show() {
    describe("GET /users/:id", [&]() {
      it("shows public profile with published posts", [&]() {
        // Arrange
        auto user = UserFactory::create({{"name", "Public User"}});

        // Create published posts
        auto publishedPosts = PostFactory::createMany(2, {
          {"user_id", user.id()},
          {"published", true}
        });

        // Create unpublished posts
        auto unpublishedPosts = PostFactory::createMany(2, {
          {"user_id", user.id()},
          {"published", false}
        });

        // Act
        auto response = get("/users/" + std::to_string(user.id()));

        // Assert
        expect(response.status).to_equal(200);
        expect(response.body()).to_contain("Public User");
        expect(response.renderParams()["title"]).to_equal("Public User's Profile");

        // Should only include published posts
        auto renderedPosts = response.renderParams()["posts"].get<std::vector<Post>>();
        expect(renderedPosts.size()).to_equal(2);

        // Verify only published posts are shown
        for (const auto& post : renderedPosts) {
          expect(post.published()).to_be_true();
        }
      });

      it("redirects to home for non-existent user", [&]() {
        // Act
        auto response = get("/users/99999");

        // Assert
        expect(response.status).to_equal(302);
        expect(response.redirectLocation()).to_equal("/");
        expect(response.flash()["alert"]).to_contain("User not found");
      });
    });
  }

  void run_tests() override {
    describe_profile();
    describe_editProfile();
    describe_updateProfile();
    describe_show();
  }
};

// Register the test case with the test runner
REGISTER_TEST_CASE(UsersControllerTest);