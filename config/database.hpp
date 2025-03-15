#pragma once

#include "cyclone/database.hpp"

void configureDatabases() {
  using namespace Cyclone::Database;

  // Default database configuration
  Configuration defaultConfig;

  // Environment-specific configurations
  if (Cyclone::isDevelopment()) {
    defaultConfig = {
      .adapter = Adapter::SQLite,
      .database = "db/development.sqlite3",
      .pool_size = 5,
      .timeout = std::chrono::seconds(5)
    };
  } else if (Cyclone::isTest()) {
    defaultConfig = {
      .adapter = Adapter::SQLite,
      .database = "db/test.sqlite3",
      .pool_size = 5,
      .timeout = std::chrono::seconds(5)
    };
  } else { // Production
    defaultConfig = {
      .adapter = Adapter::PostgreSQL,
      .host = getEnv("DB_HOST", "localhost"),
      .port = std::stoi(getEnv("DB_PORT", "5432")),
      .database = getEnv("DB_NAME", "cyclone_production"),
      .username = getEnv("DB_USER", "cyclone"),
      .password = getEnv("DB_PASSWORD", ""),
      .pool_size = std::stoi(getEnv("DB_POOL_SIZE", "10")),
      .timeout = std::chrono::seconds(std::stoi(getEnv("DB_TIMEOUT", "5"))),
      .ssl_mode = getEnv("DB_SSL_MODE", "prefer")
    };
  }

  // Configure the default connection
  configure(defaultConfig);

  // You can configure additional database connections if needed
  // For example, a read-only replica for reports
  if (Cyclone::isProduction() && getEnv("DB_REPLICA_HOST") != "") {
    Configuration replicaConfig = {
      .adapter = Adapter::PostgreSQL,
      .host = getEnv("DB_REPLICA_HOST"),
      .port = std::stoi(getEnv("DB_REPLICA_PORT", "5432")),
      .database = getEnv("DB_NAME", "cyclone_production"),
      .username = getEnv("DB_REPLICA_USER", getEnv("DB_USER", "cyclone")),
      .password = getEnv("DB_REPLICA_PASSWORD", getEnv("DB_PASSWORD", "")),
      .pool_size = std::stoi(getEnv("DB_REPLICA_POOL_SIZE", "5")),
      .timeout = std::chrono::seconds(std::stoi(getEnv("DB_REPLICA_TIMEOUT", "5"))),
      .ssl_mode = getEnv("DB_REPLICA_SSL_MODE", "prefer"),
      .read_only = true
    };

    // Register the replica connection
    configure(replicaConfig, "replica");
  }
}

// Helper function to get environment variables
std::string getEnv(const std::string& key, const std::string& defaultValue = "") {
  const char* value = std::getenv(key.c_str());
  return value ? std::string(value) : defaultValue;
}