#include "gooddisplay_gdey075t7.h"

// https://github.com/esphome/esphome/blob/dev/esphome/components/waveshare_epaper/waveshare_epaper.cpp
// # WaveshareEPaper7P5InV2P

namespace esphome {
namespace waveshare_epaper {

const char *const GDEY075T7::TAG = "gdey075t7";

int GDEY075T7::get_width_internal() { return WIDTH; }

int GDEY075T7::get_height_internal() { return HEIGHT; }

uint32_t GDEY075T7::idle_timeout_() { return IDLE_TIMEOUT; }

void GDEY075T7::set_full_update_every(uint32_t full_update_every) {
  this->full_update_every_ = full_update_every;
}

void GDEY075T7::full_refresh() {
  this->at_update_ = 0;
  this->update();
}

void GDEY075T7::initialize() {}

void HOT GDEY075T7::display() {
  bool full_update = this->at_update_ == 0;
  this->at_update_ = (this->at_update_ + 1) % this->full_update_every_;

  this->init_display_();

  this->command(0x04);
  delay(200);  // NOLINT
  this->wait_until_idle_();

  this->command(0x50);
  this->data(0xA9);
  this->data(0x07);

  if (full_update) {
    // Enable fast refresh
    this->command(0xE5);
    this->data(0x5A);

    this->command(0x92);

    this->command(0x10);
    delay(2);
    for (uint32_t i = 0; i < this->get_buffer_length_(); i++) {
      this->data(~(this->buffer_[i]));
    }

    delay(100);  // NOLINT
    this->wait_until_idle_();

    this->command(0x13);
    delay(2);
    for (uint32_t i = 0; i < this->get_buffer_length_(); i++) {
      this->data(this->buffer_[i]);
    }

  } else {
    // Enable partial refresh
    this->command(0xE5);
    this->data(0x6E);

    // Activate partial refresh and set window bounds
    this->command(0x91);
    this->command(0x90);

    this->data(0x00);
    this->data(0x00);
    this->data((WIDTH - 1) >> 8 & 0xFF);
    this->data((WIDTH - 1) & 0xFF);

    this->data(0x00);
    this->data(0x00);
    this->data((HEIGHT - 1) >> 8 & 0xFF);
    this->data((HEIGHT - 1) & 0xFF);

    this->data(0x01);

    this->command(0x13);
    delay(2);
    for (uint32_t i = 0; i < this->get_buffer_length_(); i++) {
      this->data(this->buffer_[i]);
    }
  }

  delay(100);  // NOLINT
  this->wait_until_idle_();

  this->command(0x12);
  delay(100);  // NOLINT
  this->wait_until_idle_();

  this->command(0x02);
  this->wait_until_idle_();
}

void GDEY075T7::init_display_() {
  if (!initial_) {
    reset_();
    initial_ = true;
  }

  if (hibernating_) reset_();

  // COMMAND POWER SETTING
  this->command(0x01);
  this->data(0x07);
  this->data(0x07);
  this->data(0x3f);
  this->data(0x3f);

  // COMMAND BOOSTER SOFT START
  this->command(0x06);
  this->data(0x17);
  this->data(0x17);
  this->data(0x28);
  this->data(0x17);

  // COMMAND POWER DRIVER HAT UP
  this->command(0x04);
  delay(100);  // NOLINT
  this->wait_until_idle_();

  // COMMAND PANEL SETTING
  this->command(0x00);
  this->data(0x1F);

  // COMMAND RESOLUTION SETTING
  this->command(0x61);
  this->data(0x03);
  this->data(0x20);
  this->data(0x01);
  this->data(0xE0);

  // COMMAND DUAL SPI MM_EN, DUSPI_EN
  this->command(0x15);
  this->data(0x00);

  // COMMAND VCOM AND DATA INTERVAL SETTING
  this->command(0x50);
  this->data(0x10);
  this->data(0x07);

  // COMMAND TCON SETTING
  this->command(0x60);
  this->data(0x22);

  // COMMAND ENABLE FAST UPDATE
  this->command(0xE0);
  this->data(0x02);
  this->command(0xE5);
  this->data(0x5A);
}

void GDEY075T7::reset_() {
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->digital_write(true);
    delay(20);
    this->reset_pin_->digital_write(false);
    delay(2);
    this->reset_pin_->digital_write(true);
    delay(20);
  }

  hibernating_ = false;
}

void GDEY075T7::deep_sleep() {
  if (hibernating_) return;

  this->command(0x50);  // VCOM AND DATA INTERVAL SETTING
  this->data(0xf7);     // WBmode:VBDF 17|D7 VBDW 97 VBDB 57    WBRmode:VBDF F7
                        // VBDW 77 VBDB 37  VBDR B7

  this->command(0x02);            // power off
  if (!this->wait_until_idle_())  // waiting for the electronic paper IC to
                                  // release the idle signal
    return;

  delay(100);           //!!!The delay here is necessary,100mS at least!!!
  this->command(0x07);  // deep sleep
  this->data(0xA5);
  hibernating_ = true;
}

void GDEY075T7::dump_config() {
  LOG_DISPLAY("", "Good Display e-Paper", this)
  ESP_LOGCONFIG(TAG, "  Model: GDEY075T7");
  LOG_PIN("  CS Pin: ", this->cs_)
  LOG_PIN("  Reset Pin: ", this->reset_pin_)
  LOG_PIN("  DC Pin: ", this->dc_pin_)
  LOG_PIN("  Busy Pin: ", this->busy_pin_)
  LOG_UPDATE_INTERVAL(this)
}

}  // namespace waveshare_epaper
}  // namespace esphome