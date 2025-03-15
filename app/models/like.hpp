#pragma once

#include "cyclone/model.hpp"
#include "user.hpp"
#include "post.hpp"
#include "comment.hpp"

class Like : public Cyclone::Model<Like> {
public:
    static void defineSchema() {
        schema()
          .field<int>("id", {.primaryKey = true, .autoIncrement = true})
          .field<int>("user_id", {.nullable = false})
          .field<int>("likeable_id", {.nullable = false})
          .field<std::string>("likeable_type", {.nullable = false})
          .timestamps();

        // Define relationships
        belongsTo<User>();
        polymorphicBelongsTo("likeable", {{"Post", "Comment"}});

        // Define unique index
        uniqueIndex({"user_id", "likeable_id", "likeable_type"});
    }

    // Validations
    static void defineValidations() {
        validates("user_id", {.presence = true});
        validates("likeable_id", {.presence = true});
        validates("likeable_type", {.presence = true, .inclusion = {"Post", "Comment"}});
        validates("uniqueness", {.scope = {"user_id", "likeable_id", "likeable_type"}});
    }

    // Scopes
    static QueryBuilder<Like> forPost(int postId) {
        return where("likeable_type", "Post").where("likeable_id", postId);
    }

    static QueryBuilder<Like> forComment(int commentId) {
        return where("likeable_type", "Comment").where("likeable_id", commentId);
    }

    static QueryBuilder<Like> byUser(int userId) {
        return where("user_id", userId);
    }
};