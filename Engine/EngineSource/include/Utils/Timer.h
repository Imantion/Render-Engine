#pragma once
#include <chrono>
#include <thread>

namespace Engine
{

    class Timer {

    public:
        Timer() {
            deltatime = 0.0f;
            m_startTime = clock::now();
        }
        bool timeElapsed(int frameRate)
        {
            double frameInterval = 1.0f / frameRate; // if frameInterval = 1, means 1 second passed
            calculateDeltatime();
            if (deltatime >= frameInterval) { // checks if elapsed time equels or above of 1 frame time
                m_startTime = clock::now();
                return true;
            }
            return false;
        }

        float getDeltatime() {
            return deltatime;
        }
    private:
        void calculateDeltatime()
        {
            time_point currentTime = clock::now();
            std::chrono::duration<float, std::milli> elapsedTime = currentTime - m_startTime;
            deltatime = elapsedTime.count() * 0.001f;
        }

    private:
        using clock = std::chrono::high_resolution_clock;
        using time_point = std::chrono::time_point<clock>;

        time_point m_startTime;
        float deltatime;
    
    };

}