#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 

    std::unique_lock<std::mutex> uLock (_mutex);
    this->_cond_var.wait(uLock, [this] {return ! this->_messages.empty();});
    // move ownership of this project also so it wont be deleted
    T msg = std::move(this->_messages.back());
    this->_messages.pop_back();

    // return value optimization in C++
    return msg;
}   

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    // try to get the unique lock to check the queue cond.
    std::lock_guard<std::mutex> lock_guard(_mutex);
    
    // now message queue is not empty anymore
    std::cout << "Adding message to queue" << std::endl;
    this->_messages.push_back(std::move(msg));
    // notify client waiting on new coming message
    this->_cond_var.notify_one();


}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::RED;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(1) {
        if (this->_msg_queue.receive() == TrafficLightPhase::GREEN) {
            return;
        }
    }
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „
    //simulate“ is called. To do this, use the thread queue in the base class. 
    std::thread simThread = std::thread(&TrafficLight::cycleThroughPhases, this);
    this->threads.push_back(std::move(simThread));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    auto start = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed;
    auto cycle_time = std::chrono::duration<float>(rand() % 2 + 4 + (static_cast<float> (rand())) / RAND_MAX);
    while(true) {
        elapsed = now - start;
        if (elapsed >= cycle_time) {
            // use current time as random seed 
            srand(time(0));
            cycle_time = std::chrono::duration<float>(rand() % 2 + 4 + (static_cast<float> (rand())) / RAND_MAX);
            start = std::chrono::high_resolution_clock::now();
            // change phase 
            TrafficLightPhase new_phase = TrafficLightPhase::RED == this->_currentPhase ? TrafficLightPhase::GREEN : TrafficLightPhase::RED;
            this->_currentPhase = new_phase;
            this->_msg_queue.send(std::move(new_phase));
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // always update now 
        now = std::chrono::high_resolution_clock::now();
    }

}