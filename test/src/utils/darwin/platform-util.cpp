#include "utils/platform-util.h"

#include <Carbon/Carbon.h>

namespace keyboard_auto_type_test {

void wait_millis(long ms) { CFRunLoopRunInMode(kCFRunLoopDefaultMode, ms / 1000., false); }

} // namespace keyboard_auto_type_test