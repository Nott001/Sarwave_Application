// Runs the .cfg parity harness over the repo's sample configs and the corpus of tricky lines,
// and requires exactly the text the original C# code printed (data/config/golden_cs.txt).

#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>

#include "TestFramework.hpp"

#define SARWAVE_CONFIG_PARITY_NO_MAIN
#include "Parity/ConfigParityHarness.cpp"

TEST_CASE("parity: the .cfg model parses, writes and rejects exactly like the C# version") {
    const std::string actual = configParityRun(
        {SARWAVE_TEST_DATA_DIR "/config/corpus.txt", SARWAVE_SOURCE_DIR "/configs", "--"});

    std::ifstream in(SARWAVE_TEST_DATA_DIR "/config/golden_cs.txt", std::ios::binary);
    REQUIRE(in.is_open());
    const std::string expected{std::istreambuf_iterator<char>(in),
                               std::istreambuf_iterator<char>()};

    if (actual != expected) {
        std::istringstream a(actual);
        std::istringstream e(expected);
        std::string la;
        std::string le;
        for (int line = 1;; ++line) {
            const bool ga = static_cast<bool>(std::getline(a, la));
            const bool ge = static_cast<bool>(std::getline(e, le));
            if (!ga && !ge) break;
            if (ga != ge || la != le) {
                SarwaveTest::fail(__FILE__, __LINE__,
                                  "first difference at line " + std::to_string(line) +
                                      "\n      C++: [" + la + "]\n      C#:  [" + le + "]");
                break;
            }
        }
    }
}
