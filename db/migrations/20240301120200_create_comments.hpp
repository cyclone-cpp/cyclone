#pragma once

#include "cyclone/migration.hpp"

namespace Migrations {

    class CreateComments : public Cyclone::Migration {
    public:
        void up() override {
            createTable("comments", [](Cyclone::Schema::Table& t) {
              t.integer("id", {.primaryKey = true, .autoIncrement = true});
              t.integer("user_id", {.nullable = false});
              t.integer("post_id", {.nullable = false});
              t.text("content", {.nullable = false});
              t.timestamps();

              // Add foreign key constraints
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

            // Add indexes
            addIndex("comments", "user_id");
            addIndex("comments", "post_id");
            addIndex("comments", "created_at");
        }

        void down() override {
            dropTable("comments");
        }
    };

} // namespace Migrations

// Register migration
CYCLONE_REGISTER_MIGRATION(Migrations::CreateComments, 20240301120200);