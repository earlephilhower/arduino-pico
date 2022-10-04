/* The code in this example is mostly derived from the official FreeRTOS
 * code examples.
 * 
 * For more information on static allocation and to read the original
 * code visit the following links:
 * https://www.freertos.org/Static_Vs_Dynamic_Memory_Allocation.html
 * https://www.freertos.org/xTaskCreateStatic.html
 * https://www.freertos.org/xSemaphoreCreateMutexStatic.html
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#define SERIAL_PORT Serial1
#define BLINK_ON_TIME   250
#define BLINK_OFF_TIME  500

/* Dimensions of the buffer that the task being created will use as its stack.
  NOTE:  This is the number of words the stack will hold, not the number of
  bytes.  For example, if each stack item is 32-bits, and this is set to 100,
  then 400 bytes (100 * 32-bits) will be allocated. */
#define STACK_SIZE 200

/* Structure that will hold the TCB of the task being created. */
StaticTask_t xTaskBuffer_A;
StaticTask_t xTaskBuffer_B;

/* Buffer that the task being created will use as its stack.  Note this is
  an array of StackType_t variables.  The size of StackType_t is dependent on
  the RTOS port. */
StackType_t xStack_A[ STACK_SIZE ];
StackType_t xStack_B[ STACK_SIZE ];

SemaphoreHandle_t xSemaphore = NULL;
StaticSemaphore_t xMutexBuffer;

void setup() {
  SERIAL_PORT.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  /* Create a mutex semaphore without using any dynamic memory
    allocation.  The mutex's data structures will be saved into
    the xMutexBuffer variable. */
  xSemaphore = xSemaphoreCreateMutexStatic( &xMutexBuffer );

  xTaskCreateStatic(led_ON, "led_ON", STACK_SIZE, NULL, configMAX_PRIORITIES - 1, xStack_A, &xTaskBuffer_A);
  xTaskCreateStatic(led_OFF, "led_OFF", STACK_SIZE, NULL, configMAX_PRIORITIES - 1, xStack_B, &xTaskBuffer_B);
}

void led_ON(void *pvParameters)
{
  (void) pvParameters;
  while (1)
  {
    xSemaphoreTake( xSemaphore, ( TickType_t ) portMAX_DELAY );
    SERIAL_PORT.println("LED ON!");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(BLINK_ON_TIME);
    xSemaphoreGive( xSemaphore );
  }
}

void led_OFF(void *pvParameters)
{
  (void) pvParameters;
  while (1)
  {
    xSemaphoreTake( xSemaphore, ( TickType_t ) portMAX_DELAY );
    SERIAL_PORT.println("LED OFF!");
    digitalWrite(LED_BUILTIN, LOW);
    delay(BLINK_OFF_TIME);
    xSemaphoreGive( xSemaphore );
  }
}

void loop() {
  SERIAL_PORT.println("Hello!");
  delay(1000);
}
