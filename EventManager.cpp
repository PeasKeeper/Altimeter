#include "Arduino.h"

#include "EventManager.h"

EventManager::EventManager() {
  _numEvents = 0;
}

void EventManager::pushEvent(int eventId, unsigned long t) {
  unsigned long tEnd = millis() + t;
  //Serial.print("Pushing: ");
  //Serial.print(eventId);
  //Serial.print("/");
  //Serial.println(tEnd);
  if (_numEvents == 0) {
    _eventStack[0].time = tEnd;
    _eventStack[0].eventId = eventId;
    _numEvents = 1;
  } else {
    bool inserted = false;
    for(int i=0; i<_numEvents; i++) {
      //Serial.println(_eventStack[i].time);
      if (_eventStack[i].time <= tEnd) {
	//Serial.println("inserting");
        for(int j=_numEvents; j>i; j--) {
          _eventStack[j].time = _eventStack[j - 1].time;
          _eventStack[j].eventId = _eventStack[j - 1].eventId;
        }
        _eventStack[i].time = tEnd;
        _eventStack[i].eventId = eventId;
        _numEvents++;
        inserted = true;
        break;
      }
    }
    if (!inserted) {
      //Serial.println("appending");
      _eventStack[_numEvents].time = tEnd;
      _eventStack[_numEvents].eventId = eventId;
      _numEvents++;
    }
  }
/*
  Serial.print("Stack:");
  for(int i=0; i<_numEvents; i++) {
    unsigned long t = _eventStack[i].time;
    Serial.print(_eventStack[i].eventId);
    Serial.print("/");
    Serial.print(t);
    Serial.print(" ");
  }
  Serial.println();*/
}

int EventManager::popEvent(bool sleep) {
  //Serial.print("Num events: ");
  //Serial.println(_numEvents);
  if (_numEvents == 0) {
    return EV_NO_EVENT;
  }
  unsigned long t = millis();
  if (t < _eventStack[_numEvents-1].time) {
    if (sleep) {
        unsigned long d = _eventStack[_numEvents-1].time - t;
        if (d > 10000) {
            delay(10000);
            return EV_NO_EVENT;
        } else {
            //Serial.print("Waiting for:");
            //Serial.println(d);
            delay(d);
        }
    } else {
        return EV_NO_EVENT;
    }
  }
  _numEvents--;
  return _eventStack[_numEvents].eventId;
}

void EventManager::removeEvents(int eventId) {
  for(int i=0; i<_numEvents; i++) {
    if (_eventStack[i].eventId == eventId) {
      if (i < _numEvents - 1) {
        for(int j=i; j<_numEvents - 1; j++) {
          _eventStack[j].eventId = _eventStack[j + 1].eventId;
          _eventStack[j].time = _eventStack[j + 1].time;
        }
      }
      _numEvents--;
    }
  }
}  
