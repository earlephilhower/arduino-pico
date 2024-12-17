// Released to the public domain
#include <FreeRTOS.h>
#include <task.h>
#include <map>
#define STACK_SIZE 512
#define CORE_0 (1 << 0)
#define CORE_1 (1 << 1)

std::map<eTaskState, const char *> eTaskStateName { {eReady, "Ready"}, { eRunning, "Running" }, {eBlocked, "Blocked"}, {eSuspended, "Suspended"}, {eDeleted, "Deleted"} };
void ps() {
  int tasks = uxTaskGetNumberOfTasks();
  TaskStatus_t *pxTaskStatusArray = new TaskStatus_t[tasks];
  unsigned long runtime;
  tasks = uxTaskGetSystemState( pxTaskStatusArray, tasks, &runtime );
  Serial.printf("# Tasks: %d\n", tasks);
  Serial.printf("%-3s %-16s %-10s %s %s\n", "ID", "NAME", "STATE", "PRIO", "CYCLES");
  for (int i = 0; i < tasks; i++) {
    Serial.printf("%2d: %-16s %-10s %4d %lu\n", i, pxTaskStatusArray[i].pcTaskName, eTaskStateName[pxTaskStatusArray[i].eCurrentState], (int)pxTaskStatusArray[i].uxCurrentPriority, pxTaskStatusArray[i].ulRunTimeCounter);
  }
  delete[] pxTaskStatusArray;
}

static TaskHandle_t l[16];

void loop() {
  ps();
  delay(1000);
}

#define LOOP(z) \
void loop##z(void *params) {\
  (void) params;\
  while (true) {\
    srand(z);\
    int sum = 0;\
    for (int i = 0; i < 500000; i++) sum+= rand();\
    Serial.printf("L%d: %08x\n", z, sum);\
    delay(1000 + z * 10);\
  }\
}

LOOP(0);
LOOP(1);
LOOP(2);
LOOP(3);


void setup() {
  xTaskCreate(loop0, "loop0", STACK_SIZE, NULL, 1, &l[0]);
  vTaskCoreAffinitySet(l[0], CORE_0);
//  xTaskCreate(loop1, "loop1", STACK_SIZE, NULL, 1, &l[1]);
//  vTaskCoreAffinitySet(l[1], CORE_0);
//  xTaskCreate(loop2, "loop2", STACK_SIZE, NULL, 1, &l[2]);
//  vTaskCoreAffinitySet(l[2], CORE_0);
//  xTaskCreate(loop3, "loop3", STACK_SIZE, NULL, 1, &l[3]);
//  vTaskCoreAffinitySet(l[3], CORE_1);
}
