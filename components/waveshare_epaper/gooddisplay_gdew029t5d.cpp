#include "gooddisplay_gdew029t5d.h"
#include <cstdint>
#include "esphome/core/log.h"

// Reference: https://www.good-display.com/product/210.html

namespace esphome
{
  namespace waveshare_epaper
  {
    const char *const GDEW029T5D::TAG = "gdew029t5d";

    const uint8_t GDEW029T5D::LUT_VCOM1[] PROGMEM =
        {
            0x02, 0x30, 0x00, 0x00, 0x00, 0x01,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t GDEW029T5D::LUT_WW1[] PROGMEM =
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t GDEW029T5D::LUT_BW1[] PROGMEM =
        {
            0x80, 0x30, 0x00, 0x00, 0x00, 0x01, // LOW - GND
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t GDEW029T5D::LUT_WB1[] PROGMEM =
        {
            0x40, 0x30, 0x00, 0x00, 0x00, 0x01, // HIGH - GND
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t GDEW029T5D::LUT_BB1[] PROGMEM =
        {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    int GDEW029T5D::get_width_internal() { return WIDTH; }

    int GDEW029T5D::get_width_controller() { return WIDTH; }

    int GDEW029T5D::get_height_internal() { return HEIGHT; }

    uint32_t GDEW029T5D::idle_timeout_()
    {
      return IDLE_TIMEOUT;
    }

    void GDEW029T5D::set_full_update_every(uint32_t full_update_every)
    {
      this->full_update_every_ = full_update_every;
    }

    void GDEW029T5D::full_refresh()
    {
      this->at_update_ = 0;
      this->update();
    }

    void GDEW029T5D::initialize()
    {
    }

    void GDEW029T5D::display()
    {
      bool full_update = this->at_update_ == 0;
      this->at_update_ = (this->at_update_ + 1) % this->full_update_every_;

      this->init_display_();

      if (full_update)
      {
        unsigned int i;
        // Write Data
        this->command(0x10); // Transfer old data
        for (i = 0; i < this->get_buffer_length_(); i++)
        {
          this->data(0xFF); // Transfer the actual displayed data
        }

        this->command(0x13); // Transfer new data
        for (i = 0; i < this->get_buffer_length_(); i++)
        {
          this->data(this->buffer_[i]); // Transfer the actual displayed data
          oldData[i] = this->buffer_[i];
        }
      }
      else
      {
        this->init_part_();

        unsigned int i;
        // Write Data
        this->command(0x10); // Transfer old data
        for (i = 0; i < this->get_buffer_length_(); i++)
        {
          this->data(oldData[i]); // Transfer the actual displayed data
        }

        this->command(0x13); // Transfer new data
        for (i = 0; i < this->get_buffer_length_(); i++)
        {
          this->data(this->buffer_[i]); // Transfer the actual displayed data
          oldData[i] = this->buffer_[i];
        }
      }

      this->command(0x12);           // DISPLAY REFRESH
      delay(1);                      //!!!The delay here is necessary, 200uS at least!!!
      if (!this->wait_until_idle_()) // waiting for the electronic paper IC to release the idle signal
      {
        this->status_set_warning();
        return;
      }

      this->status_clear_warning();

      this->deep_sleep();
    }

    void GDEW029T5D::init_display_()
    {
      if (hibernating_)
        reset_();

      this->command(0x00); // panel setting
      this->data(0x1f);    // LUT from OTP��?KW-BF   KWR-AF  BWROTP 0f BWOTP 1f
      this->data(0x0D);

      this->command(0x61); // resolution setting
      this->data(WIDTH);
      this->data(HEIGHT / 256);
      this->data(HEIGHT % 256);

      this->command(0x04);
      if (!this->wait_until_idle_()) // waiting for the electronic paper IC to release the idle signal
        return;

      this->command(0X50); // VCOM AND DATA INTERVAL SETTING
      this->data(0x97);    // WBmode:VBDF 17|D7 VBDW 97 VBDB 57   WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7
    }

    void GDEW029T5D::reset_()
    {
      if (this->reset_pin_ != nullptr)
      {
        int i;
        for (i = 0; i < 3; i++)
        {
          this->reset_pin_->digital_write(false);
          delay(10);
          this->reset_pin_->digital_write(true);
          delay(10);
        }
      }

      hibernating_ = false;
    }

    void GDEW029T5D::init_part_()
    {
      this->command(0x01); // POWER SETTING
      this->data(0x03);
      this->data(0x00);
      this->data(0x2b);
      this->data(0x2b);
      this->data(0x03);

      this->command(0x06); // boost soft start
      this->data(0x17);    // A
      this->data(0x17);    // B
      this->data(0x17);    // C

      this->command(0x00); // panel setting
      this->data(0xbf);    // LUT from OTP��?128x296
      this->data(0x0D);

      this->command(0x30);
      this->data(0x3C); // 3A 100HZ   29 150Hz 39 200HZ  31 171HZ

      this->command(0x61); // resolution setting
      this->data(WIDTH);
      this->data(HEIGHT / 256);
      this->data(HEIGHT % 256);

      this->command(0x82); // vcom_DC setting
      this->data(0x12);
      write_lut_();

      this->command(0x04);
      if (!this->wait_until_idle_()) // waiting for the electronic paper IC to release the idle signal
      {
        this->status_set_warning();
        return;
      }
    }

    void GDEW029T5D::write_lut_()
    {
      unsigned int count;
      this->command(0x20);
      for (count = 0; count < 44; count++)
      {
        this->data(LUT_VCOM1[count]);
      }

      this->command(0x21);
      for (count = 0; count < 42; count++)
      {
        this->data(LUT_WW1[count]);
      }

      this->command(0x22);
      for (count = 0; count < 42; count++)
      {
        this->data(LUT_BW1[count]);
      }

      this->command(0x23);
      for (count = 0; count < 42; count++)
      {
        this->data(LUT_WB1[count]);
      }

      this->command(0x24);
      for (count = 0; count < 42; count++)
      {
        this->data(LUT_BB1[count]);
      }
    }

    void GDEW029T5D::deep_sleep()
    {
      if (hibernating_)
        return;

      this->command(0X50); // VCOM AND DATA INTERVAL SETTING
      this->data(0xf7);    // WBmode:VBDF 17|D7 VBDW 97 VBDB 57    WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7

      this->command(0X02);           // power off
      if (!this->wait_until_idle_()) // waiting for the electronic paper IC to release the idle signal
        return;

      delay(100);          //!!!The delay here is necessary,100mS at least!!!
      this->command(0X07); // deep sleep
      this->data(0xA5);
      hibernating_ = true;
    }

    void GDEW029T5D::dump_config()
    {
      LOG_DISPLAY("", "Good Display e-Paper", this)
      ESP_LOGCONFIG(TAG, "  Model: GDEW029T5D");
      LOG_PIN("  CS Pin: ", this->cs_)
      LOG_PIN("  Reset Pin: ", this->reset_pin_)
      LOG_PIN("  DC Pin: ", this->dc_pin_)
      LOG_PIN("  Busy Pin: ", this->busy_pin_)
      LOG_UPDATE_INTERVAL(this)
    }

  } // namespace waveshare_epaper
} // namespace esphome