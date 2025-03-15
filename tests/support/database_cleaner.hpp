#pragma once

#include "cyclone/testing/database_helper.hpp"

/**
 * Simple wrapper around Cyclone's built-in database cleaning functionality
 * Provides a convenient API for test setup and teardown
 */
class DatabaseCleaner {
public:
    /**
     * Start a new testing transaction
     * Call this before each test to begin a clean slate
     */
    static void start() {
        cyclone::testing::DatabaseHelper::beginTransaction();
    }

    /**
     * Roll back the testing transaction
     * Call this after each test to discard all changes
     */
    static void clean() {
        cyclone::testing::DatabaseHelper::rollbackTransaction();
    }

    /**
     * Truncate all tables in the test database
     * Use this before running the entire test suite
     */
    static void truncateAll() {
        cyclone::testing::DatabaseHelper::truncateAllTables();
    }

    /**
     * Ensure specific tables are empty
     * Useful when specific tests need a clean state for certain models
     *
     * @param tables List of table names to truncate
     */
    static void truncateTables(const std::vector<std::string>& tables) {
        for (const auto& table : tables) {
            cyclone::testing::DatabaseHelper::truncateTable(table);
        }
    }
};