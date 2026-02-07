#pragma once

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/core/automation.h"

namespace esphome {
namespace athermo {

class AthermoComponent : public Component {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;

  // Pin setters (called from Python code generation)
  void set_pir_dis_pin(GPIOPin *pin) { this->pir_dis_pin_ = pin; }
  void set_periph_vcc_pin(GPIOPin *pin) { this->periph_vcc_pin_ = pin; }

  // Public methods for actions
  void periph_vcc_on();
  void periph_vcc_off();
  void power_cycle(uint32_t delay_ms = 500);

 protected:
  GPIOPin *pir_dis_pin_{nullptr};
  GPIOPin *periph_vcc_pin_{nullptr};
};

// Action: Turn periph_vcc ON (LOW state = P-MOSFET conducts)
template<typename... Ts> class PeriphVccOnAction : public Action<Ts...> {
 public:
  explicit PeriphVccOnAction(AthermoComponent *parent) : parent_(parent) {}

  void play(Ts... x) override { this->parent_->periph_vcc_on(); }

 protected:
  AthermoComponent *parent_;
};

// Action: Turn periph_vcc OFF (HIGH state = P-MOSFET off)
template<typename... Ts> class PeriphVccOffAction : public Action<Ts...> {
 public:
  explicit PeriphVccOffAction(AthermoComponent *parent) : parent_(parent) {}

  void play(Ts... x) override { this->parent_->periph_vcc_off(); }

 protected:
  AthermoComponent *parent_;
};

// Action: Power cycle (OFF -> delay -> ON)
template<typename... Ts> class PowerCycleAction : public Action<Ts...> {
 public:
  explicit PowerCycleAction(AthermoComponent *parent) : parent_(parent) {}

  void set_delay_ms(uint32_t delay_ms) { this->delay_ms_ = delay_ms; }

  void play(Ts... x) override { this->parent_->power_cycle(this->delay_ms_); }

 protected:
  AthermoComponent *parent_;
  uint32_t delay_ms_{500};
};

}  // namespace athermo
}  // namespace esphome
