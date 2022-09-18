//#include "stm32f10x.h"                  // Device header
#include "string.h"
#include <stdio.h>

#include "bps_led.h"
#include "bps_usart.h"
#include "key.h"

#include "FreeRTOS.h"
#include "task.h"
#include "limits.h"

/**************************** ȫ�ֱ��� ********************************/
uint32_t r_data1=0;
//uint32_t r_data2=0;
char *r_data2="";

/**************************** ������ ********************************/
/* 
 * ��������һ��ָ�룬����ָ��һ�����񣬵����񴴽���֮�����;�����һ��������
 * �Ժ�����Ҫ��������������Ҫͨ�������������������������������Լ�����ô
 * ����������ΪNULL��
 */
 /* ���������� */
static TaskHandle_t AppTaskCreate_Handle = NULL;
static TaskHandle_t Receive1_Task_Handle = NULL;/*Receive1_Task ������ */
static TaskHandle_t Receive2_Task_Handle = NULL;/*Receive2_Task ������ */
static TaskHandle_t Send_Task_Handle = NULL;/* Send_Task ������ */







//��������
static void Receive1_Task(void* parameter);
static void Receive2_Task(void* parameter);
static void Send_Task(void* parameter);
static void AppTaskCreate(void);

static void BSP_Init(void)
{
	/* 
	* STM32 �ж����ȼ�����Ϊ 4���� 4bit ��������ʾ��ռ���ȼ�����ΧΪ��0~15 
	* ���ȼ�����ֻ��Ҫ����һ�μ��ɣ��Ժ������������������Ҫ�õ��жϣ� 
	* ��ͳһ��������ȼ����飬ǧ��Ҫ�ٷ��飬�мɡ� 
	*/ 
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 
	LED_GPIO_Config();
	KEY1_GPIO_Config();
	KEY2_GPIO_Config();
	USART_Config();
	
	//����
//	led_G(on);
//	printf("���ڲ���");
}

int main()
{
	BaseType_t xReturn = NULL;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
	
	BSP_Init();
	printf("����һ��[Ұ��]-STM32 ȫϵ�п�����-FreeRTOS ����֪ͨ������Ϣ����ʵ�飡\n");
	printf("���� KEY1 ���� KEY2 ����������Ϣ֪ͨ\n");
	printf("����KEY1���Ա�������   ����KEY2�����ַ�������\n");
	

	
	  /* ����AppTaskCreate���� */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* ������ں��� */
                        (const char*    )"AppTaskCreate",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )1, /* ��������ȼ� */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* ������ƿ�ָ�� */ 
																							
	if(xReturn==pdPASS)
	{
		printf("��ʼ���񴴽��ɹ�\r\n");
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
	BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��Ϊ pdPASS */ 
	while(1)
	{
		xReturn=xTaskNotifyWait(0,              //���ú���ǰֵ֪ͨ����ĸ�λ������ѡ�������
		                        ULONG_MAX,      //�˳�������ʱ��������е� bit(���ǰ��ֵ���浽r_data1��) 
		                        &r_data1,       //��������ֵ֪ͨ
		                        portMAX_DELAY); //����ʱ��
		if(xReturn==pdTRUE)
		{
			printf("Receive1_Task ����֪ͨΪ %d \n",r_data1);
			LED_G_TOGGLE();
		}
	}
}



static void Receive2_Task(void* parameter)
{
	
	BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��Ϊ pdPASS */ 
	while(1)
	{
		xReturn=xTaskNotifyWait(0,              //���ú���ǰֵ֪ͨ����ĸ�λ������ѡ�������
		                        ULONG_MAX,      //�˳�������ʱ��������е� bit 
		                        (uint32_t *)&r_data2,       //��������ֵ֪ͨ
		                        portMAX_DELAY); //����ʱ��
		if(xReturn==pdTRUE)
		{
			printf("Receive2_Task ����֪ͨΪ %s \n",r_data2);
			LED_R_TOGGLE();
		}
	}    
}


static void Send_Task(void* parameter)
{
	uint32_t send1 = 1; 
//	uint32_t send2 = 2; 
	char test_str2[] = "this is a mail test 2";/* ��Ϣ test2 */
	BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��Ϊ pdPASS */ 
	while(1)
	{
		if(key_scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN)==1)
		{
			xReturn = xTaskNotify(Receive1_Task_Handle,//������
			                      send1,               //Ҫ���͵����ݣ�4�ֽڣ�
			                      eSetValueWithOverwrite);//���ǵ�ǰֵ֪ͨ
			if(xReturn==pdPASS)
			{
				printf("Receive1_Task_Handle ����֪ͨ�ͷųɹ�!\r\n"); 
			}
		}
		if(key_scan(KEY2_GPIO_PORT,KEY2_GPIO_PIN)==1)
		{
			xReturn = xTaskNotify(Receive2_Task_Handle,//������
			                      (uint32_t)&test_str2,//Ҫ���͵����ݣ�4�ֽڣ�
			                      eSetValueWithOverwrite);//���ǵ�ǰֵ֪ͨ
			if(xReturn==pdPASS)
			{
				printf("Receive1_Task_Handle ����֪ͨ�ͷųɹ�!\r\n"); 
			}
		}
		vTaskDelay(20); 
	}    
}



static void AppTaskCreate(void)
{
	BaseType_t xReturn = NULL;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
	
	taskENTER_CRITICAL();           //�����ٽ���
	
	xReturn=xTaskCreate((TaskFunction_t	)Receive1_Task,		//������
															(const char* 	)"Receive1_Task",		//��������
															(uint16_t 		)512,	//�����ջ��С
															(void* 		  	)NULL,				//���ݸ��������Ĳ���
															(UBaseType_t 	)3, 	//�������ȼ�
															(TaskHandle_t*  )&Receive1_Task_Handle);/* ������ƿ�ָ�� */ 
															
															
	if(xReturn == pdPASS)/* �����ɹ� */
		printf("Receive1_Task���񴴽��ɹ�!\n");
	else
		printf("Receive1_Task���񴴽�ʧ��!\n");
	
	xReturn=xTaskCreate((TaskFunction_t	)Receive2_Task,		//������
															(const char* 	)"Receive2_Task",		//��������
															(uint16_t 		)512,	//�����ջ��С
															(void* 		  	)NULL,				//���ݸ��������Ĳ���
															(UBaseType_t 	)3, 	//�������ȼ�
															(TaskHandle_t*  )&Receive2_Task_Handle);/* ������ƿ�ָ�� */ 
															
	if(xReturn == pdPASS)/* �����ɹ� */
		printf("Receive2_Task���񴴽��ɹ�!\n");
	else
		printf("Receive2_Task���񴴽�ʧ��!\n");
	
	
		xReturn=xTaskCreate((TaskFunction_t	)Send_Task,		//������
															(const char* 	)"Send_Task",		//��������
															(uint16_t 		)512,	//�����ջ��С
															(void* 		  	)NULL,				//���ݸ��������Ĳ���
															(UBaseType_t 	)4, 	//�������ȼ�
															(TaskHandle_t*  )&Send_Task_Handle);/* ������ƿ�ָ�� */ 
															
	if(xReturn == pdPASS)/* �����ɹ� */
		printf("Send_Task���񴴽��ɹ�!\n");
	else
		printf("Send_Task���񴴽�ʧ��!\n");
	
	
	
	vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����
	
	taskEXIT_CRITICAL();            //�˳��ٽ���
}


//��̬�����������Ҫ
///**
//  **********************************************************************
//  * @brief  ��ȡ��������������ջ��������ƿ��ڴ�
//	*					ppxTimerTaskTCBBuffer	:		������ƿ��ڴ�
//	*					ppxTimerTaskStackBuffer	:	�����ջ�ڴ�
//	*					pulTimerTaskStackSize	:		�����ջ��С
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, 
//								   StackType_t **ppxIdleTaskStackBuffer, 
//								   uint32_t *pulIdleTaskStackSize)
//{
//	*ppxIdleTaskTCBBuffer=&Idle_Task_TCB;/* ������ƿ��ڴ� */
//	*ppxIdleTaskStackBuffer=Idle_Task_Stack;/* �����ջ�ڴ� */
//	*pulIdleTaskStackSize=configMINIMAL_STACK_SIZE;/* �����ջ��С */
//}



///**
//  *********************************************************************
//  * @brief  ��ȡ��ʱ������������ջ��������ƿ��ڴ�
//	*					ppxTimerTaskTCBBuffer	:		������ƿ��ڴ�
//	*					ppxTimerTaskStackBuffer	:	�����ջ�ڴ�
//	*					pulTimerTaskStackSize	:		�����ջ��С
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
//									StackType_t **ppxTimerTaskStackBuffer, 
//									uint32_t *pulTimerTaskStackSize)
//{
//	*ppxTimerTaskTCBBuffer=&Timer_Task_TCB;/* ������ƿ��ڴ� */
//	*ppxTimerTaskStackBuffer=Timer_Task_Stack;/* �����ջ�ڴ� */
//	*pulTimerTaskStackSize=configTIMER_TASK_STACK_DEPTH;/* �����ջ��С */
//}
