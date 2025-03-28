#include "gooddisplay_gdem029t94.h"

#include <cstdint>

#include "esphome/core/log.h"

// This implementation is based on the GxEPD2 library
// https://github.com/ZinggJM/GxEPD2/blob/master/src/epd/GxEPD2_290_T94_V2.cpp

namespace esphome {
namespace waveshare_epaper {
const char *const GDEM029T94::TAG = "gdem029t94";

const uint8_t GDEM029T94::LUT_DATA_PART[] = {
    0x0,  0x40, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0x0, 0x0,  0x80,
    0x80, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0x0, 0x40, 0x40,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0x0, 0x80, 0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0x0, 0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0A, 0x0, 0x0, 0x0,  0x0,
    0x0,  0x2,  0x1,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x1, 0x0, 0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0x0, 0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0x0, 0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0x0, 0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0x0, 0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0x0, 0x0,  0x0,
    0x0,  0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x0, 0x0,  0x0};

#ifdef USE_ESP32
RTC_DATA_ATTR uint32_t GDEM029T94::at_update_ = 0;
#endif

int GDEM029T94::get_width_internal() { return WIDTH; }

int GDEM029T94::get_height_internal() { return HEIGHT; }

uint32_t GDEM029T94::idle_timeout_() { return IDLE_TIMEOUT; }

void GDEM029T94::set_full_update_every(uint32_t full_update_every) {
  this->full_update_every_ = full_update_every;
}

void GDEM029T94::full_refresh() {
  this->at_update_ = 0;
  this->update();
}

void GDEM029T94::initialize() {
#ifdef USE_ESP32
  esp_reset_reason_t reason = esp_reset_reason();
  if (reason == ESP_RST_EXT) this->at_update_ = 0;
#endif
}

void GDEM029T94::display() {
  bool full_update = this->at_update_ == 0;
  this->at_update_ = (this->at_update_ + 1) % this->full_update_every_;

  this->init_display_();

  if (full_update) {
    this->command(0x24);
    this->start_data_();
    this->write_array(this->buffer_, this->get_buffer_length_());
    this->end_data_();

    this->command(0x22);
    this->data(0xf7);
    this->command(0x20);
    if (!this->wait_until_idle_()) {
      this->status_set_warning();
      return;
    }

    this->command(0x26);
    this->start_data_();
    this->write_array(this->buffer_, this->get_buffer_length_());
    this->end_data_();
  } else {
    this->command(0x24);
    this->start_data_();
    this->write_array(this->buffer_, this->get_buffer_length_());
    this->end_data_();

    this->cmd_data(0x32, LUT_DATA_PART, sizeof(LUT_DATA_PART));

    this->command(0x22);
    this->data(0xcc);
    this->command(0x20);

    if (!this->wait_until_idle_()) {
      this->status_set_warning();
      return;
    }

    this->command(0x26);
    this->start_data_();
    this->write_array(this->buffer_, this->get_buffer_length_());
    this->end_data_();
  }

  this->status_clear_warning();

  this->deep_sleep();
}

void GDEM029T94::init_display_() {
  if (!initial_) {
    reset_();
    initial_ = true;
  }

  if (hibernating_) reset_();

  delay(10);            // 10ms according to specs
  this->command(0x12);  // SWRESET

  delay(10);            // 10ms according to specs
  this->command(0x01);  // Driver output control
  this->data(0x27);
  this->data(0x01);
  this->data(0x00);

  this->command(0x11);  // data entry mode
  this->data(0x03);

  this->command(0x3C);  // BorderWavefrom
  this->data(0x05);

  this->command(0x21);  //  Display update control
  this->data(0x00);
  this->data(0x80);

  this->command(0x18);  // Read built-in temperature sensor
  this->data(0x80);
  this->setPartialRamArea_(0, 0, WIDTH, HEIGHT);
}

void GDEM029T94::reset_() {
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->digital_write(false);
    delay(10);
    this->reset_pin_->digital_write(true);
    delay(10);
    hibernating_ = false;
  }
  this->wait_until_idle_();
}

void GDEM029T94::setPartialRamArea_(uint16_t x, uint16_t y, uint16_t w,
                                    uint16_t h) {
  this->command(0x11);  // set ram entry mode
  this->data(0x03);     // x increase, y increase : normal mode

  this->command(0x44);
  this->data(x / 8);
  this->data((x + w - 1) / 8);

  this->command(0x45);
  this->data(y % 256);
  this->data(y / 256);
  this->data((y + h - 1) % 256);
  this->data((y + h - 1) / 256);

  this->command(0x4e);
  this->data(x / 8);

  this->command(0x4f);
  this->data(y % 256);
  this->data(y / 256);
}

void GDEM029T94::deep_sleep() {
  if (hibernating_) return;

  // power off
  this->command(0x22);
  this->data(0x83);
  this->command(0x20);

  if (this->reset_pin_ != nullptr) {
    this->command(0x10);  // deep sleep mode
    this->data(0x1);      // enter deep sleep
    hibernating_ = true;
  }
}

void GDEM029T94::dump_config() {
  LOG_DISPLAY("", "Good Display e-Paper", this)
  ESP_LOGCONFIG(TAG, "  Model: GDEM029T94");
  LOG_PIN("  CS Pin: ", this->cs_)
  LOG_PIN("  Reset Pin: ", this->reset_pin_)
  LOG_PIN("  DC Pin: ", this->dc_pin_)
  LOG_PIN("  Busy Pin: ", this->busy_pin_)
  LOG_UPDATE_INTERVAL(this)
}

}  // namespace waveshare_epaper
}  // namespace esphome