#pragma once

#include "cyclone/migration.hpp"

namespace Migrations {

    class CreateLikes : public Cyclone::Migration {
    public:
        void up() override {
            createTable("likes", [](Cyclone::Schema::Table& t) {
              t.integer("id", {.primaryKey = true, .autoIncrement = true});
              t.integer("user_id", {.nullable = false});
              t.integer("likeable_id", {.nullable = false});
              t.string("likeable_type", {.nullable = false});
              t.timestamps();

              // Add foreign key constraints
              t.foreignKey("user_id", {
                .table = "users",
                .column = "id",
                .onDelete = Cyclone::Schema::ForeignKeyAction::Cascade,
                .onUpdate = Cyclone::Schema::ForeignKeyAction::Cascade
              });
            });

            // Add indexes
            addIndex("likes", "user_id");
            addIndex("likes", {"likeable_id", "likeable_type"});

            // Add unique constraint to prevent duplicate likes
            addUniqueIndex("likes", {"user_id", "likeable_id", "likeable_type"});
        }

        void down() override {
            dropTable("likes");
        }
    };

} // namespace Migrations

// Register migration
CYCLONE_REGISTER_MIGRATION(Migrations::CreateLikes, 20240301120300);