#include "../lib/FreeRTOS-Kernel/tasks.c"

//See https://github.com/FreeRTOS/FreeRTOS-Kernel/pull/496
struct _reent* __wrap___getreent(void) {
    // No lock needed because if this changes, we won't be running anymore.
    TCB_t *pxCurTask = xTaskGetCurrentTaskHandle();
    if (pxCurTask == NULL) {
        // No task running. Return global struct.
        return _GLOBAL_REENT;
    } else {
        // We have a task; return its reentrant struct.
        return &pxCurTask->xNewLib_reent;
    }
}
