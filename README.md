# ESPHome ATHERMO Component

ATB-THERMO power management component for ESPHome.

## Installation
```yaml
external_components:
  - source: github://atnel/athermo-component
    components: [ athermo ]

athermo:
  pir_dis_pin: GPIO15
  periph_vcc_pin: GPIO0
```

## Documentation

See [components/athermo/README.md](components/athermo/README.md)