#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <chrono>
#include <thread>
#include <mutex>

/* Implementation of class "MessageQueue" */


template <typename T>
T&& MessageQueue<T>::receive()
{
  std::unique_lock lck(_mutex, std::try_to_lock);
  auto chk = [&_deque]() -> bool{return !_deque.empty();};
  _cond.wait(lck, chk);
  T &&ret = std::move(_deque.front()); 
  _deque.popFront(); 
  return std::move(ret);
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
  std::lock_guard grd(_mtx);
  _queue.push_back(std::move(msg));
  _cond.notify_one();
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _type = ObjectType::objectLight; 
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
  while(_q.receive() != TrafficLightPhase::green);
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
  threads.emplace_back(&TrafficLight::cycleThroughPhases, this);
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
  using namespace std::chrono;
  std::uniform_int_distribution dist{4000, 6000};
  time_point last_run = steady_clock::now() - milliseconds(6000);
  auto seed = static_cast<default_random_engine::result_type>(last_run.time_since_epoch().count()); //seed randome generator with current time in millis. 
  std::default_random_engine generator{seed); 
  auto next_duration = milliseconds(dist(generator)); 
  while(true){
    time_point t = steady_clock::now();
    seconds secs = t - last_run;
    if(secs > next_duration) {
      last_run = t; 
      next_duration = milliseconds(dist(generator));
      switch (_currentPhase){
        case TrafficLightPhase::red:
          _currentPhase = TrafficLightPhase::green;
          break;
        case TrafficLightPhase::green:
          _currentPhase = TrafficLightPhase::red;
          break;
      }
      _q.sendMessage(_currentPhase); 
    }
    std::this_thread::sleep_for(milliseconds(1)); 
  }
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
}

