#pragma once

#include "cyclone/migration.hpp"

namespace Migrations {

    class CreateUsers : public Cyclone::Migration {
    public:
        void up() override {
            createTable("users", [](Cyclone::Schema::Table& t) {
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
              t.timestamps();
            });

            // Add indexes
            addIndex("users", "email");
            addIndex("users", "reset_password_token");
            addIndex("users", "confirmation_token");
            addIndex("users", "unlock_token");
        }

        void down() override {
            dropTable("users");
        }
    };

} // namespace Migrations

// Register migration
CYCLONE_REGISTER_MIGRATION(Migrations::CreateUsers, 20240301120000);