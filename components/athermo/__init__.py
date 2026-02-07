"""ATHERMO Custom Component for ESPHome
Author: Mirosław Kardaś (ATNEL)
Purpose: Power management and PIR control for ATB-THERMO weather station

GPIO15 (PIR_DIS) = HIGH on boot (PIR disconnected from reset)
GPIO0 (PERIPH_VCC) = LOW on boot (peripherals powered ON)
"""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins, automation
from esphome.const import (
    CONF_ID,
)

DEPENDENCIES = []
AUTO_LOAD = []
MULTI_CONF = False

CONF_PIR_DIS_PIN = "pir_dis_pin"
CONF_PERIPH_VCC_PIN = "periph_vcc_pin"

athermo_ns = cg.esphome_ns.namespace("athermo")
AthermoComponent = athermo_ns.class_("AthermoComponent", cg.Component)

# Actions
PeriphVccOnAction = athermo_ns.class_("PeriphVccOnAction", automation.Action)
PeriphVccOffAction = athermo_ns.class_("PeriphVccOffAction", automation.Action)
PowerCycleAction = athermo_ns.class_("PowerCycleAction", automation.Action)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(AthermoComponent),
        cv.Required(CONF_PIR_DIS_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_PERIPH_VCC_PIN): pins.gpio_output_pin_schema,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # Configure PIR_DIS pin (GPIO15)
    pir_dis_pin = await cg.gpio_pin_expression(config[CONF_PIR_DIS_PIN])
    cg.add(var.set_pir_dis_pin(pir_dis_pin))

    # Configure PERIPH_VCC pin (GPIO0)
    periph_vcc_pin = await cg.gpio_pin_expression(config[CONF_PERIPH_VCC_PIN])
    cg.add(var.set_periph_vcc_pin(periph_vcc_pin))


# Action: periph_vcc_on
@automation.register_action(
    "athermo.periph_vcc_on",
    PeriphVccOnAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(AthermoComponent),
        }
    ),
)
async def periph_vcc_on_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)


# Action: periph_vcc_off
@automation.register_action(
    "athermo.periph_vcc_off",
    PeriphVccOffAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(AthermoComponent),
        }
    ),
)
async def periph_vcc_off_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)


# Action: power_cycle (OFF -> delay -> ON)
@automation.register_action(
    "athermo.power_cycle",
    PowerCycleAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(AthermoComponent),
            cv.Optional("delay_ms", default=500): cv.positive_int,
        }
    ),
)
async def power_cycle_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    cg.add(var.set_delay_ms(config["delay_ms"]))
    return var
