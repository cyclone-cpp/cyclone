# Cyclone Framework

A modern, Rails-inspired C++ web application framework.

## Overview

Cyclone is a high-performance web application framework for C++ that brings the developer-friendly patterns of Ruby on Rails to the C++ ecosystem. It combines the speed and efficiency of C++ with the rapid development features developers love from Rails.

## Features

- **MVC Architecture**: Clean separation of concerns with Models, Views, and Controllers
- **Active Record Pattern**: Intuitive database interaction through C++ models
- **Template Engine**: Fast template rendering with a Ruby-like syntax
- **Routing DSL**: Expressive route definition for your application endpoints
- **Background Jobs**: Asynchronous job processing with Pulse
- **Authentication System**: Built-in user authentication via Fortress
- **CLI Tools**: Powerful command-line tools for scaffolding and management

## Requirements

- C++17 compatible compiler (GCC 8+, Clang 7+, MSVC 19.14+)
- CMake 3.14+
- SQLite 3.28+ (for development, other databases supported in production)
- Mason build system

## Installation

1. Clone the Cyclone core repository:
   ```bash
   git clone https://github.com/cyclone/cyclone-cpp.git
   ```

2. Install dependencies:
   ```bash
   cd cyclone-cpp
   ./scripts/install_dependencies.sh
   ```

3. Build the framework:
   ```bash
   mason build
   ```

4. Create a new Cyclone application:
   ```bash
   bin/cy new my_awesome_app
   ```

## Quick Start

1. Navigate to your application directory:
   ```bash
   cd my_awesome_app
   ```

2. Generate a resource:
   ```bash
   bin/cy generate resource Post title:string content:text user:references
   ```

3. Run database migrations:
   ```bash
   bin/cy db:migrate
   ```

4. Start the server:
   ```bash
   bin/cy server
   ```

5. Visit http://localhost:3000 in your browser

## Project Structure

```
my_awesome_app/
├── app/                  # Application code
│   ├── controllers/      # Controller classes
│   ├── jobs/             # Background job classes
│   ├── mailers/          # Email generation classes
│   ├── models/           # Model classes
│   └── views/            # View templates
├── bin/                  # Executable scripts
│   └── cy                # Command-line utility
├── config/               # Configuration files
│   ├── routes.hpp        # Application routes
│   ├── database.json     # Database configuration
│   └── app_config.hpp    # General configuration
├── db/                   # Database files
│   └── migrations/       # Database migrations
├── public/               # Static files
├── tests/                # Test files
└── Mason.toml            # Build configuration
```

## Controllers

Controllers handle incoming HTTP requests and send back responses. Here's a simple example:

```cpp
// app/controllers/posts_controller.hpp
class PostsController : public ApplicationController {
public:
  // GET /posts
  Cyclone::Response index() {
    auto posts = Post::all().orderBy("created_at", "DESC").get();
    return render("posts/index", {{"posts", posts}});
  }

  // GET /posts/:id
  Cyclone::Response show(int id) {
    auto post = Post::find(id);
    if (!post) return redirectTo("/posts");
    return render("posts/show", {{"post", *post}});
  }
};
```

## Models

Models represent data and business logic. They interact with the database through an active record pattern:

```cpp
// app/models/post.hpp
class Post : public Cyclone::Model<Post> {
public:
  static void defineSchema() {
    schema()
      .field<int>("id", {.primaryKey = true, .autoIncrement = true})
      .field<int>("user_id", {.nullable = false})
      .field<std::string>("title", {.nullable = false})
      .field<std::string>("content", {.nullable = false})
      .timestamps();

    // Define relationships
    belongsTo<User>();
    hasMany<Comment>();
  }

  // Validations
  static void defineValidations() {
    validates("title", {.presence = true, .length = {.minimum = 3}});
    validates("content", {.presence = true});
  }
};
```

## Views

Views render HTML using a template engine with a syntax inspired by ERB:

```html
<!-- app/views/posts/show.cyc.html -->
<% setTitle(@post.title()) %>

<div class="post-container">
  <h1><%= @post.title() %></h1>
  <div class="post-meta">
    Posted by <%= @post.user().name() %> on <%= formatDate(@post.createdAt(), "%B %d, %Y") %>
  </div>
  <div class="post-content">
    <%= @post.content() %>
  </div>
</div>
```

## Testing

Cyclone provides a testing framework that makes it easy to test your application:

```cpp
// tests/models/post_test.hpp
class PostTest : public TestCase {
public:
  void describe_validations() {
    describe("validations", [&]() {
      it("validates presence of title", [&]() {
        auto post = Post::new_({
          {"user_id", 1},
          {"title", ""},
          {"content", "Test content"}
        });
        
        expect(post.isValid()).to_be_false();
        expect(post.errors().on("title")).to_contain("can't be blank");
      });
    });
  }
  
  void run_tests() override {
    describe_validations();
  }
};
```

## Background Jobs

Use background jobs for time-consuming tasks:

```cpp
// app/jobs/notification_job.hpp
class NotificationJob : public ApplicationJob {
public:
  explicit NotificationJob(int userId, int postId)
    : userId_(userId), postId_(postId) {}
  
  void perform() override {
    auto user = User::find(userId_);
    auto post = Post::find(postId_);
    
    if (user && post) {
      NotificationMailer().newPost(*user, *post);
    }
  }
  
private:
  int userId_;
  int postId_;
};

// Enqueue the job
NotificationJob::perform_later(user.id(), post.id());
```

## Documentation

For more detailed documentation, visit:
- [Cyclone Guides](https://cyclone-framework.org/guides)
- [API Reference](https://cyclone-framework.org/api)
- [Examples](https://cyclone-framework.org/examples)

## License

Cyclone is available as open source under the terms of the [MIT License](LICENSE).