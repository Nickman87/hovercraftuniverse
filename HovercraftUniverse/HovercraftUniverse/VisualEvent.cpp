#include "VisualEvent.h"
#include "GameEvent.h"

namespace HovUni {

VisualEvent::VisualEvent(VisualEventType type): NetworkEvent<VisualEventType, VisualEventTypeSize,4>(type)
{
}

VisualEvent::~VisualEvent(void)
{
}

}
