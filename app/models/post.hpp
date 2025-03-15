#pragma once

#include "cyclone/model.hpp"
#include "user.hpp"

class Post : public Cyclone::Model<Post> {
public:
    static void defineSchema() {
        schema()
          .field<int>("id", {.primaryKey = true, .autoIncrement = true})
          .field<int>("user_id", {.nullable = false})
          .field<std::string>("title", {.nullable = false})
          .field<std::string>("content", {.nullable = false})
          .field<bool>("published", {.default_ = false})
          .field<TimePoint>("published_at")
          .timestamps();

        // Define relationships
        belongsTo<User>();
        hasMany<Comment>();
        hasMany<Like>();
    }

    // Validations
    static void defineValidations() {
        validates("title", {.presence = true, .length = {.minimum = 3, .maximum = 255}});
        validates("content", {.presence = true});
        validates("user_id", {.presence = true});
    }

    // Scopes
    static QueryBuilder<Post> published() {
        return where("published", true)
          .where("published_at", "<=", TimePoint::now());
    }

    static QueryBuilder<Post> recent() {
        return orderBy("created_at", "DESC").limit(5);
    }

    // Callbacks
    void beforeSave() {
        if (published() && !published_at()) {
            setPublishedAt(TimePoint::now());
        }
    }

    // Methods
    int commentCount() const {
        return comments().count();
    }

    int likeCount() const {
        return likes().count();
    }
};