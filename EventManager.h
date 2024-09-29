#ifndef __EventManager_h__
#define __EventManager_h__

#define EV_NO_EVENT 0

struct Event {
  unsigned long time;
  int eventId;
};

class EventManager {
  private:
  struct Event _eventStack[20];
  int _numEvents;
  
  public:
  EventManager();
  void pushEvent(int eventId, unsigned long t);
  int popEvent(bool sleep=false);
  void removeEvents(int eventId);
};

#endif
