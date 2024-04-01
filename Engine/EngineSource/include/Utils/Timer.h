#pragma once
#include <chrono>
#include <thread>

namespace Engine
{

    class Timer {

    public:
        Timer() {
            m_startTime = clock::now();
        }
        bool timeElapsed(int frameRate)
        {
            double frameInterval = 1000.0f / frameRate;
            time_point currentTime = clock::now();
            std::chrono::duration<float, std::milli> elapsedTime = currentTime - m_startTime; // calculating elapsed time
            if (elapsedTime.count() >= frameInterval) { // checks if elapsed time equels or above of 1 frame time
                deltatime = elapsedTime.count();
                m_startTime = currentTime;
                return true;
            }
            return false;
        }

        float DeltaTime() {
            return deltatime * 0.001;
        }

    private:
        using clock = std::chrono::high_resolution_clock;
        using time_point = std::chrono::time_point<clock>;

        time_point m_startTime;
        float deltatime;
    
    };

}