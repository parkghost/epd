#include "hink_e0213a09.h"
#include <cstdint>
#include "esphome/core/log.h"

// This implementation is based on the GxEPD2 library
// https://github.com/ZinggJM/GxEPD2/blob/master/src/epd/GxEPD2_213_B72.cpp

namespace esphome
{
  namespace waveshare_epaper
  {

    static const char *const TAG = "e0213a09";

    static const uint16_t WIDTH = 104;

    static const uint16_t HEIGHT = 212;

    static const uint16_t IDLE_TIMEOUT = 2500;

    static const uint8_t LUT_SIZE = 70;

    static const uint8_t LUT_DATA_full[LUT_SIZE] =
        {
            0x80, 0x60, 0x40, 0x00, 0x00, 0x00, 0x00, // LUT0: BB:     VS 0 ~7
            0x10, 0x60, 0x20, 0x00, 0x00, 0x00, 0x00, // LUT1: BW:     VS 0 ~7
            0x80, 0x60, 0x40, 0x00, 0x00, 0x00, 0x00, // LUT2: WB:     VS 0 ~7
            0x10, 0x60, 0x20, 0x00, 0x00, 0x00, 0x00, // LUT3: WW:     VS 0 ~7
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // LUT4: VCOM:   VS 0 ~7
            0x03, 0x03, 0x00, 0x00, 0x02,             // TP0 A~D RP0
            0x09, 0x09, 0x00, 0x00, 0x02,             // TP1 A~D RP1
            0x03, 0x03, 0x00, 0x00, 0x02,             // TP2 A~D RP2
            0x00, 0x00, 0x00, 0x00, 0x00,             // TP3 A~D RP3
            0x00, 0x00, 0x00, 0x00, 0x00,             // TP4 A~D RP4
            0x00, 0x00, 0x00, 0x00, 0x00,             // TP5 A~D RP5
            0x00, 0x00, 0x00, 0x00, 0x00,             // TP6 A~D RP6
    };

    static const uint8_t LUT_DATA_part[LUT_SIZE] =
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // LUT0: BB:     VS 0 ~7
            0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // LUT1: BW:     VS 0 ~7
            0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // LUT2: WB:     VS 0 ~7
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // LUT3: WW:     VS 0 ~7
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // LUT4: VCOM:   VS 0 ~7
            0x0A, 0x00, 0x00, 0x00, 0x00,             // TP0 A~D RP0
            0x00, 0x00, 0x00, 0x00, 0x00,             // TP1 A~D RP1
            0x00, 0x00, 0x00, 0x00, 0x00,             // TP2 A~D RP2
            0x00, 0x00, 0x00, 0x00, 0x00,             // TP3 A~D RP3
            0x00, 0x00, 0x00, 0x00, 0x00,             // TP4 A~D RP4
            0x00, 0x00, 0x00, 0x00, 0x00,             // TP5 A~D RP5
            0x00, 0x00, 0x00, 0x00, 0x00,             // TP6 A~D RP6
    };

    int E0213A09::get_width_internal() { return WIDTH; }

    int E0213A09::get_width_controller() { return WIDTH; }

    int E0213A09::get_height_internal() { return HEIGHT; }

    uint32_t E0213A09::idle_timeout_()
    {
      return IDLE_TIMEOUT;
    }

    void E0213A09::set_full_update_every(uint32_t full_update_every)
    {
      this->full_update_every_ = full_update_every;
    }

    void E0213A09::full_refresh()
    {
      this->at_update_ = 0;
      this->update();
    }

    void E0213A09::initialize()
    {
    }

    void E0213A09::display()
    {
      bool full_update = this->at_update_ == 0;
      this->at_update_ = (this->at_update_ + 1) % this->full_update_every_;

      this->init_display_();
      if (full_update)
      {
        this->write_lut_(LUT_DATA_full, sizeof(LUT_DATA_full));
      }
      else
      {
        this->command(0x2C); // VCOM Voltage
        this->data(0x26);    // NA ??
        this->write_lut_(LUT_DATA_part, sizeof(LUT_DATA_part));
      }
      this->command(0x22);
      this->data(0xc0);
      this->command(0x20);
      if (!this->wait_until_idle_())
      {
        this->status_set_warning();
        return;
      }

      this->setPartialRamArea_(0, 0, WIDTH, HEIGHT);
      this->command(0x24);
      this->start_data_();
      this->write_array(this->buffer_, this->get_buffer_length_());
      this->end_data_();

      this->command(0x22);
      this->data(full_update ? 0xC4 : 0x04);
      this->command(0x20);
      if (!this->wait_until_idle_())
      {
        this->status_set_warning();
        return;
      }

      this->setPartialRamArea_(0, 0, WIDTH, HEIGHT);
      this->command(0x26);
      this->start_data_();
      this->write_array(this->buffer_, this->get_buffer_length_());
      this->end_data_();

      this->status_clear_warning();

      this->deep_sleep();
    }

    void E0213A09::init_display_()
    {
      if (hibernating_)
        reset_();

      this->command(0x74); // set analog block control
      this->data(0x54);

      this->command(0x7E); // set digital block control
      this->data(0x3B);

      this->command(0x01);           // Driver output control
      this->data(HEIGHT - 1);        // (HEIGHT - 1) % 256
      this->data((HEIGHT - 1) >> 8); // (HEIGHT - 1) / 256
      this->data(0x00);

      this->command(0x3C); // BorderWavefrom
      this->data(0x03);

      this->command(0x2C); // VCOM Voltage
      this->data(0x70);    // NA ??

      this->command(0x03); // Gate Driving voltage Control
      this->data(0x15);    // 19V

      this->command(0x04); // Source Driving voltage Control
      this->data(0x41);    // VSH1 15V
      this->data(0xA8);    // VSH2 5V
      this->data(0x32);    // VSL -15V

      this->command(0x3A); // Dummy Line
      this->data(0x30);

      this->command(0x3B); // Gate time
      this->data(0x0A);
      this->setPartialRamArea_(0, 0, WIDTH, HEIGHT);
    }

    void E0213A09::reset_()
    {
      if (this->reset_pin_ != nullptr)
      {
        this->reset_pin_->digital_write(false);
        delay(10);
        this->reset_pin_->digital_write(true);
        delay(10);
        hibernating_ = false;
      }
      this->wait_until_idle_();
    }

    void E0213A09::setPartialRamArea_(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
    {
      this->command(0x11); // set ram entry mode
      this->data(0x03);    // x increase, y increase : normal mode

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

    void E0213A09::write_lut_(const uint8_t *lut, uint8_t n)
    {
      this->command(0x32);
      for (uint8_t i = 0; i < n; i++)
        this->data(lut[i]);
    }

    void E0213A09::deep_sleep()
    {
      if (hibernating_)
        return;

      // power off
      this->command(0x22);
      this->data(0xc3);
      this->command(0x20);

      if (this->reset_pin_ != nullptr)
      {
        this->command(0x10); // deep sleep mode
        this->data(0x1);     // enter deep sleep
        hibernating_ = true;
      }
    }

    void E0213A09::dump_config()
    {
      LOG_DISPLAY("", "HINK e-Paper", this)
      ESP_LOGCONFIG(TAG, "  Model: E0213A09");
      LOG_PIN("  CS Pin: ", this->cs_)
      LOG_PIN("  Reset Pin: ", this->reset_pin_)
      LOG_PIN("  DC Pin: ", this->dc_pin_)
      LOG_PIN("  Busy Pin: ", this->busy_pin_)
      LOG_UPDATE_INTERVAL(this)
    }

  } // namespace waveshare_epaper
} // namespace esphome