// Demonstrates a simple use of the setup1()/loop1() functions
// for a multiprocessor run.

// Will output something like, where C0 is running on core 0 and
// C1 is on core 1, in parallel.

// 11:23:07.507 -> C0: Blue leader standing by...
// 11:23:07.507 -> C1: Red leader standing by...
// 11:23:07.507 -> C1: Stay on target...
// 11:23:08.008 -> C1: Stay on target...
// 11:23:08.505 -> C0: Blue leader standing by...
// 11:23:08.505 -> C1: Stay on target...
// 11:23:09.007 -> C1: Stay on target...
// 11:23:09.511 -> C0: Blue leader standing by...
// 11:23:09.511 -> C1: Stay on target...
// 11:23:10.015 -> C1: Stay on target...

// Released to the public domain
#include <FreeRTOS.h>
#include <task.h>
#include <map>
#include <EEPROM.h>

std::map<eTaskState, const char *> eTaskStateName { {eReady, "Ready"}, { eRunning, "Running" }, {eBlocked, "Blocked"}, {eSuspended, "Suspended"}, {eDeleted, "Deleted"} };
void ps() {
  int tasks = uxTaskGetNumberOfTasks();
  TaskStatus_t *pxTaskStatusArray = new TaskStatus_t[tasks];
  unsigned long runtime;
  tasks = uxTaskGetSystemState( pxTaskStatusArray, tasks, &runtime );
  Serial.printf("# Tasks: %d\n", tasks);
  Serial.println("ID, NAME, STATE, PRIO, CYCLES");
  for (int i=0; i < tasks; i++) {
    Serial.printf("%d: %-16s %-10s %d %lu\n", i, pxTaskStatusArray[i].pcTaskName, eTaskStateName[pxTaskStatusArray[i].eCurrentState], pxTaskStatusArray[i].uxCurrentPriority, pxTaskStatusArray[i].ulRunTimeCounter);
  }
  delete[] pxTaskStatusArray;
}


void blink(void *param) {
  (void) param;
  pinMode(LED_BUILTIN, OUTPUT);
  while (true) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(750);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
  }
}


void setup() {
  Serial.begin(115200);
  xTaskCreate(blink, "BLINK", 128, nullptr, 1, nullptr);
  delay(5000);
}

volatile int val= 0;
void loop() {
  Serial.printf("C0: Blue leader standing by...\n");
  ps();
  Serial.printf("val: %d\n", val);
  delay(1000);
}

// Running on core1
void setup1() {
  delay(5000);
  Serial.printf("C1: Red leader standing by...\n");
}

void loop1() {
  static int x = 0;
  Serial.printf("C1: Stay on target...\n");
  val++;
  if (++x < 10) {
    EEPROM.begin(512);
    EEPROM.write(0,x);
    EEPROM.commit();
  }
  delay(1000);
}
