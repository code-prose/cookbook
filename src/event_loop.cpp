#include <variant>
#include "types.h"

using Time = std::chrono::sys_time<std::chrono::nanoseconds>;

struct Event {
     Time timestamp;
}


using Event = std::variant(Event, TickerEvent);
