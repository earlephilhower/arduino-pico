// Simple stress test to ensure each thread has its own Newlib reent structure
// The random numbers from each task should be identical.

#include <FreeRTOS.h>
#include <task.h>

void go(void *param) {
  (void) param;
  srand(0);
  int i = 0;
  while(1) {
    char buff[100];
    TaskStatus_t st;
    vTaskGetInfo(NULL, &st, pdFALSE, eInvalid);
    sprintf(buff, "task %ld: %d = %d\n", st.xTaskNumber, i++, rand());
    Serial.print(buff);
    delay(1000);
  }
}

void setup() {
  delay(5000);
  // put your setup code here, to run once:
  xTaskCreate(go, "c1", 1024, nullptr, 1, nullptr);
  xTaskCreate(go, "c2", 1024, nullptr, 1, nullptr);
  xTaskCreate(go, "c3", 1024, nullptr, 1, nullptr);
}

void loop() {
  // put your main code here, to run repeatedly:

}
