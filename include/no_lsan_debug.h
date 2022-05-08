#pragma once

#ifndef NDEBUG

// disables lsan when debugging
// (lsan doesn't like gdb for its use of ptrace)
extern "C" int __lsan_is_turned_off() { return 1; }

#endif
