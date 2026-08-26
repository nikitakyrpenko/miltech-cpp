#pragma once

#include <cstdint>
#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_spi.h"

class Sx1280Device {
  static constexpr uint8_t BUSY_TIMEOUT_MS = 100;
  static constexpr uint8_t RESET_PULSE_MS = 5;

public:
  enum class CircutMode : uint8_t {
    RESERVED = 0,
    RESERVED_1 = 1,
    STDBY_RC = 2,   /*Idle, clocked by internal 13MHz RC oscillator*/
    STDBY_XOSC = 3, /*Idle, clocked by external crystal/TCXO*/
    FS = 4,         /*PLL locked to configured RF frequency, not yet Tx/Rx*/
    RX = 5,         /*Actively receiving*/
    TX = 6,         /*Actively transmitting*/
  };
  enum class CommandStatus : uint8_t {
    RESERVED,
    PROCCESSED /*The command has been terminated correctly*/,
    DATA_AVAILABLE,    /*A packet has been successfully received and data can be retrieved*/
    COMMAND_TIMEOUT,   /*A transaction from the host took too long to complete and triggered an internal watchdog*/
    COMMAND_ERROR,     /*The transceiver was unable to process command either because of an invalid opcode or because an incorrect number of
    parameters has been   provided*/
    FAILED_TO_EXECUTE, /*The command was successfully processed, however the transceiver could not execute the command;*/
    TX_DONE,           /*The transmission of the current packet has terminated */
  };
  struct Status {
    CircutMode mode;
    CommandStatus status;

    bool is_ok() const;
  };

  struct BLE_PDU {
    static constexpr uint8_t ADV_NONCONN_IND = 0x02;
    static constexpr uint8_t TX_ADD = 1;
    static constexpr uint8_t RX_ADD = 0;

    static constexpr uint8_t HEADER_SIZE = 2;
    static constexpr uint8_t ADV_A_SIZE = 6;
    static constexpr uint8_t ADV_DATA_SIZE = 25;

    static constexpr uint8_t SIZE = HEADER_SIZE + ADV_A_SIZE + ADV_DATA_SIZE;

    static constexpr uint8_t HEADER_BYTE0 = ADV_NONCONN_IND | (TX_ADD << 6) | (RX_ADD << 7);
    static constexpr uint8_t HEADER_BYTE1 = ADV_A_SIZE + ADV_DATA_SIZE;

    static constexpr uint8_t header[2] = {HEADER_BYTE0, HEADER_BYTE1};
    static constexpr uint8_t adv_a[6] = {0x01, 0x00, 0x00, 0x00, 0x00, 0xC0};
  };

#pragma pack(push, 1)
  struct AdvData {
    uint16_t angle_raw;
    uint32_t timestamp_ms;
    uint8_t status;

    void to_bytes(uint8_t out[BLE_PDU::ADV_DATA_SIZE]) const;
  };
#pragma pack(pop)

  static_assert(sizeof(AdvData) <= BLE_PDU::ADV_DATA_SIZE, "AdvData must fit inside the fixed ADV_DATA_SIZE slot");

  static constexpr uint16_t IRQ_TX_DONE = 0x0001;

  explicit Sx1280Device(SPI_HandleTypeDef* spi,
                        GPIO_TypeDef* NSS_chip,
                        GPIO_TypeDef* BUSY_chip,
                        GPIO_TypeDef* NRESET_chip,
                        uint16_t NSS_pin,
                        uint16_t BUSY_pin,
                        uint16_t NRESET_pin);

  Status get_status();
  bool init();
  void reset();
  bool read_register(uint16_t address, uint8_t* out, uint16_t out_len);
  bool read_buffer(uint8_t offset, uint8_t* out, uint8_t out_len);
  bool write_buffer(const uint8_t* buf, uint8_t len, bool (*verify)(Status) = &check_processed);
  bool clear_irq(bool (*verify)(Status) = &check_processed);
  bool set_tx(bool (*verify)(Status) = &check_processed);
  bool get_irq_status(uint16_t* out_irq_status);
  bool send_test_packet(const AdvData& data);

private:
  SPI_HandleTypeDef* hspi_;
  GPIO_TypeDef* NSS_chip_;
  GPIO_TypeDef* BUSY_chip_;
  GPIO_TypeDef* NRESET_chip_;
  uint16_t BUSY_pin_;
  uint16_t NSS_pin_;
  uint16_t NRESET_pin_;

  static bool check_processed(Status s);

  void SPI_NSS_begin();
  void SPI_NSS_end();
  bool BUSY_wait();
  bool send_command(uint8_t opcode, const uint8_t* params, uint16_t params_len, bool (*verify)(Status) = &check_processed);
};
