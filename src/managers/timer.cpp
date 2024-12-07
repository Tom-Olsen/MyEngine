#include "timer.h"



// Static members:
float Timer::time = 0.0f;
float Timer::deltaTime = 0.0f;
bool Timer::isInitialized = false;
std::chrono::steady_clock::time_point Timer::start = std::chrono::steady_clock::now();
std::chrono::steady_clock::time_point Timer::end = std::chrono::steady_clock::now();



// Timer management:
void Timer::Init()
{
	if (isInitialized)
		return;

	isInitialized = true;
}
void Timer::Clear()
{
	time = 0.0f;
	deltaTime = 0.0f;
	isInitialized = false;
}
void Timer::Update()
{
	end = std::chrono::steady_clock::now();
	deltaTime = std::chrono::duration<float>(end - start).count();
	time += deltaTime;
	start = end;
}
void Timer::Reset()
{
	time = 0.0f;
	deltaTime = 0.0f;
	auto start = std::chrono::steady_clock::now();
}



// Getters:
float Timer::GetTime()
{
	return time;
}
float Timer::GetDeltaTime()
{
	return deltaTime;
}