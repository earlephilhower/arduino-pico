/* Simple demonstration of setting and printing the current time */
/* The initial time will need to come from an RTC, NTP, or user */

/* Released to the public domain by Earle F. Philhower, III <earlephilhower@yahoo.com> */

#include <time.h>
#include <sys/time.h>

void setup() {
  Serial.begin(115200);
  delay(5000);
  struct timeval tv;

  tv.tv_sec = 1611198855; // Jan 21, 2021  3:14:15AM ...RPi Pico Release;
  tv.tv_usec = 0;
  settimeofday(&tv, nullptr);
}

void loop() {
  time_t now;
  char buff[80];

  time(&now);
  strftime(buff, sizeof(buff), "%c", localtime(&now));
  Serial.println(buff);
  delay(1000);
}
