#include <stdio.h>
#include <stdlib.h>
#include "diag/trace.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#define CCM_RAM __attribute__((section(".ccmram")))
TaskHandle_t sender1Handle =NULL ;
TaskHandle_t sender2Handle =NULL ;
TaskHandle_t receiverHandle =NULL ;
QueueHandle_t globalQueue =0 ;
SemaphoreHandle_t Sender_Sem1,Sender_Sem2  ;
SemaphoreHandle_t Receiver_Sem ;
TickType_t XYZ;
int blocked =0;
int total_send =0;
int total_rec =0 ;
int Tsender1[6],Tsender2[6];
char statement [100] ;
int lower[]={50,80,110,140,170,200};
int upper[]={150,200,250,300,350,400};
int i =0;
int j =0;
static TimerHandle_t timer_sender1 ;
static TimerHandle_t timer_sender2 ;
static TimerHandle_t timer_receiver ;
BaseType_t timer_sender1Started ;
BaseType_t  timer_sender2Started ;
BaseType_t  timer_receiverStarted ;
void sender1 (void* p)
{
     while(1){
    	 if(xSemaphoreTake(Sender_Sem1 ,portMAX_DELAY)){
    	 char statement [100] ;

    	 XYZ = xTaskGetTickCount();
    	 sprintf(statement ,"Time is %d",XYZ);

    	 if(xQueueSend(globalQueue, &statement, 0)==pdPASS)
    	  total_send++;

    	     else
    	       blocked++;
     }
 }
}
void sender2 (void* p)
{
     while(1){
    	 if(xSemaphoreTake(Sender_Sem2 ,portMAX_DELAY)){

    	 XYZ = xTaskGetTickCount();
    	 sprintf(statement ,"Time is %d",XYZ);

    	 if(xQueueSend(globalQueue, &statement, 0)==pdPASS)
    	  total_send++;

    	     else
    	       blocked++;
     }
 }
}
void receiver (void* p)
{
     while(1){
    	if(xSemaphoreTake( Receiver_Sem, portMAX_DELAY )){

    	if(uxQueueMessagesWaiting(globalQueue)!=0)    //checks for messages
    			{
    				xQueueReceive(globalQueue, &statement, 0);//read the queue
    				sprintf ("%s\r\n",statement);
    				total_rec++;
    			}
             }
        }
}

/*-----------------------------------------------------------*/
static void sender1_timer_callback (TimerHandle_t xTimer){
	xSemaphoreGive(Sender_Sem1);
}
static void sender2_timer_callback (TimerHandle_t xTimer){
	xSemaphoreGive(Sender_Sem2);
}

static void Receiver_callback (TimerHandle_t xTimer){
	xSemaphoreGive(Receiver_Sem);
	if( total_rec == 500){
        Reset();
	}
}
void random_period(){
 {
          Tsender1[i]= (rand() % (upper[i] - lower[i] + 1)) + lower[i];
          Tsender2[i]= (rand() % (upper[i] - lower[i] + 1)) + lower[i];
       }
 ++i;
}
void Reset()
{
	    if (total_rec == 500){
	    printf ("Total number of sent message = %d \r\n" , total_send);
	    printf ("Total number of RE message = %d \r\n" , total_rec);
		printf ("Total number of Blocked message = %d \r\n" ,blocked);
		blocked =0;    // Reset all the counters
		total_send =0;
		total_rec =0 ;
		xQueueReset( globalQueue );  //Clear Queue
	    }

		if( j >5 )// If we used all values in the array,destroy the timers and print a message “Game Over” and stop execution
			{
				xTimerDelete(timer_sender1, 0); // Destroy the Timers
			 	xTimerDelete(timer_sender2, 0);
			 	xTimerDelete(timer_receiver, 0);
			 	printf("Game Over\r\n");
			 	vTaskEndScheduler();
			 	return;
			}
		random_period();
				xTimerChangePeriod(timer_sender1,pdMS_TO_TICKS(Tsender1[j]),(TickType_t) 0);
				xTimerChangePeriod(timer_sender2,pdMS_TO_TICKS(Tsender2[j]),(TickType_t) 0);
				printf("Tsender1 is %d\r\n",Tsender1[j]);
				printf("Tsender2 is %d\r\n",Tsender2[j]);
				j++;
}

// ----- main() ---------------------------------------------------------------
// Sample pragmas to cope with warnings. Please note the related line at the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

int main(int argc, char* argv[])
{
	 srand(time(NULL));
	 globalQueue = xQueueCreate (2, 20* sizeof (char));
	 Receiver_Sem = xSemaphoreCreateBinary ( );
	 Sender_Sem1 = xSemaphoreCreateBinary ( );
	 Sender_Sem2 = xSemaphoreCreateBinary ( );

	 if( globalQueue != NULL ){
	 xTaskCreate (sender1 ,"TSender1" ,1024, (void*) 0, 0 , &sender1Handle );
	 xTaskCreate (sender2 ,"TSender2" ,1024, (void*) 0, 0 , &sender2Handle);
	 xTaskCreate (receiver , "TReceiver" ,1024 , (void*) 0, 1 , &receiverHandle);
	 }
	 timer_sender1 = xTimerCreate( "Timer_sender1", ( pdMS_TO_TICKS(1000) ), pdTRUE, ( void * ) 0, sender1_timer_callback);
	 timer_sender2 = xTimerCreate( "Timer_sender2", ( pdMS_TO_TICKS(1000) ), pdTRUE, ( void * ) 0, sender2_timer_callback);
	 timer_receiver = xTimerCreate( "Timer_receiver", ( pdMS_TO_TICKS(100) ), pdTRUE, ( void * ) 0, Receiver_callback);
	 Reset();
	if( ( timer_sender1 != NULL ) && ( timer_sender2 != NULL ) && ( timer_receiver != NULL ) )
	{
		timer_sender1Started = xTimerStart( timer_sender1, 0 );
		timer_sender2Started = xTimerStart( timer_sender2, 0 );
		timer_receiverStarted = xTimerStart( timer_receiver, 0 );
	}

	if( timer_sender1Started == pdPASS && timer_sender2Started == pdPASS && timer_receiverStarted == pdPASS)
	{
		vTaskStartScheduler();
	}

	return 0;
}



// ----------------------------------------------------

void vApplicationMallocFailedHook( void )
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
volatile size_t xFreeStackSpace;

	/* This function is called on each cycle of the idle task.  In this case it
	does nothing useful, other than report the amout of FreeRTOS heap that
	remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if( xFreeStackSpace > 100 )
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}

void vApplicationTickHook(void) {
}

StaticTask_t xIdleTaskTCB CCM_RAM;
StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE] CCM_RAM;

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
  /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
  state will be stored. */
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

  /* Pass out the array that will be used as the Idle task's stack. */
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;

  /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
  Note that, as the array is necessarily of type StackType_t,
  configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

static StaticTask_t xTimerTaskTCB CCM_RAM;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH] CCM_RAM;

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

