#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <IRremote.h>
int RECV_PIN = 7;
IRrecv irrecv(RECV_PIN);
decode_results results;

SemaphoreHandle_t xSemaforoMain;
void TaskMain( void *pvParameters );
void TaskJanelas( void *pvParameters );
void TaskControle( void *pvParameters );
void TaskNivelAgua( void *pvParameters );

int Janelas=0;
int Alarme=0;
int Motor=0;
int AcenderLuz=0;
int ControleLuz=0;
int HabilitaAutomatico=0;
int HabilitaAlarme=1;

void setup() {
  pinMode(2,INPUT);
  pinMode(3,INPUT);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(8,OUTPUT);
  pinMode(12,OUTPUT);
  Serial.begin(9600);
  irrecv.enableIRIn();

  if ( xSemaforoMain == NULL ) 
    {
    xSemaforoMain = xSemaphoreCreateMutex();
  if ( ( xSemaforoMain ) != NULL )
      xSemaphoreGive( ( xSemaforoMain ) );
  }

  xTaskCreate(
    TaskMain
    ,  (const portCHAR *)"TaskMain"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    TaskJanelas
    ,  (const portCHAR *)"TaskJanelas"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    TaskControle
    ,  (const portCHAR *)"TaskControle"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

   xTaskCreate(
    TaskNivelAgua
    ,  (const portCHAR *)"TaskNivelAgua"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  Serial.println("Programa Inicializado..");
}

void loop() {
  // put your main code here, to run repeatedly:
}

void TaskMain( void *pvParameters __attribute__((unused)) ){
  for(;;){
    vTaskDelay(300 / portTICK_PERIOD_MS );
    xSemaphoreTake( xSemaforoMain , portMAX_DELAY );
      if(Janelas==1 && Alarme==0 && HabilitaAlarme){
        Serial.println("Alarme!");
        Alarme=1;
      }
      if(results.value == 16738455 && Alarme==0){
        Serial.println("Habilita Alarme");
        Janelas=0;
        HabilitaAlarme=1;
      }
      if(results.value == 16738455 && Alarme==1){
        Serial.println("Desligo Alarme");
        Alarme=0;
        HabilitaAlarme=0;
      }
      if(results.value == 16724175 && Motor==0){
        Serial.println("Ligo Motor");
        digitalWrite(12,HIGH);
        Motor=1;
      }
      if(results.value == 16718055 && Motor==1){
        Serial.println("Desligo Motor");
        digitalWrite(12,LOW);
        Motor=0;
      }
      if(results.value == 16743045 && AcenderLuz==0){
        digitalWrite(5,HIGH);
        AcenderLuz=1;
        ControleLuz=1;
        HabilitaAutomatico=0;
      }
      if(results.value == 16716015 && AcenderLuz ==1){
        digitalWrite(5,LOW);
        AcenderLuz=0;
        ControleLuz=0;
        HabilitaAutomatico=0;
      }
      if(results.value == 16726215){
        HabilitaAutomatico=1;
      }
      if(HabilitaAutomatico){
        if(AcenderLuz){
          digitalWrite(5,HIGH);
        }
        else{
          digitalWrite(5,LOW);
        }
      }
    
  }
}

void TaskJanelas( void *pvParameters __attribute__((unused)) ){
  int J1;
  int J2;
  for(;;){
    vTaskDelay(100 / portTICK_PERIOD_MS );
    J1=digitalRead(3);
    J2=digitalRead(2);
    if((J1==0 || J2==0) && Janelas==0){
      xSemaphoreGive(xSemaforoMain);
      Janelas=1;
    }
  }
}

void TaskControle( void *pvParameters __attribute__((unused)) ){
  for(;;){
    if (irrecv.decode(&results)) {
      //Serial.println(results.value, DEC);
      xSemaphoreGive(xSemaforoMain);
      irrecv.resume(); // Receive the next value
    }
    vTaskDelay(20 / portTICK_PERIOD_MS );
  }
}

void TaskNivelAgua( void *pvParameters __attribute__((unused)) ){
  int agua;
  int luz;
  for(;;){
    vTaskDelay(1000 / portTICK_PERIOD_MS );
    agua=analogRead(A5);
    luz=analogRead(A0);
    if (agua<200){
      digitalWrite(4,HIGH);
      digitalWrite(8,LOW);
    }
    else{
      digitalWrite(4,LOW);
      digitalWrite(8,HIGH);
    }
    if (HabilitaAutomatico==1){
      if (luz>800 && AcenderLuz==0){
        xSemaphoreGive(xSemaforoMain);
        AcenderLuz=1;
      }
      else if(luz<=800 && AcenderLuz==1){
        xSemaphoreGive(xSemaforoMain);
        AcenderLuz=0;
      }
    }
  }
}

