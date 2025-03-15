#pragma once

#include "test_framework.hpp"
#include "../../app/jobs/notification_job.hpp"
#include "../../app/services/mailer_service_mock.hpp"
#include <memory>

class NotificationJobTest : public TestCase {
public:
  void SetUp() override {
    // Create mock objects and inject them into the service container
    mailerMock = std::make_shared<MailerServiceMock>();
    ServiceContainer::registerService<MailerService>(mailerMock);

    // Create test users
    testUser = createTestUser("recipient@example.com", "Test User");
    sourceUser = createTestUser("source@example.com", "Source User");

    // Create test post and comment
    testPost = createTestPost(sourceUser->id(), "Test Post Title", "Test post content");
    testComment = createTestComment(testPost->id(), sourceUser->id(), "This is a test comment mentioning @TestUser");
  }

  void TearDown() override {
    // Clean up test data
    Comment::deleteAll("id = ?", {testComment->id()});
    Post::deleteAll("id = ?", {testPost->id()});
    User::deleteAll("id IN (?, ?)", {testUser->id(), sourceUser->id()});

    // Reset service container
    ServiceContainer::reset();
  }

  void describe_new_comment_notification() {
    describe("when sending a new comment notification", [&]() {
      it("sends an email with correct template and data", [&]() {
        // Arrange
        NotificationJob::Params params;
        params.type = NotificationJob::Type::NewComment;
        params.user_id = testUser->id();
        params.source_user_id = sourceUser->id();
        params.post_id = testPost->id();
        params.comment_id = testComment->id();

        // Act
        NotificationJob job(params);
        job.perform();

        // Assert
        expect(mailerMock->sentEmails.size()).to_equal(1);

        const auto& email = mailerMock->sentEmails[0];
        expect(email.recipient).to_equal(testUser->email());
        expect(email.subject).to_equal("New comment on your post");
        expect(email.templateName).to_equal("comment_notification");

        expect(email.templateData.at("user_name")).to_equal(testUser->name());
        expect(email.templateData.at("commenter_name")).to_equal(sourceUser->name());
        expect(email.templateData.at("post_title")).to_equal(testPost->title());
        expect(email.templateData.contains("comment_excerpt")).to_be_true();
        expect(email.templateData.at("post_url")).to_equal("/posts/" + std::to_string(testPost->id()));
      });

      it("logs error when post is not found", [&]() {
        // Arrange
        NotificationJob::Params params;
        params.type = NotificationJob::Type::NewComment;
        params.user_id = testUser->id();
        params.source_user_id = sourceUser->id();
        params.post_id = 99999; // Non-existent post ID
        params.comment_id = testComment->id();

        // Act & Assert
        expect_logs([&]() {
          NotificationJob job(params);
          job.perform();
        }).to_contain("NotificationJob: Post not found");

        expect(mailerMock->sentEmails.size()).to_equal(0);
      });
    });
  }

  void describe_new_like_notification() {
    describe("when sending a new like notification for a post", [&]() {
      it("sends an email with correct template and data", [&]() {
        // Arrange
        NotificationJob::Params params;
        params.type = NotificationJob::Type::NewLike;
        params.user_id = testUser->id();
        params.source_user_id = sourceUser->id();
        params.post_id = testPost->id();

        // Act
        NotificationJob job(params);
        job.perform();

        // Assert
        expect(mailerMock->sentEmails.size()).to_equal(1);

        const auto& email = mailerMock->sentEmails[0];
        expect(email.recipient).to_equal(testUser->email());
        expect(email.subject).to_equal("Someone liked your post");
        expect(email.templateName).to_equal("like_notification");

        expect(email.templateData.at("user_name")).to_equal(testUser->name());
        expect(email.templateData.at("liker_name")).to_equal(sourceUser->name());
        expect(email.templateData.at("content_type")).to_equal("post");
        expect(email.templateData.contains("content_excerpt")).to_be_true();
        expect(email.templateData.at("url")).to_equal("/posts/" + std::to_string(testPost->id()));
      });

      it("sends an email with correct data for a comment like", [&]() {
        // Arrange
        NotificationJob::Params params;
        params.type = NotificationJob::Type::NewLike;
        params.user_id = testUser->id();
        params.source_user_id = sourceUser->id();
        params.comment_id = testComment->id();

        // Act
        NotificationJob job(params);
        job.perform();

        // Assert
        expect(mailerMock->sentEmails.size()).to_equal(1);

        const auto& email = mailerMock->sentEmails[0];
        expect(email.subject).to_equal("Someone liked your comment");
        expect(email.templateData.at("content_type")).to_equal("comment");
        expect(email.templateData.at("url")).to_contain("#comment-" + std::to_string(testComment->id()));
      });
    });
  }

  void describe_mention_notification() {
    describe("when sending a mention notification", [&]() {
      it("sends an email with correct template and data", [&]() {
        // Arrange
        NotificationJob::Params params;
        params.type = NotificationJob::Type::MentionedInComment;
        params.user_id = testUser->id();
        params.source_user_id = sourceUser->id();
        params.comment_id = testComment->id();

        // Act
        NotificationJob job(params);
        job.perform();

        // Assert
        expect(mailerMock->sentEmails.size()).to_equal(1);

        const auto& email = mailerMock->sentEmails[0];
        expect(email.recipient).to_equal(testUser->email());
        expect(email.subject).to_equal("You were mentioned in a comment");
        expect(email.templateName).to_equal("mention_notification");

        expect(email.templateData.at("user_name")).to_equal(testUser->name());
        expect(email.templateData.at("mentioner_name")).to_equal(sourceUser->name());
        expect(email.templateData.at("post_title")).to_equal(testPost->title());
        expect(email.templateData.contains("comment_excerpt")).to_be_true();
        expect(email.templateData.at("post_url")).to_contain("#comment-" + std::to_string(testComment->id()));
      });
    });
  }

  void describe_post_published_notification() {
    describe("when sending a post published notification", [&]() {
      it("sends an email with correct template and data", [&]() {
        // Arrange
        NotificationJob::Params params;
        params.type = NotificationJob::Type::PostPublished;
        params.user_id = testUser->id();
        params.source_user_id = sourceUser->id();
        params.post_id = testPost->id();

        // Act
        NotificationJob job(params);
        job.perform();

        // Assert
        expect(mailerMock->sentEmails.size()).to_equal(1);

        const auto& email = mailerMock->sentEmails[0];
        expect(email.recipient).to_equal(testUser->email());
        expect(email.subject).to_equal("Your post has been published!");
        expect(email.templateName).to_equal("post_published_notification");

        expect(email.templateData.at("user_name")).to_equal(testUser->name());
        expect(email.templateData.at("post_title")).to_equal(testPost->title());
        expect(email.templateData.contains("post_excerpt")).to_be_true();
        expect(email.templateData.at("post_url")).to_equal("/posts/" + std::to_string(testPost->id()));
      });
    });
  }

  void describe_metrics_recording() {
    describe("when performing any notification job", [&]() {
      it("increments the appropriate metrics counter", [&]() {
        // Arrange
        auto metricsService = std::make_shared<MetricsServiceMock>();
        ServiceContainer::registerService<MetricsService>(metricsService);

        NotificationJob::Params params;
        params.type = NotificationJob::Type::NewComment;
        params.user_id = testUser->id();
        params.source_user_id = sourceUser->id();
        params.post_id = testPost->id();
        params.comment_id = testComment->id();

        // Act
        NotificationJob job(params);
        job.perform();

        // Assert
        expect(metricsService->incrementedCounters).to_contain("notifications.new_comment.sent");
      });
    });
  }

  void run_tests() override {
    describe_new_comment_notification();
    describe_new_like_notification();
    describe_mention_notification();
    describe_post_published_notification();
    describe_metrics_recording();
  }

private:
  std::shared_ptr<MailerServiceMock> mailerMock;
  std::shared_ptr<User> testUser;
  std::shared_ptr<User> sourceUser;
  std::shared_ptr<Post> testPost;
  std::shared_ptr<Comment> testComment;

  std::shared_ptr<User> createTestUser(const std::string& email, const std::string& name) {
    auto user = std::make_shared<User>();
    user->setEmail(email);
    user->setName(name);
    user->save();
    return user;
  }

  std::shared_ptr<Post> createTestPost(int userId, const std::string& title, const std::string& content) {
    auto post = std::make_shared<Post>();
    post->setUserId(userId);
    post->setTitle(title);
    post->setContent(content);
    post->save();
    return post;
  }

  std::shared_ptr<Comment> createTestComment(int postId, int userId, const std::string& content) {
    auto comment = std::make_shared<Comment>();
    comment->setPostId(postId);
    comment->setUserId(userId);
    comment->setContent(content);
    comment->save();
    return comment;
  }
};

// Register the test case with the test runner
REGISTER_TEST_CASE(NotificationJobTest);