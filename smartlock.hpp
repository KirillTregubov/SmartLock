/**
 * @file smartlock.hpp
 * @author Kirill Tregubov (KirillTregubov)
 * @author Philip Cai (Gadnalf)
 * @copyright Copyright (c) 2022 Kirill Tregubov & Philip Cai
 *
 * @brief This header defines the class for operating Smart Lock.
 * @bug No known bugs.
 */
#ifndef SMARTLOCK_H
#define SMARTLOCK_H

#include "mbed.h"

class SmartLock {
public:
  enum lock_state_t { LOCKED, UNLOCKED };

  /**
   * @brief Construct a new SmartLock object.
   *
   * @return Instance of SmartLock.
   */
  SmartLock(events::EventQueue *event_queue);

  /**
   * @brief Lock the Smart Lock and update states.
   *
   * @return Void.
   */
  void lock();

  /**
   * @brief Unlock the Smart Lock and update states.
   *
   * @return Void.
   */
  void unlock();

private:
  lock_state_t _lock_state;
  EventQueue *_event_queue;
  AnalogOut _out_pin;
  DigitalOut _led1;
  DigitalOut _led2;

  /**
   * @brief Update the state of the Smart Lock.
   *
   * @return Void.
   */
  void _update_state(lock_state_t new_state);
};

#endif // SMARTLOCK_H
