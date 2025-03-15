#pragma once

#include "test_framework.hpp"
#include "../../app/models/post.hpp"
#include "../../app/models/user.hpp"
#include "../factories/user_factory.hpp"
#include "../factories/post_factory.hpp"
#include "../support/database_cleaner.hpp"

class PostTest : public TestCase {
public:
  void SetUp() override {
    // Clean database before each test
    DatabaseCleaner::start();
  }

  void TearDown() override {
    // Clean up after tests
    DatabaseCleaner::clean();
  }

  void describe_validations() {
    describe("validations", [&]() {
      it("validates presence of title", [&]() {
        auto user = UserFactory::create();
        auto post = Post::create({
          {"user_id", user.id()},
          {"title", ""},
          {"content", "Test content"}
        });

        expect(post.isValid()).to_be_false();
        expect(post.errors().on("title")).to_contain("can't be blank");
      });

      it("validates minimum length of title", [&]() {
        auto user = UserFactory::create();
        auto post = Post::create({
          {"user_id", user.id()},
          {"title", "AB"},  // Too short, minimum is 3
          {"content", "Test content"}
        });

        expect(post.isValid()).to_be_false();
        expect(post.errors().on("title")).to_contain("is too short (minimum is 3 characters)");
      });

      it("validates maximum length of title", [&]() {
        auto user = UserFactory::create();
        std::string longTitle(256, 'A');  // Too long, maximum is 255

        auto post = Post::create({
          {"user_id", user.id()},
          {"title", longTitle},
          {"content", "Test content"}
        });

        expect(post.isValid()).to_be_false();
        expect(post.errors().on("title")).to_contain("is too long (maximum is 255 characters)");
      });

      it("validates presence of content", [&]() {
        auto user = UserFactory::create();
        auto post = Post::create({
          {"user_id", user.id()},
          {"title", "Test Title"},
          {"content", ""}
        });

        expect(post.isValid()).to_be_false();
        expect(post.errors().on("content")).to_contain("can't be blank");
      });

      it("validates presence of user_id", [&]() {
        auto post = Post::create({
          {"title", "Test Title"},
          {"content", "Test content"}
        });

        expect(post.isValid()).to_be_false();
        expect(post.errors().on("user_id")).to_contain("can't be blank");
      });

      it("is valid with all required attributes", [&]() {
        auto user = UserFactory::create();
        auto post = Post::create({
          {"user_id", user.id()},
          {"title", "Test Title"},
          {"content", "Test content"}
        });

        expect(post.isValid()).to_be_true();
      });
    });
  }

  void describe_scopes() {
    describe("scopes", [&]() {
      it("finds published posts", [&]() {
        auto user = UserFactory::create();

        // Create unpublished post
        auto unpublishedPost = PostFactory::create({
          {"user_id", user.id()},
          {"published", false}
        });

        // Create published post
        auto publishedPost = PostFactory::create({
          {"user_id", user.id()},
          {"published", true},
          {"published_at", TimePoint::now() - std::chrono::hours(1)}  // Published 1 hour ago
        });

        // Create future published post
        auto futurePost = PostFactory::create({
          {"user_id", user.id()},
          {"published", true},
          {"published_at", TimePoint::now() + std::chrono::hours(1)}  // Will be published in 1 hour
        });

        auto posts = Post::published().get();

        expect(posts.size()).to_equal(1);
        expect(posts[0].id()).to_equal(publishedPost.id());
      });

      it("finds recent posts", [&]() {
        auto user = UserFactory::create();

        // Create 10 posts with different created_at times
        for (int i = 0; i < 10; i++) {
          PostFactory::create({
            {"user_id", user.id()},
            {"created_at", TimePoint::now() - std::chrono::hours(i)}
          });
        }

        auto recentPosts = Post::recent().get();

        expect(recentPosts.size()).to_equal(5);  // Should limit to 5

        // Posts should be ordered by created_at DESC
        for (size_t i = 1; i < recentPosts.size(); i++) {
          expect(recentPosts[i-1].created_at() > recentPosts[i].created_at()).to_be_true();
        }
      });
    });
  }

  void describe_callbacks() {
    describe("callbacks", [&]() {
      it("sets published_at when publishing a post", [&]() {
        auto user = UserFactory::create();
        auto post = PostFactory::create({
          {"user_id", user.id()},
          {"published", false},
          {"published_at", std::nullopt}
        });

        // Initially unpublished
        expect(post.published()).to_be_false();
        expect(post.published_at()).to_be_null();

        // Publish the post
        post.setPublished(true);
        post.save();

        // Should have set published_at
        expect(post.published()).to_be_true();
        expect(post.published_at()).not_to_be_null();
      });

      it("doesn't change published_at when already set", [&]() {
        auto user = UserFactory::create();
        auto originalTime = TimePoint::now() - std::chrono::hours(24);

        auto post = PostFactory::create({
          {"user_id", user.id()},
          {"published", true},
          {"published_at", originalTime}
        });

        // Update the post without changing published status
        post.setTitle("Updated Title");
        post.save();

        // published_at should remain unchanged
        expect(post.published_at()).to_equal(originalTime);
      });
    });
  }

  void describe_relationships() {
    describe("relationships", [&]() {
      it("belongs to a user", [&]() {
        auto user = UserFactory::create();
        auto post = PostFactory::create({
          {"user_id", user.id()}
        });

        auto relatedUser = post.user();
        expect(relatedUser.id()).to_equal(user.id());
      });

      it("has many comments", [&]() {
        auto post = PostFactory::create();

        // Create comments manually since we don't have a factory for them yet
        Comment::create({
          {"user_id", UserFactory::create().id()},
          {"post_id", post.id()},
          {"content", "Comment 1"}
        });

        Comment::create({
          {"user_id", UserFactory::create().id()},
          {"post_id", post.id()},
          {"content", "Comment 2"}
        });

        expect(post.comments().count()).to_equal(2);
      });

      it("has many likes", [&]() {
        auto post = PostFactory::create();

        // Create likes manually since we don't have a factory for them yet
        Like::create({
          {"user_id", UserFactory::create().id()},
          {"post_id", post.id()}
        });

        Like::create({
          {"user_id", UserFactory::create().id()},
          {"post_id", post.id()}
        });

        expect(post.likes().count()).to_equal(2);
      });
    });
  }

  void describe_methods() {
    describe("methods", [&]() {
      it("counts comments", [&]() {
        auto post = PostFactory::create();

        // Initially no comments
        expect(post.commentCount()).to_equal(0);

        // Add some comments
        Comment::create({
          {"user_id", UserFactory::create().id()},
          {"post_id", post.id()},
          {"content", "Comment 1"}
        });

        Comment::create({
          {"user_id", UserFactory::create().id()},
          {"post_id", post.id()},
          {"content", "Comment 2"}
        });

        expect(post.commentCount()).to_equal(2);
      });

      it("counts likes", [&]() {
        auto post = PostFactory::create();

        // Initially no likes
        expect(post.likeCount()).to_equal(0);

        // Add some likes
        Like::create({
          {"user_id", UserFactory::create().id()},
          {"post_id", post.id()}
        });

        Like::create({
          {"user_id", UserFactory::create().id()},
          {"post_id", post.id()}
        });

        expect(post.likeCount()).to_equal(2);
      });
    });
  }

  void run_tests() override {
    describe_validations();
    describe_scopes();
    describe_callbacks();
    describe_relationships();
    describe_methods();
  }
};

// Register the test case with the test runner
REGISTER_TEST_CASE(PostTest);