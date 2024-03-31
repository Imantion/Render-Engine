#pragma once
#include <chrono>
#include <thread>

namespace Engine
{

    class Timer {

    public:
        Timer(int frameRate) : m_frameInterval(1000.0 / frameRate) {
            m_startTime = clock::now();
        }
        bool FrameElapsed()
        {
            time_point currentTime = clock::now();
            std::chrono::duration<double, std::milli> elapsedTime = currentTime - m_startTime; // calculating elapsed time
            if (elapsedTime.count() >= m_frameInterval) { // checks if elapsed time equels or above of 1 frame time
                deltatime = elapsedTime.count();
                time_to_sleep = 0; // no need to sleep thread because frame was elapsed;
                m_startTime = currentTime;
                return true;
            }
            time_to_sleep = m_frameInterval - elapsedTime.count(); // time for thread to sleep
            return false;
        }

        void Sleep()
        {
            auto start = clock::now();
        
            while ((clock::now() - start).count() < time_to_sleep) {
                std::this_thread::yield();
           }
        }
        float DeltaTime() {
            return deltatime * 0.001;
        }

    private:
        using clock = std::chrono::high_resolution_clock;
        using time_point = std::chrono::time_point<clock>;
        time_point m_startTime;
        time_point previousTime;

        float deltatime;
        double time_to_sleep;
        double m_frameInterval; // Interval in milliseconds for each frame
    
    };

}