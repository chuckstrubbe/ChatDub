#include "YinPitchDetector.h"
#include <algorithm>
#include <cmath>

YinPitchDetector::YinPitchDetector(int size)
    : windowSize(size), difference(static_cast<size_t>(size / 2), 0.0f),
      cumulativeMean(static_cast<size_t>(size / 2), 0.0f)
{
}

void YinPitchDetector::prepare(double newSampleRate)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
}

float YinPitchDetector::detect(const float* x, int count, float threshold) const
{
    if (x == nullptr || count < windowSize)
        return 0.0f;

    const int maxTau = std::min(windowSize / 2 - 1, static_cast<int>(sampleRate / 45.0));
    const int minTau = std::max(2, static_cast<int>(sampleRate / 1100.0));
    if (maxTau <= minTau)
        return 0.0f;

    std::fill(difference.begin(), difference.end(), 0.0f);
    for (int tau = 1; tau <= maxTau; ++tau)
    {
        double sum = 0.0;
        for (int i = 0; i < windowSize - maxTau; ++i)
        {
            const float delta = x[i] - x[i + tau];
            sum += static_cast<double>(delta) * delta;
        }
        difference[static_cast<size_t>(tau)] = static_cast<float>(sum);
    }

    cumulativeMean[0] = 1.0f;
    float runningSum = 0.0f;
    for (int tau = 1; tau <= maxTau; ++tau)
    {
        runningSum += difference[static_cast<size_t>(tau)];
        cumulativeMean[static_cast<size_t>(tau)] = runningSum > 0.0f
            ? difference[static_cast<size_t>(tau)] * static_cast<float>(tau) / runningSum
            : 1.0f;
    }

    int selectedTau = -1;
    for (int tau = minTau; tau <= maxTau; ++tau)
    {
        if (cumulativeMean[static_cast<size_t>(tau)] < threshold)
        {
            while (tau + 1 <= maxTau && cumulativeMean[static_cast<size_t>(tau + 1)] < cumulativeMean[static_cast<size_t>(tau)])
                ++tau;
            selectedTau = tau;
            break;
        }
    }

    if (selectedTau < 0)
        return 0.0f;

    // Parabolic interpolation improves frequency precision between integer lags.
    const int tau = selectedTau;
    float betterTau = static_cast<float>(tau);
    if (tau > 1 && tau < maxTau)
    {
        const float s0 = cumulativeMean[static_cast<size_t>(tau - 1)];
        const float s1 = cumulativeMean[static_cast<size_t>(tau)];
        const float s2 = cumulativeMean[static_cast<size_t>(tau + 1)];
        const float denominator = 2.0f * (2.0f * s1 - s2 - s0);
        if (std::abs(denominator) > 1.0e-9f)
            betterTau += (s2 - s0) / denominator;
    }

    return betterTau > 0.0f ? static_cast<float>(sampleRate / betterTau) : 0.0f;
}
