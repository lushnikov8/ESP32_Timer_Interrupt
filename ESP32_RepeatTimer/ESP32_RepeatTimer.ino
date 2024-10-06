/*
 Repeat timer example

 This example shows how to use hardware timer in ESP32. The timer calls onTimer
 function every second. The timer can be stopped with button attached to PIN 0
 (IO0).

 This example code is in the public domain.
 */

// Stop button is attached to PIN 0 (IO0)
//#define BTN_STOP_ALARM    0

#include <UnixTime.h>

UnixTime stamp(8); 


unsigned long CurrentTimeUnix = 0;




hw_timer_t * timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile uint32_t isrCounter = 0;
volatile uint32_t lastIsrAt = 0;


void ARDUINO_ISR_ATTR onTimer(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  isrCounter++;
  //lastIsrAt = millis();
  CurrentTimeUnix++;
  portEXIT_CRITICAL_ISR(&timerMux);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
  // It is safe to use digitalRead/Write here if you want to toggle an output
}

void setup() {
  Serial.begin(115200);
  
  StartInSetupTimerISR();
  
}

void StartInSetupTimerISR(){

  // Set BTN_STOP_ALARM to input mode
  //  pinMode(BTN_STOP_ALARM, INPUT);

  // Create semaphore to inform us when the timer has fired
  timerSemaphore = xSemaphoreCreateBinary();

  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
  // info).
  timer = timerBegin(0, 80, true);

  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer, &onTimer, true);

  // Set alarm to call onTimer function every second (value in microseconds).
  // Repeat the alarm (third parameter)
  timerAlarmWrite(timer, 1000000, true);

  // Start an alarm
  timerAlarmEnable(timer);
}


void loop() {
  
  // If Timer has fired
  if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE){
    
    uint32_t isrCount = 0, isrTime = 0;
    // Read the interrupt count and time
    portENTER_CRITICAL(&timerMux);
    
    //isrTime = lastIsrAt;
    portEXIT_CRITICAL(&timerMux);
    
    isrCount = isrCounter;


    stamp.getDateTime(CurrentTimeUnix);
    //Serial.println("CurrentTimeUnix: "+String(CurrentTimeUnix) +" "+stamp.year+"."+stamp.month+"."+stamp.day+" "+stamp.hour+":"+stamp.minute+":"+stamp.second+" day: "+stamp.dayOfWeek);

    //stamp.year, stamp.month, stamp.day
    //stamp.hour, stamp.minute, stamp.second


    // Print it
    Serial.print("onTimer no. ");
    Serial.print(isrCount);
    
    Serial.print(" CurrentTimeUnix: ");
    Serial.print(CurrentTimeUnix);
    

     
    //unsigned long MyTime = CurrentTimeUnix + 3600*24*130;
    unsigned long MyTime = CurrentTimeUnix;


    Serial.print(" MyTime: ");
    Serial.print(MyTime);
    


    Serial.print(" Время с момента включения: ");
    
    //Высчитываем целые дни
    if (MyTime/60/60/24<1) Serial.print ("0");
    Serial.print (MyTime/60/60/24);
    Serial.print (" д. ");
   
    //Высчитываем целые часы
    if (MyTime/60/60<10) Serial.print ("0");
    Serial.print ((MyTime/60/60)%24);
    Serial.print (":");
   
    //Высчитываем целые минуты, их остаток уже за вычетом целых часов.
    if (MyTime/60%60<10) Serial.print ("0");
    Serial.print ((MyTime/60)%60);
    Serial.print (":");
    
    //Высчитываем секунды.
    if (MyTime%60<10) Serial.print ("0");
    Serial.print (MyTime%60);

    Serial.println();
  }
  
  /*
  // If button is pressed
  if (digitalRead(BTN_STOP_ALARM) == LOW) {
    // If timer is still running
    if (timer) {
      // Stop and free timer
      timerEnd(timer);
      timer = NULL;
    }
  }
  */
}
