#pragma once

extern bool isControlMainFinished;

// This is called from the main thread (as it should be)
void control_main_wait_for_ready();

// This runs in its own thread
int control_main(int argc, const char *argv[]);
