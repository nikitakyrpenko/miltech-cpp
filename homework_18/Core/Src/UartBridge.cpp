#include "UartBridge.h"

namespace {
UART_HandleTypeDef* HUART = nullptr;

constexpr uint32_t UART_TX_TIMEOUT_MS = 100;
}  // namespace

void Uart_Init(UART_HandleTypeDef* huart)
{
  HUART = huart;
}

void Uart_SendMeasure(Measure* measure)
{
  if (HUART == nullptr) {
    return;
  }

  char out[64];
  uint8_t written = measure->to_string(out, sizeof(out));

  HAL_UART_Transmit(HUART, reinterpret_cast<uint8_t*>(out), written, UART_TX_TIMEOUT_MS);
}
