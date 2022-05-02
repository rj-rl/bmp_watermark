#pragma once

#ifndef NDEBUG
    // disables lsan when debugging (lsan doesn't like gdb for using ptrace)
extern "C" int __lsan_is_turned_off() { return 1; }
#endif
