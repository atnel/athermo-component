#include "athermo.h"
#include "esphome/core/log.h"
// test2
// aaa mmm
namespace esphome {
namespace athermo {

static const char *const TAG = "athermo";

// HARDWARE priority = 600 (runs very early, before most components)
float AthermoComponent::get_setup_priority() const { 
  return setup_priority::HARDWARE; 
}

void AthermoComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ATHERMO component...");

  // CRITICAL: Set PIR_DIS to HIGH immediately to disconnect PIR from ESP RST
  if (this->pir_dis_pin_ != nullptr) {
    this->pir_dis_pin_->setup();
    this->pir_dis_pin_->digital_write(true);  // HIGH = PIR disconnected
    ESP_LOGD(TAG, "PIR_DIS (GPIO15) set to HIGH - PIR disconnected from reset");
  } else {
    ESP_LOGE(TAG, "PIR_DIS pin not configured!");
  }

  // CRITICAL: Set PERIPH_VCC to LOW to power ON peripherals (P-MOSFET conducts)
  if (this->periph_vcc_pin_ != nullptr) {
    this->periph_vcc_pin_->setup();
    this->periph_vcc_pin_->digital_write(false);  // LOW = power ON
    ESP_LOGD(TAG, "PERIPH_VCC (GPIO0) set to LOW - peripherals powered ON");
  } else {
    ESP_LOGE(TAG, "PERIPH_VCC pin not configured!");
  }
}

void AthermoComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "ATHERMO Component:");
  LOG_PIN("  PIR_DIS Pin: ", this->pir_dis_pin_);
  LOG_PIN("  PERIPH_VCC Pin: ", this->periph_vcc_pin_);
  ESP_LOGCONFIG(TAG, "  Boot State: PIR_DIS=HIGH, PERIPH_VCC=LOW (power ON)");
}

// Turn peripherals ON (P-MOSFET conducts when pin is LOW)
void AthermoComponent::periph_vcc_on() {
  if (this->periph_vcc_pin_ != nullptr) {
    this->periph_vcc_pin_->digital_write(false);  // LOW = power ON
    ESP_LOGD(TAG, "Peripherals power ON (PERIPH_VCC = LOW)");
  }
}

// Turn peripherals OFF (P-MOSFET off when pin is HIGH)
void AthermoComponent::periph_vcc_off() {
  if (this->periph_vcc_pin_ != nullptr) {
    this->periph_vcc_pin_->digital_write(true);  // HIGH = power OFF
    ESP_LOGD(TAG, "Peripherals power OFF (PERIPH_VCC = HIGH)");
  }
}

// Power cycle: OFF -> delay -> ON (useful for resetting frozen OLED/sensors)
void AthermoComponent::power_cycle(uint32_t delay_ms) {
  ESP_LOGD(TAG, "Power cycling peripherals (delay: %u ms)", delay_ms);
  
  this->periph_vcc_off();
  delay(delay_ms);
  this->periph_vcc_on();
  
  ESP_LOGD(TAG, "Power cycle complete");
}

}  // namespace athermo
}  // namespace esphome
