/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include <app/drivers/blink.h>

#include <app_version.h>

#include <zephyr/settings/settings.h>
#include <inttypes.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

#define BLINK_PERIOD_MS_STEP 100U
#define BLINK_PERIOD_MS_MAX 1000U


#define DEFAULT_FOO_VAL_VALUE 1

static uint8_t foo_val = DEFAULT_FOO_VAL_VALUE;

static int foo_settings_set(const char *name, size_t len,
                            settings_read_cb read_cb, void *cb_arg)
{
    const char *next;
    int rc;

    printk("loading the settings\n");

    if (settings_name_steq(name, "bar", &next) && !next) {
        if (len != sizeof(foo_val)) {
            return -EINVAL;
        }

        rc = read_cb(cb_arg, &foo_val, sizeof(foo_val));
        if (rc >= 0) {
            /* key-value pair was properly read.
             * rc contains value length.
             */            
            printf("foo: %d\n", foo_val);
            return 0;
        }
        else{
            printk("error reading settings  %d\n", rc);
        }
        /* read-out error */
        return rc;
    }

    return -ENOENT;
}

static int foo_settings_export(int (*storage_func)(const char *name,
                                                   const void *value,
                                                   size_t val_len))
{
    return storage_func("foo/bar", &foo_val, sizeof(foo_val));
}

struct settings_handler my_conf = {
    .name = "foo",
    .h_set = foo_settings_set,
    .h_export = foo_settings_export
};



int main(void)
{
  int err;
  int ret;
  unsigned int period_ms = BLINK_PERIOD_MS_MAX;
  const struct device *sensor, *blink;
  struct sensor_value last_val = {0}, val;


  err = settings_subsys_init();
    if (err) {
        printk("Settings subsystem init failed (err %d)\n", err);
        return 1;
    }
    LOG_INF("Settings subsystem init done\n");

    err = settings_register(&my_conf);    
    if (err) {
        printk("Failed to register settings handler: %d\n", err);
    } else {
        printk("Settings handler registered\n");
    }
    LOG_INF("Settings handler registered\n");

    err = settings_load();
    if (err) {
        printk("Settings load failed (err %d)\n", err);
        return 1; // Handle the error as needed
    } else {
        printk("Settings loaded successfully\n");
    }
    LOG_INF("Settings loaded successfully\n");

   foo_val++;
    err = settings_save_one("foo/bar", &foo_val, sizeof(foo_val));
    if (err) {
        LOG_ERR("Settings save failed (err %d)\n", err);
    } else {
        LOG_DBG("foo_val updated and saved: %d\n", foo_val);
    }

    k_msleep(1000);
    



  printk("Firmware Template %s\n", APP_VERSION_STRING);
  LOG_DBG("Logging works.");

  sensor = DEVICE_DT_GET(DT_NODELABEL(example_sensor));
  if (!device_is_ready(sensor))
  {
    LOG_ERR("Sensor not ready");
    return 0;
  }

  blink = DEVICE_DT_GET(DT_NODELABEL(blink_led));
  if (!device_is_ready(blink))
  {
    LOG_ERR("Blink LED not ready");
    return 0;
  }

  ret = blink_off(blink);
  if (ret < 0)
  {
    LOG_ERR("Could not turn off LED (%d)", ret);
    return 0;
  }

  printk("Use the sensor to change LED blinking period\n");

  while (1)
  {
    ret = sensor_sample_fetch(sensor);
    if (ret < 0)
    {
      LOG_ERR("Could not fetch sample (%d)", ret);
      return 0;
    }

    ret = sensor_channel_get(sensor, SENSOR_CHAN_PROX, &val);
    if (ret < 0)
    {
      LOG_ERR("Could not get sample (%d)", ret);
      return 0;
    }

    if ((last_val.val1 == 0) && (val.val1 == 1))
    {
      if (period_ms == 0U)
      {
        period_ms = BLINK_PERIOD_MS_MAX;
      }
      else
      {
        period_ms -= BLINK_PERIOD_MS_STEP;
      }

      printk("Proximity detected, setting LED period to %u ms\n",
             period_ms);
      blink_set_period_ms(blink, period_ms);
    }

    last_val = val;

    k_sleep(K_MSEC(100));
  }

  return 0;
}
