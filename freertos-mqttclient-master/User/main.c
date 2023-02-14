#include "main.h"
/* FreeRTOSͷ�ļ� */
#include "FreeRTOS.h"
#include "task.h"

#include "mqttclient.h"

//#define TEST_USEING_TLS  

extern const char *test_ca_get(void);

static TaskHandle_t app_task_create_handle = NULL;/* ���������� */
static TaskHandle_t mqtt_task_handle = NULL;/* LED������ */

static void app_task_create(void);/* ���ڴ������� */

static void mqtt_task(void* pvParameters);/* mqtt_task����ʵ�� */

extern void TCPIP_Init(void);

/*****************************************************************
  * @brief  ������
  * @param  ��
  * @retval ��
  * @note   ��һ����������Ӳ����ʼ�� 
            �ڶ���������APPӦ������
            ������������FreeRTOS����ʼ���������
  ****************************************************************/
int main(void)
{	
  BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
  
  /* ������Ӳ����ʼ�� */
  BSP_Init();

  /* ����app_task_create���� */
  xReturn = xTaskCreate((TaskFunction_t )app_task_create,  /* ������ں��� */
                        (const char*    )"app_task_create",/* �������� */
                        (uint16_t       )2048,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )10, /* ��������ȼ� */
                        (TaskHandle_t*  )&app_task_create_handle);/* ������ƿ�ָ�� */ 
  /* ����������� */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   /* �������񣬿������� */
  else
    return -1;  
  
  while(1);   /* ��������ִ�е����� */    
}


/***********************************************************************
  * @ ������  �� app_task_create
  * @ ����˵���� Ϊ�˷���������е����񴴽����������������������
  * @ ����    �� ��  
  * @ ����ֵ  �� ��
  **********************************************************************/
static void app_task_create(void)
{
    int err;
    
    mqtt_client_t *client = NULL;
    
    BaseType_t xReturn = pdPASS;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */

    TCPIP_Init();
    
    printf("\nwelcome to mqttclient test...\n");

    mqtt_log_init();

    client = mqtt_lease();

#ifdef TEST_USEING_TLS
    mqtt_set_port(client, "8883");
    mqtt_set_ca(client, (char*)test_ca_get());
#else
    mqtt_set_port(client, "1883");
#endif

    mqtt_set_host(client, "www.jiejie01.top");
    mqtt_set_client_id(client, random_string(10));
    mqtt_set_user_name(client, random_string(10));
    mqtt_set_password(client, random_string(10));
    mqtt_set_clean_session(client, 1);

    err = mqtt_connect(client);
    
    MQTT_LOG_I("mqtt_connect err = %d", err);
    
    err = mqtt_subscribe(client, "freertos-topic", QOS0, NULL);
    
    MQTT_LOG_I("mqtt_subscribe err = %d", err);    


    taskENTER_CRITICAL();           //�����ٽ���

    /* ����mqtt_task���� */
    xReturn = xTaskCreate((TaskFunction_t )mqtt_task, /* ������ں��� */
                        (const char*    )"mqtt_task",/* �������� */
                        (uint16_t       )2048,   /* ����ջ��С */
                        (void*          )client,	/* ������ں������� */
                        (UBaseType_t    )10,	    /* ��������ȼ� */
                        (TaskHandle_t*  )&mqtt_task_handle);/* ������ƿ�ָ�� */
    if(pdPASS == xReturn)
        printf("Create mqtt_task sucess...\r\n");

    vTaskDelete(app_task_create_handle); //ɾ��app_task_create����

    taskEXIT_CRITICAL();            //�˳��ٽ���
}



/**********************************************************************
  * @ ������  �� mqtt_task
  * @ ����˵���� mqtt_task��������
  * @ ����    ��   
  * @ ����ֵ  �� ��
  ********************************************************************/
static void mqtt_task(void* arg)
{	
    mqtt_client_t *client = (mqtt_client_t *)arg;
    
    char buf[100] = { 0 };
    mqtt_message_t msg;
    memset(&msg, 0, sizeof(msg));
    sprintf(buf, "welcome to mqttclient, this is a publish test...");

    vTaskDelay(4000);
    
    mqtt_list_subscribe_topic(client);

    msg.payload = (void *) buf;
    msg.qos = QOS0;
    
    while(1) {
        sprintf(buf, "welcome to mqttclient, this is a publish test, a rand number: %d ...", random_number());

        mqtt_publish(client, "freertos-topic", &msg);
        
        vTaskDelay(4000);
    }
}
