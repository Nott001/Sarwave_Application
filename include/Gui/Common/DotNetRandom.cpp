#include "Gui/Common/DotNetRandom.hpp"

#include <climits>
#include <cstdlib>

namespace Sarwave::Core::Gui::Common {

namespace {
constexpr int MSeed = 161803398;
}

DotNetRandom::DotNetRandom(int seed) {
    // CompatPrng(int seed), unchanged since .NET Framework.
    const int subtraction = seed == INT_MIN ? INT_MAX : std::abs(seed);
    int mj = MSeed - subtraction;
    m_seedArray[55] = mj;
    int mk = 1;
    int ii = 0;
    for (int i = 1; i < 55; i++) {
        // The range [1..55] is special (Knuth) and so the 0'th position is wasted.
        if ((ii += 21) >= 55) ii -= 55;
        m_seedArray[ii] = mk;
        mk = mj - mk;
        if (mk < 0) mk += INT_MAX;
        mj = m_seedArray[ii];
    }

    for (int k = 1; k < 5; k++) {
        for (int i = 1; i < 56; i++) {
            int n = i + 30;
            if (n >= 55) n -= 55;
            m_seedArray[i] -= m_seedArray[1 + n];
            if (m_seedArray[i] < 0) m_seedArray[i] += INT_MAX;
        }
    }
}

int DotNetRandom::internalSample() {
    int locINext = m_inext;
    int locINextp = m_inextp;
    if (++locINext >= 56) locINext = 1;
    if (++locINextp >= 56) locINextp = 1;

    int retVal = m_seedArray[locINext] - m_seedArray[locINextp];
    if (retVal == INT_MAX) retVal--;
    if (retVal < 0) retVal += INT_MAX;

    m_seedArray[locINext] = retVal;
    m_inext = locINext;
    m_inextp = locINextp;
    return retVal;
}

double DotNetRandom::sample() {
    return internalSample() * (1.0 / INT_MAX);
}

int DotNetRandom::next() {
    return internalSample();
}

double DotNetRandom::nextDouble() {
    return sample();
}

float DotNetRandom::nextSingle() {
    return static_cast<float>(sample());
}

}  // namespace Sarwave::Core::Gui::Common
