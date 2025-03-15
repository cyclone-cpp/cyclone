#pragma once

#include "cyclone/testing/test_case.hpp"
#include "cyclone/testing/expectations.hpp"
#include "cyclone/testing/parallel_runner.hpp"
#include "support/database_cleaner.hpp"

/**
 * Configure the testing framework for this application
 * Sets up RSpec-like syntax, parallel execution, and common test utilities
 */
class TestFramework {
public:
  /**
   * Initialize and configure the testing framework
   * Call once before running tests
   */
  static void initialize() {
    // Configure test runner
    auto& config = cyclone::testing::Config::instance();

    // Set up parallel test execution
    config.setParallelWorkers(4);  // Use 4 parallel workers
    config.setRandomTestOrder(true);  // Randomize test order to catch dependencies
    config.setSeed(std::time(nullptr));  // Random seed based on current time

    // Configure output format
    config.setOutputFormat(cyclone::testing::OutputFormat::Documentation);
    config.setColorOutput(true);

    // Register global hooks
    config.setBeforeAll([]() {
      // Set up test environment once before all tests
      DatabaseCleaner::truncateAll();
    });

    config.setAfterAll([]() {
      // Clean up test environment after all tests
      cyclone::testing::logInfo("All tests completed");
    });

    // Register per-test hooks
    config.setBeforeEach([](const std::string& testName) {
      // Set up clean database state for each test
      DatabaseCleaner::start();
    });

    config.setAfterEach([](const std::string& testName) {
      // Clean up after each test
      DatabaseCleaner::clean();
    });
  }
};

// Have the framework initialized automatically
namespace {
  struct AutoInitializer {
    AutoInitializer() {
      TestFramework::initialize();
    }
  };

  // Static instance will call constructor before main()
  static AutoInitializer initializer;
}

// Define RSpec-like syntax shortcuts
// These are thin wrappers around Cyclone's testing functions

// Define a group of related tests
#define describe(description, block) cyclone::testing::describeGroup(description, block)

// Define a specific test case
#define it(description, block) cyclone::testing::specifyTest(description, block)

// Create expectations (assertions)
template<typename T>
auto expect(const T& actual) {
  return cyclone::testing::Expectation<T>(actual);
}

// Register a test case class with the test runner
#define REGISTER_TEST_CASE(TestClass) \
  static cyclone::testing::TestRegistration<TestClass> TestClass##_registration(#TestClass);