#include <cstring>

#include "TestFramework.hpp"

// Runs every registered TEST_CASE (or only those whose name contains argv[1]).
int main(int argc, char** argv) {
    const char* filter = argc > 1 ? argv[1] : nullptr;
    int failedCases = 0;
    int ran = 0;

    for (const auto& test : SarwaveTest::registry()) {
        if (filter && !std::strstr(test.name, filter)) continue;
        ++ran;

        SarwaveTest::currentFailures() = 0;
        try {
            test.body();
        } catch (const SarwaveTest::RequireFailed&) {
        } catch (const std::exception& e) {
            SarwaveTest::fail(test.file, test.line,
                              std::string("unexpected exception: ") + e.what());
        } catch (...) {
            SarwaveTest::fail(test.file, test.line, "unexpected non-standard exception");
        }

        const bool ok = SarwaveTest::currentFailures() == 0;
        if (!ok) ++failedCases;
        std::cout << (ok ? "[ OK ] " : "[FAIL] ") << test.name << "\n";
    }

    std::cout << "\n" << ran - failedCases << "/" << ran << " test cases passed\n";
    return failedCases == 0 ? 0 : 1;
}
