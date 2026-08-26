#include "Sx1280Device.hpp"
#include <cstdint>
#include "stm32l4xx_hal_def.h"
#include "stm32l4xx_hal_spi.h"

namespace {
namespace Sx1280_OPCODE {
constexpr uint8_t GET_STATUS_OP_CODE = 0xC0;

constexpr uint8_t SET_STANDBY_OP_CODE = 0x80;
constexpr uint8_t SET_PACKET_TYPE_OP_CODE = 0x8A;
constexpr uint8_t SET_FREQUENCY_OP_CODE = 0x86;
constexpr uint8_t SET_BUFFER_BASE_ADDRESS_OP_CODE = 0x8F;
constexpr uint8_t SET_MODULATION_OP_CODE = 0x8B;
constexpr uint8_t SET_PACKET_PARAMS_OP_CODE = 0x8C;
constexpr uint8_t WRITE_REGISTER_OP_CODE = 0x18;
constexpr uint8_t READ_REGISTER_OP_CODE = 0x19;
constexpr uint8_t WRITE_BUFFER_OP_CODE = 0x1A;
constexpr uint8_t CLEAR_IRQ_STATUS_OP_CODE = 0x97;
constexpr uint8_t SET_TX_OP_CODE = 0x83;
constexpr uint8_t GET_IRQ_STATUS_OP_CODE = 0x15;
constexpr uint8_t SET_TX_PARAMS_OP_CODE = 0x8E;
constexpr uint8_t SET_DIO_IRQ_PARAMS_OP_CODE = 0x8D;
constexpr uint8_t READ_BUFFER_OP_CODE = 0x1B;
}  // namespace Sx1280_OPCODE

namespace Sx1280_Constants {
// SetStandby: STDBY_RC (0x00) -- SetPacketType must run in STDBY_RC per
// datasheet §13.1.1; STDBY_XOSC (0x01) switch happens later, before SetTx.
constexpr uint8_t STANDBY_PARAMS[] = {0x00};

// SetPacketType: PACKET_TYPE_BLE.
constexpr uint8_t PACKET_TYPE_PARAMS[] = {0x04};

// SetRfFrequency: channel 37 (2402 MHz) -> freq_reg = RF_Hz / (52e6/2^18) ->
// 12,109,036 -> 0xB8C4EC, big-endian.
constexpr uint8_t FREQUENCY_PARAMS[] = {0xB8, 0xC4, 0xEC};

// SetBufferBaseAddress: txBaseAddress, rxBaseAddress.
constexpr uint8_t BUFFER_BASE_ADDRESS_PARAMS[] = {0x00, 0x00};

// SetModulationParams, BLE 1M PHY: 1Mbps/2.4MHz bitrate-bandwidth
// (BLE_BR_1_000_BW_2_4, sx1280.txt:4898 -- trying this over BW_1_2/0x45 to
// see if it's the bandwidth variant real BLE receivers expect),
// mod index 0.5, BT 0.5 (Bluetooth Core Spec mandated).
constexpr uint8_t MODULATION_PARAMS[] = {0x4C, 0x01, 0x20};

// SetPacketParams, BLE mode (only 4 params accepted, not the general 7):
// ConnectionState=BLE_ADVERTISER, CrcLength=BLE_CRC_3B,
// BleTestPayload=0x00, Whitening=BLE_WHITENING_ENABLE.
constexpr uint8_t PACKET_PARAMS[] = {0x02, 0x10, 0x00, 0x00};

// SetTxParams: power=31 -> Pout = -18+31 = +13dBm (max), rampTime=0x00
// (RADIO_RAMP_02_US, fastest). Datasheet-required before first SetTx
// (sx1280.txt:4702-4704) -- missing this causes SetTx to fail with
// FAILED_TO_EXECUTE, since the chip has no configured Tx power/ramp.
constexpr uint8_t TX_PARAMS[] = {31, 0x00};

// SetDioIrqParams: irqMask enables TxDone (bit 0) so it can be latched
// into the IRQ register at all -- confirmed (sx1280.txt:4170-4171) that a
// bit is only ever set in the IRQ register if the matching irqMask bit is
// enabled, independent of whether it's routed to a physical DIO pin.
// dio1/dio2/dio3 masks stay 0 -- not wiring DIO1 yet, polling only.
constexpr uint8_t DIO_IRQ_PARAMS[] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// WriteRegister: SyncWord1 register address (0x09CF) followed by the
// standard BLE advertising access address (0x8E89BED6).
constexpr uint8_t SYNC_WORD_1_PARAMS[] = {0x09, 0xCF, 0x8E, 0x89, 0xBE, 0xD6};

// WriteRegister: CrcInit register address (0x09C7, 3 bytes MSB-first) set
// to the standard BLE advertising CRC seed 0x555555 (sx1280.txt:5080-5088).
// Without this the chip computes CRC with whatever seed it defaults to,
// which real BLE receivers won't match -- they always assume 0x555555 for
// advertising-channel packets, and silently drop anything that fails CRC.
constexpr uint8_t CRC_INIT_PARAMS[] = {0x09, 0xC7, 0x55, 0x55, 0x55};

// WriteBuffer: offset relative to txBaseAddress. Always 0 -- every send
// rewrites the full PDU from the start of the TX FIFO region.
constexpr uint8_t WRITE_BUFFER_OFFSET = 0x00;

// ClearIrqStatus: irqMask[15:8], irqMask[7:0] -- clears only TxDone (bit 0).
constexpr uint8_t CLEAR_IRQ_TX_DONE_PARAMS[] = {0x00, 0x01};

// SetTx: periodBase, periodBaseCount[15:8], periodBaseCount[7:0].
// periodBaseCount=0x0000 -- Single mode, no timeout, auto-returns to
// STDBY_RC on completion.
constexpr uint8_t SET_TX_PARAMS[] = {0x00, 0x00, 0x00};

// AdvData AD structures: Complete Local Name + Manufacturer Specific Data
// (sensor payload). 0xFFFF is the conventional non-SIG-assigned company ID
// used for testing -- a real product needs a Bluetooth SIG-registered one.
constexpr uint8_t AD_TYPE_COMPLETE_LOCAL_NAME = 0x09;
constexpr uint8_t AD_TYPE_MANUFACTURER_DATA = 0xFF;
constexpr uint8_t DEVICE_NAME[] = {'S', 'X', '1', '2', '8', '0'};
constexpr uint16_t MANUFACTURER_ID = 0xFFFF;

constexpr uint8_t NAME_AD_LEN = 1 + sizeof(DEVICE_NAME);
constexpr uint8_t SENSOR_AD_LEN = 1 + 2 + 7;  // type + companyId + (angle_raw+timestamp_ms+status)
}  // namespace Sx1280_Constants
}  // namespace

static_assert(2 + Sx1280_Constants::NAME_AD_LEN + Sx1280_Constants::SENSOR_AD_LEN <= Sx1280Device::BLE_PDU::ADV_DATA_SIZE,
              "Name + sensor AD structures must fit inside the fixed ADV_DATA_SIZE slot");

void Sx1280Device::AdvData::to_bytes(uint8_t out[BLE_PDU::ADV_DATA_SIZE]) const
{
  uint8_t i = 0;

  // Complete Local Name AD structure.
  out[i++] = Sx1280_Constants::NAME_AD_LEN;
  out[i++] = Sx1280_Constants::AD_TYPE_COMPLETE_LOCAL_NAME;
  for (uint8_t j = 0; j < sizeof(Sx1280_Constants::DEVICE_NAME); ++j) {
    out[i++] = Sx1280_Constants::DEVICE_NAME[j];
  }

  // Manufacturer Specific Data AD structure: company ID + sensor payload.
  out[i++] = Sx1280_Constants::SENSOR_AD_LEN;
  out[i++] = Sx1280_Constants::AD_TYPE_MANUFACTURER_DATA;
  out[i++] = static_cast<uint8_t>(Sx1280_Constants::MANUFACTURER_ID & 0xFF);
  out[i++] = static_cast<uint8_t>((Sx1280_Constants::MANUFACTURER_ID >> 8) & 0xFF);
  out[i++] = static_cast<uint8_t>(angle_raw & 0xFF);
  out[i++] = static_cast<uint8_t>((angle_raw >> 8) & 0xFF);
  out[i++] = static_cast<uint8_t>(timestamp_ms & 0xFF);
  out[i++] = static_cast<uint8_t>((timestamp_ms >> 8) & 0xFF);
  out[i++] = static_cast<uint8_t>((timestamp_ms >> 16) & 0xFF);
  out[i++] = static_cast<uint8_t>((timestamp_ms >> 24) & 0xFF);
  out[i++] = status;

  for (; i < BLE_PDU::ADV_DATA_SIZE; ++i) {
    out[i] = 0;
  }
}

bool Sx1280Device::check_processed(Status s)
{
  return s.status == CommandStatus::PROCCESSED;
}

bool Sx1280Device::Status::is_ok() const
{
  switch (mode) {
    case Sx1280Device::CircutMode::STDBY_RC:
    case Sx1280Device::CircutMode::STDBY_XOSC:
    case Sx1280Device::CircutMode::FS:
    case Sx1280Device::CircutMode::RX:
    case Sx1280Device::CircutMode::TX:
      return true;
    default:
      return false;
  }
}

Sx1280Device::Sx1280Device(SPI_HandleTypeDef* spi,
                           GPIO_TypeDef* NSS_chip,
                           GPIO_TypeDef* BUSY_chip,
                           GPIO_TypeDef* NRESET_chip,
                           uint16_t NSS_pin,
                           uint16_t BUSY_pin,
                           uint16_t NRESET_pin)
  : hspi_(spi)
  , NSS_chip_(NSS_chip)
  , BUSY_chip_(BUSY_chip)
  , NRESET_chip_(NRESET_chip)
  , BUSY_pin_(BUSY_pin)
  , NSS_pin_(NSS_pin)
  , NRESET_pin_(NRESET_pin)
{
}

Sx1280Device::Status Sx1280Device::get_status()
{
  if (!BUSY_wait()) {
    return Status{CircutMode::RESERVED, CommandStatus::RESERVED};
  }

  uint8_t tx[] = {Sx1280_OPCODE::GET_STATUS_OP_CODE, 0};
  uint8_t rx[] = {0, 0};

  SPI_NSS_begin();
  HAL_SPI_TransmitReceive(hspi_, tx, rx, 2, HAL_MAX_DELAY);
  SPI_NSS_end();

  CircutMode mode = static_cast<CircutMode>((rx[1] >> 5) & 0x07);
  CommandStatus status = static_cast<CommandStatus>((rx[1] >> 2) & 0x07);

  return Status{mode, status};
}

bool Sx1280Device::init()
{
  const auto check_status_or_reserved = [](Status s) -> bool {
    return s.status == CommandStatus::PROCCESSED || s.status == CommandStatus::RESERVED;
  };

  if (!send_command(Sx1280_OPCODE::SET_STANDBY_OP_CODE, Sx1280_Constants::STANDBY_PARAMS, sizeof(Sx1280_Constants::STANDBY_PARAMS))) {
    return false;
  }

  if (!send_command(
        Sx1280_OPCODE::SET_PACKET_TYPE_OP_CODE, Sx1280_Constants::PACKET_TYPE_PARAMS, sizeof(Sx1280_Constants::PACKET_TYPE_PARAMS))) {
    return false;
  }

  if (!send_command(Sx1280_OPCODE::SET_FREQUENCY_OP_CODE, Sx1280_Constants::FREQUENCY_PARAMS, sizeof(Sx1280_Constants::FREQUENCY_PARAMS))) {
    return false;
  }

  if (!send_command(Sx1280_OPCODE::SET_BUFFER_BASE_ADDRESS_OP_CODE,
                    Sx1280_Constants::BUFFER_BASE_ADDRESS_PARAMS,
                    sizeof(Sx1280_Constants::BUFFER_BASE_ADDRESS_PARAMS))) {
    return false;
  }

  if (!send_command(
        Sx1280_OPCODE::SET_MODULATION_OP_CODE, Sx1280_Constants::MODULATION_PARAMS, sizeof(Sx1280_Constants::MODULATION_PARAMS))) {
    return false;
  }

  if (!send_command(Sx1280_OPCODE::SET_PACKET_PARAMS_OP_CODE, Sx1280_Constants::PACKET_PARAMS, sizeof(Sx1280_Constants::PACKET_PARAMS))) {
    return false;
  }

  if (!send_command(Sx1280_OPCODE::SET_TX_PARAMS_OP_CODE, Sx1280_Constants::TX_PARAMS, sizeof(Sx1280_Constants::TX_PARAMS))) {
    return false;
  }

  if (!send_command(
        Sx1280_OPCODE::SET_DIO_IRQ_PARAMS_OP_CODE, Sx1280_Constants::DIO_IRQ_PARAMS, sizeof(Sx1280_Constants::DIO_IRQ_PARAMS))) {
    return false;
  }

  if (!send_command(Sx1280_OPCODE::WRITE_REGISTER_OP_CODE,
                    Sx1280_Constants::SYNC_WORD_1_PARAMS,
                    sizeof(Sx1280_Constants::SYNC_WORD_1_PARAMS),
                    check_status_or_reserved)) {
    return false;
  }

  if (!send_command(Sx1280_OPCODE::WRITE_REGISTER_OP_CODE,
                    Sx1280_Constants::CRC_INIT_PARAMS,
                    sizeof(Sx1280_Constants::CRC_INIT_PARAMS),
                    check_status_or_reserved)) {
    return false;
  }

  return true;
}

void Sx1280Device::reset()
{
  HAL_GPIO_WritePin(NRESET_chip_, NRESET_pin_, GPIO_PIN_RESET);
  HAL_Delay(RESET_PULSE_MS);
  HAL_GPIO_WritePin(NRESET_chip_, NRESET_pin_, GPIO_PIN_SET);
  BUSY_wait();
}

bool Sx1280Device::read_register(uint16_t address, uint8_t* out, uint16_t out_len)
{
  if (!BUSY_wait()) {
    return false;
  }

  uint16_t total_len = 4 + out_len;
  uint8_t tx[16] = {0};
  uint8_t rx[16] = {0};
  tx[0] = Sx1280_OPCODE::READ_REGISTER_OP_CODE;
  tx[1] = static_cast<uint8_t>(address >> 8);
  tx[2] = static_cast<uint8_t>(address & 0xFF);

  SPI_NSS_begin();
  HAL_StatusTypeDef result = HAL_SPI_TransmitReceive(hspi_, tx, rx, total_len, HAL_MAX_DELAY);
  SPI_NSS_end();

  if (result != HAL_OK) {
    return false;
  }

  for (uint16_t i = 0; i < out_len; ++i) {
    out[i] = rx[4 + i];
  }
  return true;
}

bool Sx1280Device::read_buffer(uint8_t offset, uint8_t* out, uint8_t out_len)
{
  if (!BUSY_wait()) {
    return false;
  }

  // ReadBuffer response layout differs from read_register: 3 status bytes
  // before real data starts, not 4 (sx1280.txt:3126-3134).
  uint16_t total_len = 3 + out_len;
  uint8_t tx[3 + BLE_PDU::SIZE] = {0};
  uint8_t rx[3 + BLE_PDU::SIZE] = {0};
  tx[0] = Sx1280_OPCODE::READ_BUFFER_OP_CODE;
  tx[1] = offset;

  SPI_NSS_begin();
  HAL_StatusTypeDef result = HAL_SPI_TransmitReceive(hspi_, tx, rx, total_len, HAL_MAX_DELAY);
  SPI_NSS_end();

  if (result != HAL_OK) {
    return false;
  }

  for (uint8_t i = 0; i < out_len; ++i) {
    out[i] = rx[3 + i];
  }
  return true;
}

void Sx1280Device::SPI_NSS_begin()
{
  HAL_GPIO_WritePin(NSS_chip_, NSS_pin_, GPIO_PIN_RESET);
}

void Sx1280Device::SPI_NSS_end()
{
  HAL_GPIO_WritePin(NSS_chip_, NSS_pin_, GPIO_PIN_SET);
}

bool Sx1280Device::BUSY_wait()
{
  uint32_t start = HAL_GetTick();
  while (HAL_GPIO_ReadPin(BUSY_chip_, BUSY_pin_) == GPIO_PIN_SET) {
    if (HAL_GetTick() - start > BUSY_TIMEOUT_MS) {
      return false;  // chip unresponsive
    }
  }
  return true;
}

bool Sx1280Device::send_command(uint8_t opcode, const uint8_t* params, uint16_t params_len, bool (*verify)(Status))
{
  if (!BUSY_wait()) {
    return false;
  }

  uint8_t tx[8] = {0};
  uint8_t rx[8] = {0};
  tx[0] = opcode;
  for (uint16_t i = 0; i < params_len; ++i) {
    tx[1 + i] = params[i];
  }

  SPI_NSS_begin();
  HAL_StatusTypeDef result = HAL_SPI_TransmitReceive(hspi_, tx, rx, 1 + params_len, HAL_MAX_DELAY);
  SPI_NSS_end();

  const auto s = this->get_status();

  return result == HAL_OK && verify(s);
}

bool Sx1280Device::write_buffer(const uint8_t* buf, uint8_t len, bool (*verify)(Status))
{
  if (!BUSY_wait()) {
    return false;
  }

  if (len > BLE_PDU::SIZE) {
    return false;
  }

  uint8_t tx[BLE_PDU::SIZE + 2] = {Sx1280_OPCODE::WRITE_BUFFER_OP_CODE, Sx1280_Constants::WRITE_BUFFER_OFFSET};
  uint8_t rx[BLE_PDU::SIZE + 2] = {};

  for (int i = 0; i < len; i++) {
    tx[i + 2] = buf[i];
  }

  SPI_NSS_begin();
  HAL_StatusTypeDef result = HAL_SPI_TransmitReceive(hspi_, tx, rx, BLE_PDU::SIZE + 2, HAL_MAX_DELAY);
  SPI_NSS_end();

  const auto s = this->get_status();

  return result == HAL_OK && verify(s);
}

bool Sx1280Device::clear_irq(bool (*verify)(Status))
{
  return send_command(Sx1280_OPCODE::CLEAR_IRQ_STATUS_OP_CODE,
                      Sx1280_Constants::CLEAR_IRQ_TX_DONE_PARAMS,
                      sizeof(Sx1280_Constants::CLEAR_IRQ_TX_DONE_PARAMS),
                      verify);
}

bool Sx1280Device::set_tx(bool (*verify)(Status))
{
  return send_command(Sx1280_OPCODE::SET_TX_OP_CODE, Sx1280_Constants::SET_TX_PARAMS, sizeof(Sx1280_Constants::SET_TX_PARAMS), verify);
}

bool Sx1280Device::get_irq_status(uint16_t* out_irq_status)
{
  if (!BUSY_wait()) {
    return false;
  }

  // GetIrqStatus response layout differs from the generic command pattern:
  // 2 status bytes before the real data, not 1.
  uint8_t tx[4] = {Sx1280_OPCODE::GET_IRQ_STATUS_OP_CODE, 0, 0, 0};
  uint8_t rx[4] = {0};

  SPI_NSS_begin();
  HAL_StatusTypeDef result = HAL_SPI_TransmitReceive(hspi_, tx, rx, sizeof(tx), HAL_MAX_DELAY);
  SPI_NSS_end();

  if (result != HAL_OK) {
    return false;
  }

  *out_irq_status = (static_cast<uint16_t>(rx[2]) << 8) | rx[3];
  return true;
}

bool Sx1280Device::send_test_packet(const AdvData& data)
{
  uint8_t payload[BLE_PDU::SIZE];
  payload[0] = BLE_PDU::header[0];
  payload[1] = BLE_PDU::header[1];

  for (uint8_t i = 0; i < BLE_PDU::ADV_A_SIZE; ++i) {
    payload[BLE_PDU::HEADER_SIZE + i] = BLE_PDU::adv_a[i];
  }

  data.to_bytes(payload + BLE_PDU::HEADER_SIZE + BLE_PDU::ADV_A_SIZE);
  const auto wb = write_buffer(payload, sizeof(payload), [](Status s) -> bool {
    return s.status == CommandStatus::PROCCESSED || s.status == CommandStatus::RESERVED;
  });
  const auto rq = clear_irq();
  const auto tx = set_tx();

  return wb && rq && tx;
}
