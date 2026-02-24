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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

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
/* Definitions for myTask02 */
osThreadId_t myTask02Handle;
const osThreadAttr_t myTask02_attributes = {
  .name = "myTask02",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for myTask03 */
osThreadId_t myTask03Handle;
const osThreadAttr_t myTask03_attributes = {
  .name = "myTask03",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for myTask04 */
osThreadId_t myTask04Handle;
const osThreadAttr_t myTask04_attributes = {
  .name = "myTask04",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for Usart_Rx */
osMessageQueueId_t Usart_RxHandle;
const osMessageQueueAttr_t Usart_Rx_attributes = {
  .name = "Usart_Rx"
};
/* Definitions for PWM_Step */
osMessageQueueId_t PWM_StepHandle;
const osMessageQueueAttr_t PWM_Step_attributes = {
  .name = "PWM_Step"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void tskLED(void *argument);
void tskUsartRx(void *argument);
void tskOLED(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

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

  /* Create the queue(s) */
  /* creation of Usart_Rx */
  Usart_RxHandle = osMessageQueueNew (20, 64, &Usart_Rx_attributes);

  /* creation of PWM_Step */
  PWM_StepHandle = osMessageQueueNew (1, sizeof(uint8_t), &PWM_Step_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of myTask02 */
  myTask02Handle = osThreadNew(tskLED, NULL, &myTask02_attributes);

  /* creation of myTask03 */
  myTask03Handle = osThreadNew(tskUsartRx, NULL, &myTask03_attributes);

  /* creation of myTask04 */
  myTask04Handle = osThreadNew(tskOLED, NULL, &myTask04_attributes);

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

/* USER CODE BEGIN Header_tskLED */
/**
* @brief Function implementing the myTask02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_tskLED */
void tskLED(void *argument)
{
  /* USER CODE BEGIN tskLED */
  int16_t PWM_Compare = 0;
  uint16_t period = 10;
  int8_t dir = 1;
  uint8_t step = 1;
  uint32_t previouswaken = osKernelGetTickCount();
  /* Infinite loop */
  for(;;)
  {
    uint8_t new_step;

    if(xQueueReceive(PWM_StepHandle, &new_step, 0) == pdTRUE)
    {
      step = new_step;
    }

    PWM_Light_Config(PWM_Compare, period);
    PWM_Compare += dir * step;
    if(PWM_Compare >= 100) dir = -1;
    else if(PWM_Compare <= 0) dir = 1;
    vTaskDelayUntil(&previouswaken, pdMS_TO_TICKS(10));
  }
  /* USER CODE END tskLED */
}

/* USER CODE BEGIN Header_tskUsartRx */
/**
* @brief Function implementing the myTask03 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_tskUsartRx */
void tskUsartRx(void *argument)
{
  /* USER CODE BEGIN tskUsartRx */
  uint8_t msg[64];
  uint8_t step;
  /* Infinite loop */
  for(;;)
  {
    if(xQueueReceive(Usart_RxHandle, msg, portMAX_DELAY) == pdTRUE)
    {

      msg[rx_data_len] = '\0';

      char *p = strstr((char*)msg, "step=");
      if(p != NULL)
      {
        p += 5;
        int val = atoi(p);
        if(val >= 1 && val <= 10)
        {
          step = (uint8_t)val;
          xQueueSend(PWM_StepHandle, &step, 0); 
          char ok[] = "OK\n";
          HAL_UART_Transmit_DMA(&huart1, (uint8_t*)ok, strlen(ok));
        }
        else
        {
          char err[] = "ERR: step must be 1-10\n";
          HAL_UART_Transmit_DMA(&huart1, (uint8_t*)err, strlen(err));
        }
      }
      else
      {
        char err[] = "ERR: use 'step=<1-10>'\n";
        HAL_UART_Transmit_DMA(&huart1, (uint8_t*)err, strlen(err));
      }
    }
  }
  /* USER CODE END tskUsartRx */
}

/* USER CODE BEGIN Header_tskOLED */
/**
* @brief Function implementing the myTask04 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_tskOLED */
void tskOLED(void *argument)
{
  /* USER CODE BEGIN tskOLED */
  int16_t x = 0;
  int8_t dir = 2;           // 每次移动2像素
  uint8_t screen_width = 128;
  uint8_t char_width = 16;  // 16x16汉字宽度
  uint8_t num_chars = 3;    // 显示3个汉字
  uint8_t total_width = char_width * num_chars;

  // 初始化OLED
  OLED_Init();
  OLED_NewFrame();
  OLED_ShowFrame();

  uint32_t previouswaken = osKernelGetTickCount();
  /* Infinite loop */
  for(;;)
  {
    OLED_NewFrame();  // 清屏

    OLED_PrintString(x, 24, "彭宇熙", &font16x16, OLED_COLOR_NORMAL);

    // 刷新屏幕
    OLED_ShowFrame();

    // 移动坐标
    x += dir;
    if(x >= (screen_width - total_width)) dir = -2;
    else if(x <= 0) dir = 2;
    vTaskDelayUntil(&previouswaken, pdMS_TO_TICKS(10));
  }
  /* USER CODE END tskOLED */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

