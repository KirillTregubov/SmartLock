/**
 * @file smartlock.cpp
 * @author Kirill Tregubov (KirillTregubov)
 * @author Philip Cai (Gadnalf)
 * @copyright Copyright (c) 2022 Kirill Tregubov & Philip Cai
 *
 * @brief This module contains functionality for operating Smart Lock.
 * @bug No known bugs.
 */
#include "smartlock.hpp"

SmartLock::SmartLock(EventQueue *event_queue)
    : _lock_state(SmartLock::LOCKED), _event_queue(event_queue), _out_pin(D7),
      _led1(LED1), _led2(LED2) {
  _out_pin = 0.0f;

  printf("> SmartLock initialized to LOCKED\n");
}

void SmartLock::lock() {
  _update_state(SmartLock::LOCKED);
  printf("> SmartLock LOCKED\n");

  for (int i = 0; i < 2; i++) {
    _led1 = 1;
    thread_sleep_for(300);
    _led1 = 0;
    thread_sleep_for(150);
  }
}

void SmartLock::unlock() {
  _update_state(SmartLock::UNLOCKED);
  printf("> SmartLock UNLOCKED\n");

  _led2 = 1;
  for (int i = 0; i < 2; i++) {
    _led1 = 1;
    thread_sleep_for(300);
    _led1 = 0;
    if (i < 1) {
      thread_sleep_for(150);
    }
  }
  _led2 = 0;

  _event_queue->call_in(7500ms, this, &SmartLock::lock);
}

bool SmartLock::is_unlocked() { return _lock_state == SmartLock::UNLOCKED; }

void SmartLock::_update_state(lock_state_t new_state) {
  _lock_state = new_state;

  if (new_state == SmartLock::UNLOCKED) {
    _out_pin = 1.0f;
  } else {
    _out_pin = 0.0f;
  }
}
