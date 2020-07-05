#ifndef EventBaseObjectH
#define EventBaseObjectH

#include "EventSocketCommon.h"

class EventBaseObject
{
public:
  EventBaseObject();

  EventBasePtr get() const;

private:
  EventBasePtr _base;
};

#endif // EVENTBASEOBJECT_H
