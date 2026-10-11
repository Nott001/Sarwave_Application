#pragma once

#include <array>

namespace Sarwave::Core::Gui::Common {

/// System.Random constructed with a seed, reproduced number for number: .NET keeps the legacy
/// Knuth subtractive generator (Net5CompatSeedImpl / CompatPrng) for seeded instances, so design
/// data scattered with `new Random(1)` comes out at exactly the same places here.
class DotNetRandom {
   public:
    explicit DotNetRandom(int seed);

    /// Random.Next(): a non-negative int below int.MaxValue.
    int next();

    /// Random.NextDouble(): [0, 1).
    double nextDouble();

    /// Random.NextSingle(): (float)Sample() for a seeded instance.
    float nextSingle();

   private:
    int internalSample();
    double sample();

    std::array<int, 56> m_seedArray{};
    int m_inext = 0;
    int m_inextp = 21;
};

}  // namespace Sarwave::Core::Gui::Common
