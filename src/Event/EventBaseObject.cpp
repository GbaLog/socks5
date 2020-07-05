#include "EventBaseObject.h"

EventBaseObject::EventBaseObject() :
  _base(event_base_new(), event_base_free)
{}

EventBasePtr EventBaseObject::get() const
{
  return _base;
}

