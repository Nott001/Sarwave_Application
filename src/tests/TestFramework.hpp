#pragma once

// A deliberately tiny test framework, so the core libraries can be tested without pulling in Qt
// or a third-party package.
//
//   TEST_CASE("frame reader resyncs on a second magic word") {
//       CHECK(x == 1);                 // records a failure and keeps going
//       REQUIRE(ptr != nullptr);       // records a failure and ends this test case
//       CHECK_EQ(a, b);                // prints both values on failure (needs operator<<)
//       CHECK_THROWS_AS(f(), SomeException);
//   }

#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace SarwaveTest {

struct TestCase {
    const char* name;
    const char* file;
    int line;
    std::function<void()> body;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> cases;
    return cases;
}

inline int& currentFailures() {
    static int failures = 0;
    return failures;
}

struct Registrar {
    Registrar(const char* name, const char* file, int line, std::function<void()> body) {
        registry().push_back({name, file, line, std::move(body)});
    }
};

struct RequireFailed {};

inline void fail(const char* file, int line, const std::string& message) {
    ++currentFailures();
    std::cerr << "    " << file << ":" << line << ": " << message << "\n";
}

}  // namespace SarwaveTest

#define SARWAVE_CONCAT_INNER(a, b) a##b
#define SARWAVE_CONCAT(a, b) SARWAVE_CONCAT_INNER(a, b)

#define TEST_CASE(name)                                                                 \
    static void SARWAVE_CONCAT(sarwave_test_, __LINE__)();                              \
    static const ::SarwaveTest::Registrar SARWAVE_CONCAT(sarwave_registrar_, __LINE__)( \
        name, __FILE__, __LINE__, &SARWAVE_CONCAT(sarwave_test_, __LINE__));            \
    static void SARWAVE_CONCAT(sarwave_test_, __LINE__)()

#define CHECK(expr)                                                                      \
    do {                                                                                 \
        if (!(expr)) ::SarwaveTest::fail(__FILE__, __LINE__, "CHECK(" #expr ") failed"); \
    } while (0)

#define REQUIRE(expr)                                                             \
    do {                                                                          \
        if (!(expr)) {                                                            \
            ::SarwaveTest::fail(__FILE__, __LINE__, "REQUIRE(" #expr ") failed"); \
            throw ::SarwaveTest::RequireFailed{};                                 \
        }                                                                         \
    } while (0)

#define CHECK_EQ(actual, expected)                                                         \
    do {                                                                                   \
        const auto& sarwave_a = (actual);                                                  \
        const auto& sarwave_e = (expected);                                                \
        if (!(sarwave_a == sarwave_e)) {                                                   \
            std::ostringstream sarwave_os;                                                 \
            sarwave_os << "CHECK_EQ(" #actual ", " #expected ") failed\n      actual:   [" \
                       << sarwave_a << "]\n      expected: [" << sarwave_e << "]";         \
            ::SarwaveTest::fail(__FILE__, __LINE__, sarwave_os.str());                     \
        }                                                                                  \
    } while (0)

#define CHECK_THROWS_AS(expr, type)                                              \
    do {                                                                         \
        bool sarwave_caught = false;                                             \
        try {                                                                    \
            (void)(expr);                                                        \
        } catch (const type&) {                                                  \
            sarwave_caught = true;                                               \
        } catch (...) {                                                          \
        }                                                                        \
        if (!sarwave_caught)                                                     \
            ::SarwaveTest::fail(__FILE__, __LINE__,                              \
                                "CHECK_THROWS_AS(" #expr ", " #type ") failed"); \
    } while (0)
