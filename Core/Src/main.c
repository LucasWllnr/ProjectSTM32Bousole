/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LIS2MDL_ADDR              0x1EU
#define LIS2MDL_WHO_AM_I          0x4FU
#define LIS2MDL_WHO_AM_I_VALUE    0x40U
#define LIS2MDL_CFG_REG_A         0x60U
#define LIS2MDL_CFG_REG_B         0x61U
#define LIS2MDL_CFG_REG_C         0x62U
#define LIS2MDL_OUTX_L_REG        0x68U

#define LIS2DW12_ADDR             0x19U
#define LIS2DW12_WHO_AM_I         0x0FU
#define LIS2DW12_WHO_AM_I_VALUE   0x44U
#define LIS2DW12_CTRL1            0x20U
#define LIS2DW12_CTRL6            0x25U
#define LIS2DW12_OUT_X_L          0x28U
#define ACCEL_TILT_LIMIT_RAW      1400

#define MAX7219_REG_DIGIT0        0x01U
#define MAX7219_REG_DECODE        0x09U
#define MAX7219_REG_INTENSITY     0x0AU
#define MAX7219_REG_SCAN_LIMIT    0x0BU
#define MAX7219_REG_SHUTDOWN      0x0CU
#define MAX7219_REG_DISPLAY_TEST  0x0FU

#define COMPASS_UPDATE_MS         150U
#define COMPASS_MAX_READ_ERRORS   3U
#define BUZZER_NEAR_NORTH_DEG     20U
#define BUZZER_ON_NORTH_DEG       5U
#define ADC_UPDATE_REQUESTS       1U
#define ADC_TIMEOUT_LOOPS         100000U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
static uint8_t sensor_ok = 0U;
static uint8_t accel_ok = 0U;
static volatile uint8_t timer6_adc_request = 0U;
static volatile uint32_t timer6_ticks = 0U;
static volatile uint8_t button_event = 0U;
static uint16_t adc_value = 0U;
static uint32_t adc_display_until = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
static void Compass_AppInit(void);
static void Compass_AppTask(void);
static void ADC1_AppInit(void);
static bool ADC1_ReadChannel0(uint16_t *value);
static void TIM6_AppInit(void);
static void Compass_HandleButtonEvent(void);
static void Compass_DisplayError(void);
static bool Compass_ReadHeading(uint16_t *heading_deg);
static uint16_t Compass_HeadingFromMag(int16_t x, int16_t y);
static uint16_t Compass_CorrectHeading(uint16_t heading_deg);
static uint16_t Compass_UserHeading(uint16_t heading_deg);
static char Compass_Cardinal(uint16_t heading_deg);
static void Compass_DisplayHeading(uint16_t heading_deg);
static void Compass_DisplayAdc(uint16_t value);
static void Compass_UpdateLeds(uint16_t heading_deg);
static bool Compass_IsTilted(void);
static void Compass_DisplayTilt(void);
static uint16_t Compass_NorthError(uint16_t heading_deg);
static void Buzzer_UpdateNorth(uint16_t heading_deg);
static void Buzzer_PlayTone(uint16_t duration_ms);
static void Buzzer_DelayHalfPeriod(void);
static void LED_Write(uint8_t value);
static bool LIS2MDL_Init(void);
static bool LIS2MDL_ReadRaw(int16_t *x, int16_t *y, int16_t *z);
static bool LIS2MDL_ReadReg(uint8_t reg, uint8_t *value);
static bool LIS2MDL_WriteReg(uint8_t reg, uint8_t value);
static bool LIS2DW12_Init(void);
static bool LIS2DW12_ReadRaw(int16_t *x, int16_t *y, int16_t *z);
static bool LIS2DW12_ReadReg(uint8_t reg, uint8_t *value);
static bool LIS2DW12_WriteReg(uint8_t reg, uint8_t value);
static bool I2C_WriteBytes(uint8_t addr, const uint8_t *data, uint8_t len);
static bool I2C_ReadRegBytes(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len);
static void I2C_Start(void);
static void I2C_Stop(void);
static bool I2C_WriteByte(uint8_t data);
static uint8_t I2C_ReadByte(bool ack);
static void I2C_SdaOutput(void);
static void I2C_SdaInput(void);
static void I2C_Delay(void);
static void MAX7219_Init(void);
static void MAX7219_Write(uint8_t reg, uint8_t data);
static void MAX7219_SendByte(uint8_t data);
static void MAX7219_DisplayChars(const char chars[4]);
static uint8_t SevenSeg_Code(char c);
static uint16_t Abs16(int16_t value);
int __io_putchar(int ch);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  ADC1_AppInit();
  TIM6_AppInit();
  Compass_AppInit();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    Compass_AppTask();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1U, 0U);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /*Configure GPIO pin : myADC_Pin */
  GPIO_InitStruct.Pin = myADC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(myADC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BZ1_Pin */
  GPIO_InitStruct.Pin = BZ1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(BZ1_GPIO_Port, &GPIO_InitStruct);
  HAL_GPIO_WritePin(BZ1_GPIO_Port, BZ1_Pin, GPIO_PIN_RESET);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* MAX7219 driven in software SPI: PA5=SCK, PA7=MOSI, PA8=CS. */
  GPIO_InitStruct.Pin = SEG_SCK_Pin|SEG_MOSI_Pin|SEG_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOA, SEG_SCK_Pin|SEG_MOSI_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SEG_CS_GPIO_Port, SEG_CS_Pin, GPIO_PIN_SET);

  /* LIS2MDL software I2C: PB8=SCL, PB9=SDA, open-drain with pull-ups. */
  GPIO_InitStruct.Pin = I2C_SCL_Pin|I2C_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, I2C_SCL_Pin|I2C_SDA_Pin, GPIO_PIN_SET);

  /* Shield LED bar L0..L7. */
  GPIO_InitStruct.Pin = L0_Pin|L1_Pin|L2_Pin|L3_Pin|L4_Pin|L5_Pin|L6_Pin|L7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LEDS_GPIO_Port, &GPIO_InitStruct);
  LED_Write(0x00U);

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
static void ADC1_AppInit(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
  ADC->CCR &= ~ADC_CCR_ADCPRE;
  ADC->CCR |= ADC_CCR_ADCPRE_0;

  ADC1->CR1 = 0U;
  ADC1->CR2 = 0U;
  ADC1->SMPR3 = ADC_SMPR3_SMP0;
  ADC1->SQR1 = 0U;
  ADC1->SQR5 = 0U;
  ADC1->CR2 = ADC_CR2_ADON;
}

static bool ADC1_ReadChannel0(uint16_t *value)
{
  uint32_t timeout = ADC_TIMEOUT_LOOPS;

  if (value == NULL) {
    return false;
  }

  ADC1->SR = 0U;
  ADC1->CR2 |= ADC_CR2_SWSTART;
  while ((ADC1->SR & ADC_SR_EOC) == 0U) {
    if (timeout == 0U) {
      return false;
    }
    timeout--;
  }

  *value = (uint16_t)(ADC1->DR & 0x0FFFU);
  return true;
}

static void TIM6_AppInit(void)
{
  RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
  TIM6->CR1 = 0U;
  TIM6->PSC = 32000U - 1U;
  TIM6->ARR = 1000U - 1U;
  TIM6->EGR = TIM_EGR_UG;
  TIM6->SR = 0U;
  TIM6->DIER = TIM_DIER_UIE;
  HAL_NVIC_SetPriority(TIM6_IRQn, 2U, 0U);
  HAL_NVIC_EnableIRQ(TIM6_IRQn);
  TIM6->CR1 = TIM_CR1_CEN;
}

static void Compass_HandleButtonEvent(void)
{
  if (button_event == 0U) {
    return;
  }

  button_event = 0U;
  adc_display_until = HAL_GetTick() + 1200U;
  Compass_DisplayAdc(adc_value);
}

static void Compass_AppInit(void)
{
  MAX7219_Init();
  MAX7219_DisplayChars("INIT");
  sensor_ok = LIS2MDL_Init() ? 1U : 0U;
  accel_ok = LIS2DW12_Init() ? 1U : 0U;

  if (sensor_ok == 0U) {
    Compass_DisplayError();
  }

  printf("Boussole: MAX7219 OK, LIS2MDL %s, LIS2DW12 accel %s\r\n",
         sensor_ok ? "OK" : "absent",
         accel_ok ? "OK" : "absent");
}

static void Compass_AppTask(void)
{
  static uint32_t last_update = 0U;
  static uint8_t read_errors = 0U;
  uint16_t heading = 0U;
  uint16_t user_heading = 0U;
  uint16_t latest_adc = 0U;

  Compass_HandleButtonEvent();

  if (timer6_adc_request >= ADC_UPDATE_REQUESTS) {
    timer6_adc_request = 0U;
    if (ADC1_ReadChannel0(&latest_adc)) {
      adc_value = latest_adc;
    }
  }

  if ((HAL_GetTick() - last_update) < COMPASS_UPDATE_MS) {
    return;
  }
  last_update = HAL_GetTick();

  if ((int32_t)(adc_display_until - HAL_GetTick()) > 0) {
    return;
  }

  if (!Compass_ReadHeading(&heading)) {
    sensor_ok = 0U;
    if (read_errors < COMPASS_MAX_READ_ERRORS) {
      read_errors++;
    }
    if (read_errors >= COMPASS_MAX_READ_ERRORS) {
      Compass_DisplayError();
      LED_Write(0x00U);
    }
    HAL_GPIO_WritePin(BZ1_GPIO_Port, BZ1_Pin, GPIO_PIN_RESET);
    return;
  }

  sensor_ok = 1U;
  read_errors = 0U;
  if (Compass_IsTilted()) {
    Compass_DisplayTilt();
    printf("Inclinaison trop forte: mesure du cap ignoree\r\n");
    return;
  }

  user_heading = Compass_UserHeading(heading);
  Compass_DisplayHeading(user_heading);
  Compass_UpdateLeds(user_heading);
  Buzzer_UpdateNorth(user_heading);
  printf("Cap affiche: %03u deg [%c], ecart nord affiche %03u deg\r\n",
         user_heading,
         Compass_Cardinal(user_heading),
         Compass_NorthError(user_heading));
}

static void Compass_DisplayError(void)
{
  MAX7219_DisplayChars("ERR ");
}

static bool Compass_ReadHeading(uint16_t *heading_deg)
{
  int16_t x = 0;
  int16_t y = 0;
  int16_t z = 0;

  if (heading_deg == NULL) {
    return false;
  }

  if (!LIS2MDL_ReadRaw(&x, &y, &z)) {
    (void)z;
    return false;
  }

  *heading_deg = Compass_CorrectHeading(Compass_HeadingFromMag(x, y));
  return true;
}

static uint16_t Compass_HeadingFromMag(int16_t x, int16_t y)
{
  int32_t ax = Abs16(x);
  int32_t ay = Abs16(y);
  int32_t angle;

  if (x == 0 && y == 0) {
    return 0U;
  }

  if (ax >= ay) {
    angle = (45L * ay) / (ax == 0 ? 1 : ax);
  } else {
    angle = 90L - ((45L * ax) / (ay == 0 ? 1 : ay));
  }

  if (x >= 0 && y >= 0) {
    return (uint16_t)angle;
  }
  if (x < 0 && y >= 0) {
    return (uint16_t)(180L - angle);
  }
  if (x < 0 && y < 0) {
    return (uint16_t)(180L + angle);
  }
  return (uint16_t)(360L - angle);
}

static uint16_t Compass_CorrectHeading(uint16_t heading_deg)
{
  heading_deg %= 360U;
  if (heading_deg == 0U) {
    return 0U;
  }
  return (uint16_t)(360U - heading_deg);
}

static uint16_t Compass_UserHeading(uint16_t heading_deg)
{
  return (uint16_t)((heading_deg + 180U) % 360U);
}

static char Compass_Cardinal(uint16_t heading_deg)
{
  static const char dirs[8] = {'N', 'E', 'E', 'S', 'S', 'W', 'W', 'N'};
  uint8_t sector = (uint8_t)(((uint32_t)heading_deg + 22U) / 45U) & 0x07U;
  return dirs[sector];
}

static void Compass_DisplayHeading(uint16_t heading_deg)
{
  char text[4];

  heading_deg %= 360U;
  text[0] = Compass_Cardinal(heading_deg);
  text[1] = (char)('0' + (heading_deg / 100U));
  text[2] = (char)('0' + ((heading_deg / 10U) % 10U));
  text[3] = (char)('0' + (heading_deg % 10U));
  MAX7219_DisplayChars(text);
}

static void Compass_DisplayAdc(uint16_t value)
{
  char text[4];
  uint16_t display_value;

  display_value = (uint16_t)(((uint32_t)(value & 0x0FFFU) * 999U) / 4095U);
  text[0] = 'A';
  text[1] = (char)('0' + (display_value / 100U));
  text[2] = (char)('0' + ((display_value / 10U) % 10U));
  text[3] = (char)('0' + (display_value % 10U));
  MAX7219_DisplayChars(text);
}

static void Compass_UpdateLeds(uint16_t heading_deg)
{
  uint8_t sector = (uint8_t)(((uint32_t)heading_deg + 22U) / 45U) & 0x07U;
  LED_Write((uint8_t)(1U << sector));
}

static bool Compass_IsTilted(void)
{
  int16_t ax = 0;
  int16_t ay = 0;
  int16_t az = 0;

  if (accel_ok == 0U) {
    return false;
  }

  if (!LIS2DW12_ReadRaw(&ax, &ay, &az)) {
    accel_ok = 0U;
    return false;
  }

  (void)az;
  return Abs16(ax) > ACCEL_TILT_LIMIT_RAW || Abs16(ay) > ACCEL_TILT_LIMIT_RAW;
}

static void Compass_DisplayTilt(void)
{
  MAX7219_DisplayChars("TILT");
  LED_Write(0x81U);
  HAL_GPIO_WritePin(BZ1_GPIO_Port, BZ1_Pin, GPIO_PIN_RESET);
}

static uint16_t Compass_NorthError(uint16_t heading_deg)
{
  heading_deg %= 360U;
  return heading_deg <= 180U ? heading_deg : (uint16_t)(360U - heading_deg);
}

static void Buzzer_UpdateNorth(uint16_t heading_deg)
{
  static uint32_t last_beep = 0U;
  uint32_t now = HAL_GetTick();
  uint16_t error = Compass_NorthError(heading_deg);
  uint32_t interval_ms;

  if (error > BUZZER_NEAR_NORTH_DEG) {
    HAL_GPIO_WritePin(BZ1_GPIO_Port, BZ1_Pin, GPIO_PIN_RESET);
    return;
  }

  interval_ms = error <= BUZZER_ON_NORTH_DEG ? 180U : 650U;
  if ((now - last_beep) < interval_ms) {
    return;
  }

  last_beep = now;
  Buzzer_PlayTone(error <= BUZZER_ON_NORTH_DEG ? 80U : 35U);
}

static void Buzzer_PlayTone(uint16_t duration_ms)
{
  uint32_t start = HAL_GetTick();

  while ((HAL_GetTick() - start) < duration_ms) {
    HAL_GPIO_WritePin(BZ1_GPIO_Port, BZ1_Pin, GPIO_PIN_SET);
    Buzzer_DelayHalfPeriod();
    HAL_GPIO_WritePin(BZ1_GPIO_Port, BZ1_Pin, GPIO_PIN_RESET);
    Buzzer_DelayHalfPeriod();
  }
}

static void Buzzer_DelayHalfPeriod(void)
{
  volatile uint16_t i;

  for (i = 0U; i < 650U; i++) {
    __NOP();
  }
}

static void LED_Write(uint8_t value)
{
  HAL_GPIO_WritePin(LEDS_GPIO_Port, L0_Pin,  (value & 0x01U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LEDS_GPIO_Port, L1_Pin,  (value & 0x02U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LEDS_GPIO_Port, L2_Pin,  (value & 0x04U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LEDS_GPIO_Port, L3_Pin,  (value & 0x08U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LEDS_GPIO_Port, L4_Pin,  (value & 0x10U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LEDS_GPIO_Port, L5_Pin,  (value & 0x20U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LEDS_GPIO_Port, L6_Pin,  (value & 0x40U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LEDS_GPIO_Port, L7_Pin,  (value & 0x80U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static bool LIS2MDL_Init(void)
{
  uint8_t whoami = 0U;

  if (!LIS2MDL_ReadReg(LIS2MDL_WHO_AM_I, &whoami) || whoami != LIS2MDL_WHO_AM_I_VALUE) {
    return false;
  }

  if (!LIS2MDL_WriteReg(LIS2MDL_CFG_REG_A, 0x8CU)) {
    return false;
  }
  if (!LIS2MDL_WriteReg(LIS2MDL_CFG_REG_B, 0x02U)) {
    return false;
  }
  return LIS2MDL_WriteReg(LIS2MDL_CFG_REG_C, 0x10U);
}

static bool LIS2MDL_ReadRaw(int16_t *x, int16_t *y, int16_t *z)
{
  uint8_t data[6] = {0};

  if (x == NULL || y == NULL || z == NULL) {
    return false;
  }

  if (!I2C_ReadRegBytes(LIS2MDL_ADDR, (uint8_t)(LIS2MDL_OUTX_L_REG | 0x80U), data, sizeof(data))) {
    return false;
  }

  *x = (int16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
  *y = (int16_t)((uint16_t)data[2] | ((uint16_t)data[3] << 8));
  *z = (int16_t)((uint16_t)data[4] | ((uint16_t)data[5] << 8));
  return true;
}

static bool LIS2MDL_ReadReg(uint8_t reg, uint8_t *value)
{
  return I2C_ReadRegBytes(LIS2MDL_ADDR, reg, value, 1U);
}

static bool LIS2MDL_WriteReg(uint8_t reg, uint8_t value)
{
  uint8_t data[2] = {reg, value};
  return I2C_WriteBytes(LIS2MDL_ADDR, data, sizeof(data));
}

static bool LIS2DW12_Init(void)
{
  uint8_t whoami = 0U;

  if (!LIS2DW12_ReadReg(LIS2DW12_WHO_AM_I, &whoami) || whoami != LIS2DW12_WHO_AM_I_VALUE) {
    return false;
  }

  if (!LIS2DW12_WriteReg(LIS2DW12_CTRL6, 0x00U)) {
    return false;
  }
  return LIS2DW12_WriteReg(LIS2DW12_CTRL1, 0x50U);
}

static bool LIS2DW12_ReadRaw(int16_t *x, int16_t *y, int16_t *z)
{
  uint8_t data[6] = {0};

  if (x == NULL || y == NULL || z == NULL) {
    return false;
  }

  if (!I2C_ReadRegBytes(LIS2DW12_ADDR, (uint8_t)(LIS2DW12_OUT_X_L | 0x80U), data, sizeof(data))) {
    return false;
  }

  *x = (int16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
  *y = (int16_t)((uint16_t)data[2] | ((uint16_t)data[3] << 8));
  *z = (int16_t)((uint16_t)data[4] | ((uint16_t)data[5] << 8));

  *x >>= 2;
  *y >>= 2;
  *z >>= 2;
  return true;
}

static bool LIS2DW12_ReadReg(uint8_t reg, uint8_t *value)
{
  return I2C_ReadRegBytes(LIS2DW12_ADDR, reg, value, 1U);
}

static bool LIS2DW12_WriteReg(uint8_t reg, uint8_t value)
{
  uint8_t data[2] = {reg, value};
  return I2C_WriteBytes(LIS2DW12_ADDR, data, sizeof(data));
}

static bool I2C_WriteBytes(uint8_t addr, const uint8_t *data, uint8_t len)
{
  uint8_t i;

  I2C_Start();
  if (!I2C_WriteByte((uint8_t)(addr << 1))) {
    I2C_Stop();
    return false;
  }
  for (i = 0; i < len; i++) {
    if (!I2C_WriteByte(data[i])) {
      I2C_Stop();
      return false;
    }
  }
  I2C_Stop();
  return true;
}

static bool I2C_ReadRegBytes(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len)
{
  uint8_t i;

  if (data == NULL || len == 0U) {
    return false;
  }

  I2C_Start();
  if (!I2C_WriteByte((uint8_t)(addr << 1)) || !I2C_WriteByte(reg)) {
    I2C_Stop();
    return false;
  }

  I2C_Start();
  if (!I2C_WriteByte((uint8_t)((addr << 1) | 1U))) {
    I2C_Stop();
    return false;
  }

  for (i = 0; i < len; i++) {
    data[i] = I2C_ReadByte(i < (uint8_t)(len - 1U));
  }
  I2C_Stop();
  return true;
}

static void I2C_Start(void)
{
  I2C_SdaOutput();
  HAL_GPIO_WritePin(I2C_SDA_GPIO_Port, I2C_SDA_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_SET);
  I2C_Delay();
  HAL_GPIO_WritePin(I2C_SDA_GPIO_Port, I2C_SDA_Pin, GPIO_PIN_RESET);
  I2C_Delay();
  HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_RESET);
}

static void I2C_Stop(void)
{
  I2C_SdaOutput();
  HAL_GPIO_WritePin(I2C_SDA_GPIO_Port, I2C_SDA_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_SET);
  I2C_Delay();
  HAL_GPIO_WritePin(I2C_SDA_GPIO_Port, I2C_SDA_Pin, GPIO_PIN_SET);
  I2C_Delay();
}

static bool I2C_WriteByte(uint8_t data)
{
  uint8_t i;
  GPIO_PinState ack;

  I2C_SdaOutput();
  for (i = 0; i < 8U; i++) {
    HAL_GPIO_WritePin(I2C_SDA_GPIO_Port, I2C_SDA_Pin,
                      (data & 0x80U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    I2C_Delay();
    HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_SET);
    I2C_Delay();
    HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_RESET);
    data <<= 1;
  }

  I2C_SdaInput();
  I2C_Delay();
  HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_SET);
  I2C_Delay();
  ack = HAL_GPIO_ReadPin(I2C_SDA_GPIO_Port, I2C_SDA_Pin);
  HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_RESET);
  I2C_SdaOutput();
  return ack == GPIO_PIN_RESET;
}

static uint8_t I2C_ReadByte(bool ack)
{
  uint8_t i;
  uint8_t data = 0U;

  I2C_SdaInput();
  for (i = 0; i < 8U; i++) {
    data <<= 1;
    HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_SET);
    I2C_Delay();
    if (HAL_GPIO_ReadPin(I2C_SDA_GPIO_Port, I2C_SDA_Pin) == GPIO_PIN_SET) {
      data |= 1U;
    }
    HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_RESET);
    I2C_Delay();
  }

  I2C_SdaOutput();
  HAL_GPIO_WritePin(I2C_SDA_GPIO_Port, I2C_SDA_Pin, ack ? GPIO_PIN_RESET : GPIO_PIN_SET);
  I2C_Delay();
  HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_SET);
  I2C_Delay();
  HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(I2C_SDA_GPIO_Port, I2C_SDA_Pin, GPIO_PIN_SET);
  return data;
}

static void I2C_SdaOutput(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = I2C_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(I2C_SDA_GPIO_Port, &GPIO_InitStruct);
}

static void I2C_SdaInput(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = I2C_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(I2C_SDA_GPIO_Port, &GPIO_InitStruct);
}

static void I2C_Delay(void)
{
  volatile uint16_t i;

  for (i = 0U; i < 90U; i++) {
    __NOP();
  }
}

static void MAX7219_Init(void)
{
  MAX7219_Write(MAX7219_REG_DISPLAY_TEST, 0x00U);
  MAX7219_Write(MAX7219_REG_SCAN_LIMIT, 0x03U);
  MAX7219_Write(MAX7219_REG_DECODE, 0x00U);
  MAX7219_Write(MAX7219_REG_INTENSITY, 0x08U);
  MAX7219_Write(MAX7219_REG_SHUTDOWN, 0x01U);
  MAX7219_DisplayChars("    ");
}

static void MAX7219_Write(uint8_t reg, uint8_t data)
{
  HAL_GPIO_WritePin(SEG_CS_GPIO_Port, SEG_CS_Pin, GPIO_PIN_RESET);
  MAX7219_SendByte(reg);
  MAX7219_SendByte(data);
  HAL_GPIO_WritePin(SEG_CS_GPIO_Port, SEG_CS_Pin, GPIO_PIN_SET);
}

static void MAX7219_SendByte(uint8_t data)
{
  uint8_t i;

  for (i = 0U; i < 8U; i++) {
    HAL_GPIO_WritePin(SEG_SCK_GPIO_Port, SEG_SCK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SEG_MOSI_GPIO_Port, SEG_MOSI_Pin,
                      (data & 0x80U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    data <<= 1;
    HAL_GPIO_WritePin(SEG_SCK_GPIO_Port, SEG_SCK_Pin, GPIO_PIN_SET);
  }
  HAL_GPIO_WritePin(SEG_SCK_GPIO_Port, SEG_SCK_Pin, GPIO_PIN_RESET);
}

static void MAX7219_DisplayChars(const char chars[4])
{
  uint8_t i;

  for (i = 0U; i < 4U; i++) {
    MAX7219_Write((uint8_t)(MAX7219_REG_DIGIT0 + i), SevenSeg_Code(chars[i]));
  }
}

static uint8_t SevenSeg_Code(char c)
{
  switch (c) {
    case '0': return 0x7eU;
    case '1': return 0x30U;
    case '2': return 0x6dU;
    case '3': return 0x79U;
    case '4': return 0x33U;
    case '5': return 0x5bU;
    case '6': return 0x5fU;
    case '7': return 0x70U;
    case '8': return 0x7fU;
    case '9': return 0x7bU;
    case 'A': return 0x77U;
    case 'C': return 0x4eU;
    case 'D': return 0x3dU;
    case 'E': return 0x4fU;
    case 'I': return 0x30U;
    case 'L': return 0x0eU;
    case 'N': return 0x76U;
    case 'O': return 0x1dU;
    case 'P': return 0x67U;
    case 'R': return 0x05U;
    case 'S': return 0x5bU;
    case 'T': return 0x0fU;
    case 'W': return 0x3eU;
    default: return 0x00U;
  }
}

static uint16_t Abs16(int16_t value)
{
  return (uint16_t)(value < 0 ? -value : value);
}

void App_ButtonIrqHandler(void)
{
  static uint32_t last_button_ms = 0U;
  uint32_t now = HAL_GetTick();

  if ((now - last_button_ms) > 200U) {
    button_event = 1U;
    last_button_ms = now;
  }
}

void App_Timer6IrqHandler(void)
{
  timer6_ticks++;
  if (timer6_adc_request < 255U) {
    timer6_adc_request++;
  }
}

int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1U, HAL_MAX_DELAY);
  return ch;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
