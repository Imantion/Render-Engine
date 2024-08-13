#pragma once
#include <random>

static std::mt19937 eng;
static std::uniform_real_distribution<> g_distribution_0_1(0.0f, 1.0f);
static std::uniform_real_distribution<> g_distribution_0_2PI(0.0f, 2.0f * 3.14159265359f);
static std::uniform_real_distribution<> g_distribution_0_2(0.0f, 2.0f);


inline float get_random(float min, float max)
{
	static std::uniform_real_distribution<> dis(min, max);
	return (float)dis(eng);
}

inline float get_random(std::uniform_real_distribution<>& distribution)
{
	return (float)distribution(eng);
}