#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{ 
     std::unique_lock<std::mutex> uLock(_mutex);
     _cond_var.wait(uLock, [this] { return !_queue.empty(); }); 
     T msg = std::move(_queue.back());
     _queue.pop_back();
  
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
      std::lock_guard<std::mutex> uLock(_mutex); 
     _queue.push_back(std::move(msg));
     _cond_var.notify_one();
}
 

/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while(true)
    {
        TrafficLightPhase msg = TrafficLight::TrLightQueue.receive();
        
        if(msg == green)
        { return; }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(4000,6000);
    int cycleDuration = distribution(generator);
  
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;
  
    // init stop watch
    lastUpdate = std::chrono::system_clock::now();
    
    while(true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        cycleDuration = distribution(generator);
        // compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >= cycleDuration)
        {
          if(_currentPhase == red)
          {
            _currentPhase = green;
          }
          else
          {
            _currentPhase = red;
          }
          
          TrafficLight::TrLightQueue.send(std::move(_currentPhase));
        }
      
        // reset stop watch for next cycle
        lastUpdate = std::chrono::system_clock::now();
    }
}

