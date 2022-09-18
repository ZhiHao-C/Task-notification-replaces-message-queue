//#include "stm32f10x.h"                  // Device header
#include "string.h"
#include <stdio.h>

#include "bps_led.h"
#include "bps_usart.h"
#include "key.h"

#include "FreeRTOS.h"
#include "task.h"
#include "limits.h"

/**************************** 全局变量 ********************************/
uint32_t r_data1=0;
//uint32_t r_data2=0;
char *r_data2="";

/**************************** 任务句柄 ********************************/
/* 
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */
 /* 创建任务句柄 */
static TaskHandle_t AppTaskCreate_Handle = NULL;
static TaskHandle_t Receive1_Task_Handle = NULL;/*Receive1_Task 任务句柄 */
static TaskHandle_t Receive2_Task_Handle = NULL;/*Receive2_Task 任务句柄 */
static TaskHandle_t Send_Task_Handle = NULL;/* Send_Task 任务句柄 */







//声明函数
static void Receive1_Task(void* parameter);
static void Receive2_Task(void* parameter);
static void Send_Task(void* parameter);
static void AppTaskCreate(void);

static void BSP_Init(void)
{
	/* 
	* STM32 中断优先级分组为 4，即 4bit 都用来表示抢占优先级，范围为：0~15 
	* 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断， 
	* 都统一用这个优先级分组，千万不要再分组，切忌。 
	*/ 
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 
	LED_GPIO_Config();
	KEY1_GPIO_Config();
	KEY2_GPIO_Config();
	USART_Config();
	
	//测试
//	led_G(on);
//	printf("串口测试");
}

int main()
{
	BaseType_t xReturn = NULL;/* 定义一个创建信息返回值，默认为pdPASS */
	
	BSP_Init();
	printf("这是一个[野火]-STM32 全系列开发板-FreeRTOS 任务通知代替消息队列实验！\n");
	printf("按下 KEY1 或者 KEY2 向任务发送消息通知\n");
	printf("按下KEY1测试变量接收   按下KEY2测试字符串接收\n");
	

	
	  /* 创建AppTaskCreate任务 */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* 任务入口函数 */
                        (const char*    )"AppTaskCreate",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )1, /* 任务的优先级 */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */ 
																							
	if(xReturn==pdPASS)
	{
		printf("初始任务创建成功\r\n");
		vTaskStartScheduler();
	}
	else 
	{
		return -1;
	}
	while(1)
	{
		
	}

}



static void Receive1_Task(void* parameter)
{
	BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */ 
	while(1)
	{
		xReturn=xTaskNotifyWait(0,              //调用函数前通知值清除哪个位（这里选择不清除）
		                        ULONG_MAX,      //退出函数的时候清除所有的 bit(清除前将值保存到r_data1中) 
		                        &r_data1,       //保存任务通知值
		                        portMAX_DELAY); //阻塞时间
		if(xReturn==pdTRUE)
		{
			printf("Receive1_Task 任务通知为 %d \n",r_data1);
			LED_G_TOGGLE();
		}
	}
}



static void Receive2_Task(void* parameter)
{
	
	BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */ 
	while(1)
	{
		xReturn=xTaskNotifyWait(0,              //调用函数前通知值清除哪个位（这里选择不清除）
		                        ULONG_MAX,      //退出函数的时候清除所有的 bit 
		                        (uint32_t *)&r_data2,       //保存任务通知值
		                        portMAX_DELAY); //阻塞时间
		if(xReturn==pdTRUE)
		{
			printf("Receive2_Task 任务通知为 %s \n",r_data2);
			LED_R_TOGGLE();
		}
	}    
}


static void Send_Task(void* parameter)
{
	uint32_t send1 = 1; 
//	uint32_t send2 = 2; 
	char test_str2[] = "this is a mail test 2";/* 消息 test2 */
	BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为 pdPASS */ 
	while(1)
	{
		if(key_scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN)==1)
		{
			xReturn = xTaskNotify(Receive1_Task_Handle,//任务句柄
			                      send1,               //要发送的数据（4字节）
			                      eSetValueWithOverwrite);//覆盖当前通知值
			if(xReturn==pdPASS)
			{
				printf("Receive1_Task_Handle 任务通知释放成功!\r\n"); 
			}
		}
		if(key_scan(KEY2_GPIO_PORT,KEY2_GPIO_PIN)==1)
		{
			xReturn = xTaskNotify(Receive2_Task_Handle,//任务句柄
			                      (uint32_t)&test_str2,//要发送的数据（4字节）
			                      eSetValueWithOverwrite);//覆盖当前通知值
			if(xReturn==pdPASS)
			{
				printf("Receive1_Task_Handle 任务通知释放成功!\r\n"); 
			}
		}
		vTaskDelay(20); 
	}    
}



static void AppTaskCreate(void)
{
	BaseType_t xReturn = NULL;/* 定义一个创建信息返回值，默认为pdPASS */
	
	taskENTER_CRITICAL();           //进入临界区
	
	xReturn=xTaskCreate((TaskFunction_t	)Receive1_Task,		//任务函数
															(const char* 	)"Receive1_Task",		//任务名称
															(uint16_t 		)512,	//任务堆栈大小
															(void* 		  	)NULL,				//传递给任务函数的参数
															(UBaseType_t 	)3, 	//任务优先级
															(TaskHandle_t*  )&Receive1_Task_Handle);/* 任务控制块指针 */ 
															
															
	if(xReturn == pdPASS)/* 创建成功 */
		printf("Receive1_Task任务创建成功!\n");
	else
		printf("Receive1_Task任务创建失败!\n");
	
	xReturn=xTaskCreate((TaskFunction_t	)Receive2_Task,		//任务函数
															(const char* 	)"Receive2_Task",		//任务名称
															(uint16_t 		)512,	//任务堆栈大小
															(void* 		  	)NULL,				//传递给任务函数的参数
															(UBaseType_t 	)3, 	//任务优先级
															(TaskHandle_t*  )&Receive2_Task_Handle);/* 任务控制块指针 */ 
															
	if(xReturn == pdPASS)/* 创建成功 */
		printf("Receive2_Task任务创建成功!\n");
	else
		printf("Receive2_Task任务创建失败!\n");
	
	
		xReturn=xTaskCreate((TaskFunction_t	)Send_Task,		//任务函数
															(const char* 	)"Send_Task",		//任务名称
															(uint16_t 		)512,	//任务堆栈大小
															(void* 		  	)NULL,				//传递给任务函数的参数
															(UBaseType_t 	)4, 	//任务优先级
															(TaskHandle_t*  )&Send_Task_Handle);/* 任务控制块指针 */ 
															
	if(xReturn == pdPASS)/* 创建成功 */
		printf("Send_Task任务创建成功!\n");
	else
		printf("Send_Task任务创建失败!\n");
	
	
	
	vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
	
	taskEXIT_CRITICAL();            //退出临界区
}


//静态创建任务才需要
///**
//  **********************************************************************
//  * @brief  获取空闲任务的任务堆栈和任务控制块内存
//	*					ppxTimerTaskTCBBuffer	:		任务控制块内存
//	*					ppxTimerTaskStackBuffer	:	任务堆栈内存
//	*					pulTimerTaskStackSize	:		任务堆栈大小
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, 
//								   StackType_t **ppxIdleTaskStackBuffer, 
//								   uint32_t *pulIdleTaskStackSize)
//{
//	*ppxIdleTaskTCBBuffer=&Idle_Task_TCB;/* 任务控制块内存 */
//	*ppxIdleTaskStackBuffer=Idle_Task_Stack;/* 任务堆栈内存 */
//	*pulIdleTaskStackSize=configMINIMAL_STACK_SIZE;/* 任务堆栈大小 */
//}



///**
//  *********************************************************************
//  * @brief  获取定时器任务的任务堆栈和任务控制块内存
//	*					ppxTimerTaskTCBBuffer	:		任务控制块内存
//	*					ppxTimerTaskStackBuffer	:	任务堆栈内存
//	*					pulTimerTaskStackSize	:		任务堆栈大小
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
//									StackType_t **ppxTimerTaskStackBuffer, 
//									uint32_t *pulTimerTaskStackSize)
//{
//	*ppxTimerTaskTCBBuffer=&Timer_Task_TCB;/* 任务控制块内存 */
//	*ppxTimerTaskStackBuffer=Timer_Task_Stack;/* 任务堆栈内存 */
//	*pulTimerTaskStackSize=configTIMER_TASK_STACK_DEPTH;/* 任务堆栈大小 */
//}
