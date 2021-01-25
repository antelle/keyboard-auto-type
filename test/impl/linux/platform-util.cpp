#include "platform-util.h"

#include <unistd.h>

namespace keyboard_auto_type_test {

void wait_millis(long ms) { usleep(ms * 1000); }

} // namespace keyboard_auto_type_test