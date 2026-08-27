#include "Sx1280Bridge.h"

#include "Sx1280Device.hpp"
#include "main.h"
#include "stm32l4xx_nucleo.h"

namespace {
Sx1280Device* LORA_1280 = nullptr;

volatile bool initialized = false;

constexpr uint32_t SEND_INTERVAL_MS = 1000;
uint32_t last_send_tick = 0;
bool waiting_for_tx_done = false;

// TCXOEN idles low (main.c) and is never otherwise driven -- without this
// the external TCXO is never powered, so the PLL has no clock to lock to
// and SetTx fails with FAILED_TO_EXECUTE. 5ms is a placeholder warm-up
// delay; check the actual TCXO module's datasheet for its real startup time.
constexpr uint32_t TCXO_WARMUP_MS = 5;
}  // namespace

void Sx1280_Init(SPI_HandleTypeDef* hspi,
                 GPIO_TypeDef* nss_port,
                 GPIO_TypeDef* busy_port,
                 GPIO_TypeDef* nreset_port,
                 uint16_t nss_pin,
                 uint16_t busy_pin,
                 uint16_t nreset_pin)
{
  HAL_GPIO_WritePin(TCXOEN_GPIO_Port, TCXOEN_Pin, GPIO_PIN_SET);
  HAL_Delay(TCXO_WARMUP_MS);

  static Sx1280Device device(hspi, nss_port, busy_port, nreset_port, nss_pin, busy_pin, nreset_pin);
  LORA_1280 = &device;
  LORA_1280->reset();
  initialized = LORA_1280->init();

  if (initialized && LORA_1280->get_status().is_ok()) {
    BSP_LED_On(LED_GREEN);
  }
}

void Sx1280_Loop()
{
  if (LORA_1280 == nullptr || !initialized) {
    return;
  }

  uint32_t now = HAL_GetTick();

  if (!waiting_for_tx_done) {
    if (now - last_send_tick < SEND_INTERVAL_MS) {
      return;
    }
    last_send_tick = now;

    // mock AdvData until the AS5600 is wired in
    Sx1280Device::AdvData mock_data{0, now, 0};

    bool b = LORA_1280->send_test_packet(mock_data);
    waiting_for_tx_done = b;

    // Debug only: read back what WriteBuffer actually stored, to confirm
    // it matches the intended PDU. Inspect via GDB.
    uint8_t readback[Sx1280Device::PAYLOAD_SIZE] = {0};
    bool readback_ok = LORA_1280->read_buffer(0, readback, sizeof(readback));
    (void)readback_ok;

    return;
  }

  uint16_t irq_status = 0;
  if (LORA_1280->get_irq_status(&irq_status) && (irq_status & Sx1280Device::IRQ_TX_DONE)) {
    LORA_1280->clear_irq();
    BSP_LED_Toggle(LED_GREEN);
    waiting_for_tx_done = false;
  }
}
