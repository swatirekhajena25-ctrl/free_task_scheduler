#include <stdio.h>

/* FreeRTOS kernel includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Console setup used by the POSIX demo port */
#include "console.h"

#define QUEUE_LENGTH        5
#define PRODUCER_PRIORITY   ( tskIDLE_PRIORITY + 1 )
#define CONSUMER_PRIORITY   ( tskIDLE_PRIORITY + 1 )
#define LOGGER_PRIORITY      ( tskIDLE_PRIORITY + 2 )   /* higher priority */

static QueueHandle_t xQueue;
static SemaphoreHandle_t xPrintMutex;

static void safePrint( const char * msg )
{
    xSemaphoreTake( xPrintMutex, portMAX_DELAY );
    printf( "%s", msg );
    fflush( stdout );
    xSemaphoreGive( xPrintMutex );
}

static void vProducerTask( void * pvParameters )
{
    int counter = 0;
    char buf[ 64 ];

    for( ;; )
    {
        counter++;
        snprintf( buf, sizeof( buf ), "[Producer] sending value: %d\n", counter );
        safePrint( buf );

        xQueueSend( xQueue, &counter, portMAX_DELAY );

        vTaskDelay( pdMS_TO_TICKS( 1000 ) );
    }
}

static void vConsumerTask( void * pvParameters )
{
    int received;
    char buf[ 64 ];

    for( ;; )
    {
        if( xQueueReceive( xQueue, &received, portMAX_DELAY ) == pdPASS )
        {
            snprintf( buf, sizeof( buf ), "[Consumer] received value: %d\n", received );
            safePrint( buf );
        }
    }
}

static void vLoggerTask( void * pvParameters )
{
    int heartbeat = 0;
    char buf[ 64 ];

    for( ;; )
    {
        heartbeat++;
        snprintf( buf, sizeof( buf ), "[Logger] heartbeat #%d (high priority)\n", heartbeat );
        safePrint( buf );

        vTaskDelay( pdMS_TO_TICKS( 2000 ) );
    }
}

int main( void )
{
    console_init();

    xQueue = xQueueCreate( QUEUE_LENGTH, sizeof( int ) );
    xPrintMutex = xSemaphoreCreateMutex();

    xTaskCreate( vProducerTask, "Producer", configMINIMAL_STACK_SIZE, NULL, PRODUCER_PRIORITY, NULL );
    xTaskCreate( vConsumerTask, "Consumer", configMINIMAL_STACK_SIZE, NULL, CONSUMER_PRIORITY, NULL );
    xTaskCreate( vLoggerTask, "Logger", configMINIMAL_STACK_SIZE, NULL, LOGGER_PRIORITY, NULL );

    vTaskStartScheduler();

    /* Should never reach here */
    for( ;; );
    return 0;
}

/* --- Required FreeRTOS hook functions --- */

void vApplicationMallocFailedHook( void )
{
    printf( "Malloc failed!\n" );
    for( ;; );
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName )
{
    ( void ) xTask;
    printf( "Stack overflow in task: %s\n", pcTaskName );
    for( ;; );
}

void vApplicationIdleHook( void )
{
}

void vApplicationTickHook( void )
{
}

void vAssertCalled( const char * pcFile, unsigned long ulLine )
{
    printf( "ASSERT FAILED! File: %s, Line: %lu\n", pcFile, ulLine );
    fflush( stdout );
    for( ;; );
}
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                     StackType_t ** ppxIdleTaskStackBuffer,
                                     uint32_t * pulIdleTaskStackSize )
{
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                      StackType_t ** ppxTimerTaskStackBuffer,
                                      uint32_t * pulTimerTaskStackSize )
{
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void vApplicationDaemonTaskStartupHook( void )
{
}
