#pragma once

#include "cyclone/schema.hpp"

namespace Schema {

// This file is auto-generated by Cyclone when migrations are run
// Do not modify this file directly - instead create a new migration

void define(Cyclone::Schema& schema) {
  schema.createTable("users", [](Cyclone::Schema::Table& t) {
    t.integer("id", {.primaryKey = true, .autoIncrement = true});
    t.string("email", {.nullable = false, .unique = true});
    t.string("name", {.nullable = false});
    t.string("encrypted_password", {.nullable = false});
    t.string("reset_password_token", {.unique = true});
    t.datetime("reset_password_sent_at");
    t.string("confirmation_token", {.unique = true});
    t.datetime("confirmed_at");
    t.datetime("confirmation_sent_at");
    t.string("unconfirmed_email");
    t.integer("failed_attempts", {.default_ = 0});
    t.string("unlock_token", {.unique = true});
    t.datetime("locked_at");
    t.string("remember_token");
    t.datetime("remember_created_at");
    t.string("role", {.default_ = "user"});
    t.datetime("created_at");
    t.datetime("updated_at");
  });

  schema.createTable("posts", [](Cyclone::Schema::Table& t) {
    t.integer("id", {.primaryKey = true, .autoIncrement = true});
    t.integer("user_id", {.nullable = false});
    t.string("title", {.nullable = false});
    t.text("content", {.nullable = false});
    t.boolean("published", {.default_ = false});
    t.datetime("published_at");
    t.datetime("created_at");
    t.datetime("updated_at");

    t.foreignKey("user_id", {
      .table = "users",
      .column = "id",
      .onDelete = Cyclone::Schema::ForeignKeyAction::Cascade,
      .onUpdate = Cyclone::Schema::ForeignKeyAction::Cascade
    });
  });

  schema.createTable("comments", [](Cyclone::Schema::Table& t) {
    t.integer("id", {.primaryKey = true, .autoIncrement = true});
    t.integer("user_id", {.nullable = false});
    t.integer("post_id", {.nullable = false});
    t.text("content", {.nullable = false});
    t.datetime("created_at");
    t.datetime("updated_at");

    t.foreignKey("user_id", {
      .table = "users",
      .column = "id",
      .onDelete = Cyclone::Schema::ForeignKeyAction::Cascade,
      .onUpdate = Cyclone::Schema::ForeignKeyAction::Cascade
    });

    t.foreignKey("post_id", {
      .table = "posts",
      .column = "id",
      .onDelete = Cyclone::Schema::ForeignKeyAction::Cascade,
      .onUpdate = Cyclone::Schema::ForeignKeyAction::Cascade
    });
  });

  schema.createTable("likes", [](Cyclone::Schema::Table& t) {
    t.integer("id", {.primaryKey = true, .autoIncrement = true});
    t.integer("user_id", {.nullable = false});
    t.integer("likeable_id", {.nullable = false});
    t.string("likeable_type", {.nullable = false});
    t.datetime("created_at");
    t.datetime("updated_at");

    t.foreignKey("user_id", {
      .table = "users",
      .column = "id",
      .onDelete = Cyclone::Schema::ForeignKeyAction::Cascade,
      .onUpdate = Cyclone::Schema::ForeignKeyAction::Cascade
    });
  });

  // Add indexes
  schema.addIndex("users", "email");
  schema.addIndex("users", "reset_password_token");
  schema.addIndex("users", "confirmation_token");
  schema.addIndex("users", "unlock_token");

  schema.addIndex("posts", "user_id");
  schema.addIndex("posts", "published");
  schema.addIndex("posts", "published_at");

  schema.addIndex("comments", "user_id");
  schema.addIndex("comments", "post_id");
  schema.addIndex("comments", "created_at");

  schema.addIndex("likes", "user_id");
  schema.addIndex("likes", {"likeable_id", "likeable_type"});
  schema.addUniqueIndex("likes", {"user_id", "likeable_id", "likeable_type"});
}

} // namespace Schema