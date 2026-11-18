#pragma once
#include <vector>

// Lightweight YIN fundamental-frequency detector for monophonic sources.
class YinPitchDetector
{
public:
    explicit YinPitchDetector(int windowSize = 2048);
    void prepare(double newSampleRate);
    // Returns 0.0 when no reliable pitch is found.
    float detect(const float* samples, int numSamples, float threshold = 0.15f) const;

private:
    int windowSize;
    double sampleRate = 44100.0;
    mutable std::vector<float> difference;
    mutable std::vector<float> cumulativeMean;
};
