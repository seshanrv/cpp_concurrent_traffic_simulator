#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include "TrafficLight.h"

using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 

    std::unique_lock<std::mutex> ulock(_mutex);
    _cond.wait(ulock, [this]{ return !_queue.empty(); });
    T msg = std::move(_queue.back());
    _queue.pop_back();
    std::cout << "message: " << msg << "removed from queue" << std::endl;
    return msg;

}

template <typename T>
void MessageQueue<T>::send(T&& msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> ulock(_mutex);
    _queue.emplace_back(msg);
    std::cout << "message: " << msg << "added to queue" << std::endl;
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    __currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(_msgQueue.receive()!=green)
    {
        std::this_thread::sleep_for(milliseconds(1));
    }
    return;
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return __currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
    
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    auto prev_time = high_resolution_clock::now();
    // auto loop_time = milliseconds(0);

    int rand_cycle_dur = 4000 + rand() % 2000;
    std::cout << "random cycle duration in ms: " << rand_cycle_dur << std::endl;

    while(true)
    {   
        auto loop_time = std::chrono::duration_cast<milliseconds>(high_resolution_clock::now() - prev_time);  

        if(loop_time.count() >= rand_cycle_dur)
        {
            auto newPhase = __currentPhase == red ? green : red; 
            __currentPhase = newPhase;

            _msgQueue.send(std::move(newPhase));
            
            prev_time = high_resolution_clock::now();
            std::cout << "loop_time: " << loop_time.count() <<std::endl;
        }
        std::this_thread::sleep_for(milliseconds(1)); 
    }
}