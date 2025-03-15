#pragma once

#include "cyclone/application.hpp"
#include "cyclone/engines/dash.hpp"
#include "cyclone/engines/pulse.hpp"
#include "cyclone/engines/fortress.hpp"

class Application : public Cyclone::Application {
public:
  Application() {
    // Set application name
    setName("Cyclone Blog");

    // Configure time zone
    setTimeZone("UTC");

    // Configure locales
    setDefaultLocale("en");
    addLocale("fr");
    addLocale("es");
    addLocale("de");

    // Configure URL options
    setHostname("localhost");
    setPort(3000);
    setForceSSL(false); // Set to true in production

    // Configure session
    setSessionStore(Cyclone::SessionStore::Cookie, {
      {"key", "cyclone_session"},
      {"secret", getEnv("SESSION_SECRET", "development_secret")},
      {"max_age", "2592000"} // 30 days
    });

    // Configure cache
    setCacheStore(Cyclone::CacheStore::Memory, {
      {"size_mb", "64"}
    });

    // Mount engines
    mountEngines();

    // Register middleware
    registerMiddleware();

    // Auto-load paths
    addAutoloadPath("app/helpers");
    addAutoloadPath("lib");
  }

private:
  void mountEngines() {
    // Mount the Dash admin engine
    mount(Cyclone::Engines::Dash::Engine, {
      .path = "/admin",
      .title = "Cyclone Admin",
      .theme = "cyclone-light",
      .models = {"User", "Post", "Comment", "Like"},
      .custom_dashboards = true
    });

    // Mount the Pulse job processing engine
    mount(Cyclone::Engines::Pulse::Engine, {
      .path = "/pulse",
      .access = Cyclone::Engines::Pulse::AccessLevel::AdminOnly,
      .retention_days = 7,
      .queues = {"default", "mailers", "reports"}
    });

    // Mount the Fortress authentication engine
    mount(Cyclone::Engines::Fortress::Engine, {
      .path = "/",
      .models = {"User"},
      .modules = {
        "Authenticatable",
        "Registerable",
        "Recoverable",
        "Rememberable",
        "Trackable",
        "Confirmable",
        "Lockable",
        "Timeoutable"
      }
    });
  }

  void registerMiddleware() {
    // Add middleware for all environments
    use(Cyclone::Middleware::RequestLogger);
    use(Cyclone::Middleware::MethodOverride);
    use(Cyclone::Middleware::ParamsParser);
    use(Cyclone::Middleware::Cookies);
    use(Cyclone::Middleware::Sessions);
    use(Cyclone::Middleware::FlashMessages);

    // Environment-specific middleware
    if (isDevelopment()) {
      use(Cyclone::Middleware::Reloader);
      use(Cyclone::Middleware::ErrorPages, {
        {"detailed", "true"}
      });
    } else {
      use(Cyclone::Middleware::ErrorPages);
      use(Cyclone::Middleware::HttpCache);

      if (isProduction()) {
        use(Cyclone::Middleware::SecurityHeaders);
        use(Cyclone::Middleware::ForceSSL);
      }
    }
  }

  std::string getEnv(const std::string& key, const std::string& defaultValue = "") {
    auto value = std::getenv(key.c_str());
    return value ? std::string(value) : defaultValue;
  }
};