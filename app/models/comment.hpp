#pragma once

#include "cyclone/model.hpp"
#include "user.hpp"
#include "post.hpp"

class Comment : public Cyclone::Model<Comment> {
public:
    static void defineSchema() {
        schema()
          .field<int>("id", {.primaryKey = true, .autoIncrement = true})
          .field<int>("user_id", {.nullable = false})
          .field<int>("post_id", {.nullable = false})
          .field<std::string>("content", {.nullable = false})
          .timestamps();

        // Define relationships
        belongsTo<User>();
        belongsTo<Post>();
        hasMany<Like>();
    }

    // Validations
    static void defineValidations() {
        validates("content", {.presence = true, .length = {.minimum = 2, .maximum = 1000}});
        validates("user_id", {.presence = true});
        validates("post_id", {.presence = true});
    }

    // Scopes
    static QueryBuilder<Comment> recent() {
        return orderBy("created_at", "DESC").limit(5);
    }

    // Methods
    int likeCount() const {
        return likes().count();
    }
};