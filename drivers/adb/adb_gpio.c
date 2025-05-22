#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(adb_gpio, CONFIG_ADB_GPIO_LOG_LEVEL);

struct adb_gpio_config {
    const struct gpio_dt_spec gpio;
};

static void adb_gpio_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    LOG_INF("ADB GPIO change detected (pins=0x%X)", pins);
    // TODO: ADB state machine or protocol logic here
}

static struct gpio_callback gpio_cb;

static int adb_gpio_init(const struct device *dev)
{
    const struct adb_gpio_config *cfg = dev->config;

    if (!device_is_ready(cfg->gpio.port)) {
        LOG_ERR("GPIO device not ready");
        return -ENODEV;
    }

    int ret = gpio_pin_configure_dt(&cfg->gpio, GPIO_INPUT);
    if (ret < 0) {
        LOG_ERR("Failed to configure GPIO pin");
        return ret;
    }

    ret = gpio_pin_interrupt_configure_dt(&cfg->gpio, GPIO_INT_EDGE_BOTH);
    if (ret < 0) {
        LOG_ERR("Failed to configure GPIO interrupt");
        return ret;
    }

    gpio_init_callback(&gpio_cb, adb_gpio_callback, BIT(cfg->gpio.pin));
    gpio_add_callback(cfg->gpio.port, &gpio_cb);

    LOG_INF("ADB GPIO driver initialized");

    return 0;
}

#define ADB_GPIO_INIT(inst) \
    static const struct adb_gpio_config adb_gpio_config_##inst = { \
        .gpio = GPIO_DT_SPEC_INST_GET(inst, gpio), \
    }; \
    DEVICE_DT_INST_DEFINE(inst, adb_gpio_init, NULL, NULL, \
                          &adb_gpio_config_##inst, \
                          POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE, NULL);

DT_INST_FOREACH_STATUS_OKAY(ADB_GPIO_INIT)
