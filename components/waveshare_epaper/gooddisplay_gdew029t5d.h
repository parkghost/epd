#include "waveshare_epaper.h"

namespace esphome
{
  namespace waveshare_epaper
  {

    class GDEW029T5D : public WaveshareEPaper
    {
    public:
      void initialize() override;

      void dump_config() override;

      void display() override;

      void deep_sleep() override;

      void set_full_update_every(uint32_t full_update_every);

      void full_refresh();

    protected:
      int get_width_internal() override;

      int get_width_controller() override;

      int get_height_internal() override;

      uint32_t idle_timeout_() override;

      void init_display_();

      void init_part_();

      void reset_();

      void write_lut_();

      uint32_t at_update_{0}, full_update_every_{30};
      bool hibernating_{false};
    };

  } // namespace waveshare_epaper
} // namespace esphome