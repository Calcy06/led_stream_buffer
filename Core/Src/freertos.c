/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stream_buffer.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define QUEUE_DEPTH     8
#define MSG_SIZE        sizeof(uint8_t)
#define UART_STREAM_BUF_SIZE    256     // 串口流缓冲�?�大小，根据你命令最大流量调
#define CMD_BUF_MAX             128     // 单条命令�??大长�??

osMessageQueueId_t ledCmdQueue;
StreamBufferHandle_t uartStreamBuf;

const osThreadAttr_t taskledAttr = {
  .name = "TaskLED",
  .stack_size = 128 *4,
  .priority = (osPriority_t) osPriorityNormal1,
};

const osThreadAttr_t taskcmdAttr = {
  .name = "cmd",
  .stack_size = 128 *12,
  .priority = (osPriority_t) osPriorityNormal2,
};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
void ledTask(void *arg)
{
    uint8_t cmd;
    osStatus_t ret;

    for(;;)
    {
        // 永久阻塞等待消息
        ret = osMessageQueueGet(ledCmdQueue, &cmd, NULL, osWaitForever);
        if(ret == osOK)
        {
            switch(cmd)
            {
                case '1':
                    HAL_GPIO_WritePin(RED_GPIO_Port, RED_Pin, GPIO_PIN_SET);
                    break;
                case '2':
                    HAL_GPIO_WritePin(YELLOW_GPIO_Port, YELLOW_Pin, GPIO_PIN_SET);
                    break;
                case '3':
                    HAL_GPIO_WritePin(GREEN_GPIO_Port, GREEN_Pin, GPIO_PIN_SET);
                    break;
                default:
                    // 其他指令全部熄灭
                    HAL_GPIO_WritePin(RED_GPIO_Port, RED_Pin, GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(YELLOW_GPIO_Port, YELLOW_Pin, GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(GREEN_GPIO_Port, GREEN_Pin, GPIO_PIN_RESET);

                    break;
            }
        }
    }
}

void cmdParseTask(void *arg)
{
    uint8_t ch=0;
    char cmdBuf[CMD_BUF_MAX]={0};
    uint16_t idx = 0;
    uint8_t lastCh = 0;   // 记录上一个字节，用来判断 0x0D 0x0A
    HAL_StatusTypeDef ret = HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_rx_dma_buf, sizeof(uart_rx_dma_buf));
    if(ret != HAL_OK)
    {
        Error_Handler(); // 卡住在这里 = 启动失败
    }    
    for(;;)
    {
        // 阻塞读取1字节
        ch= 0;
				xStreamBufferReceive(uartStreamBuf, &ch, 1, portMAX_DELAY);
        printf("%c", ch);
        // // �??测帧结束：上�??个是 0x0D，当前是 0x0A �?? CRLF
        // if(lastCh == 0x0D && ch == 0x0A)
        // {
        //     // 把末尾的 \r 删掉（idx-1 位置�??0x0D），补字符串结束�??
        //     if(idx > 0)
        //     {
        //         cmdBuf[idx - 1] = '\0';
        //     }
        //     printf("Recv command: [%s]\r\n", cmdBuf);

        //     // ========== 在这里做命令解析 ==========
        //     if(strcmp(cmdBuf, "led1") == 0)
        //     {
        //         printf("led1 on\r\n");
        //         //osMessageQueuePut(ledCmdQueue, "3", 0, 0);
        //     }
        //     else if(strcmp(cmdBuf, "led0") == 0)
        //     {
        //         printf("led0 on\r\n");

        //         //osMessageQueuePut(ledCmdQueue, "2", 0, 0);
        //     }
        //     else if(strcmp(cmdBuf, "hello") == 0)
        //     {
        //         printf("Hello from console!\r\n");
        //     }
        //     else
        //     {
        //         printf("Unknown cmd\r\n");
        //     }

        //     // 重置索引
        //     idx = 0;
        //     lastCh = 0;
        //     continue;
        // }

        // // 存入缓冲区，防止溢出
        // if(idx < (CMD_BUF_MAX - 1))
        // {
        //     cmdBuf[idx++] = ch;
        // }
        // else
        // {
        //     // 溢出，直接丢弃本条包，复�??
        //     idx = 0;
        // }
        // lastCh = ch;

    }
}

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  uartStreamBuf = xStreamBufferCreate( 1024, 1 );
  if( uartStreamBuf == NULL )
  {
      Error_Handler();
  }
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  defaultTaskHandle = osThreadNew(cmdParseTask, NULL, &defaultTask_attributes);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */


  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

