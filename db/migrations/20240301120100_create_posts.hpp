#pragma once

#include "cyclone/migration.hpp"

namespace Migrations {

    class CreatePosts : public Cyclone::Migration {
    public:
        void up() override {
            createTable("posts", [](Cyclone::Schema::Table& t) {
              t.integer("id", {.primaryKey = true, .autoIncrement = true});
              t.integer("user_id", {.nullable = false});
              t.string("title", {.nullable = false});
              t.text("content", {.nullable = false});
              t.boolean("published", {.default_ = false});
              t.datetime("published_at");
              t.timestamps();

              // Add foreign key constraint
              t.foreignKey("user_id", {
                .table = "users",
                .column = "id",
                .onDelete = Cyclone::Schema::ForeignKeyAction::Cascade,
                .onUpdate = Cyclone::Schema::ForeignKeyAction::Cascade
              });
            });

            // Add indexes
            addIndex("posts", "user_id");
            addIndex("posts", "published");
            addIndex("posts", "published_at");
        }

        void down() override {
            dropTable("posts");
        }
    };

} // namespace Migrations

// Register migration
CYCLONE_REGISTER_MIGRATION(Migrations::CreatePosts, 20240301120100);