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
#include <zephyr/storage/disk_access.h>
#include <zephyr/fs/fs.h>


#define CONFIG_SETTINGS_FILE_DIR "/SD:/settings"

#if defined(CONFIG_FAT_FILESYSTEM_ELM)
#include <ff.h>
/*
 *  Note the fatfs library is able to mount only strings inside _VOLUME_STRS
 *  in ffconf.h
 */
#define DISK_DRIVE_NAME "SD"
#define DISK_MOUNT_PT "/"DISK_DRIVE_NAME":"

static FATFS fat_fs;
/* mounting info */
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
};

#elif defined(CONFIG_FILE_SYSTEM_EXT2)

#include <zephyr/fs/ext2.h>

#define DISK_DRIVE_NAME "SDMMC"
#define DISK_MOUNT_PT "/ext"

static struct fs_mount_t mp = {
	.type = FS_EXT2,
	.flags = FS_MOUNT_FLAG_NO_FORMAT,
	.storage_dev = (void *)DISK_DRIVE_NAME,
	.mnt_point = "/ext",
};

#endif
#if defined(CONFIG_FAT_FILESYSTEM_ELM)
#define FS_RET_OK FR_OK
#else
#define FS_RET_OK 0
#endif


LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);
#define BLINK_PERIOD_MS_STEP 100U
#define BLINK_PERIOD_MS_MAX 1000U
#define DEFAULT_FOO_VAL_VALUE 1

static uint8_t foo_val = DEFAULT_FOO_VAL_VALUE;
static const char *disk_mount_pt = DISK_MOUNT_PT;
static int lsdir(const char *path);

/**
 * @brief Set a setting value.
 *
 * @param name Setting name.
 * @param len Data length.
 * @param read_cb Callback to read value.
 * @param cb_arg Callback argument.
 * @return 0 on success, negative on error.
 */
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

/**
 * @brief Export setting value.
 *
 * @param storage_func Function to store value.
 * @return 0 on success, negative on error.
 */
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


/**
 * @brief Main function of the application.
 *
 * This function initializes the SD card, mounts the filesystem, initializes
 * the settings subsystem, and sets up the sensor and LED for operation.
 * It enters a loop to read sensor data and adjust the LED blinking period.
 *
 * @return int 0 on success, non-zero error code on failure.
 */
int main(void)
{
  int err;
  int ret;
  unsigned int period_ms = BLINK_PERIOD_MS_MAX;
  const struct device *sensor, *blink;
  struct sensor_value last_val = {0}, val;

  static const char *disk_pdrv = DISK_DRIVE_NAME;
  uint64_t memory_size_mb;
  uint32_t block_count;
  uint32_t block_size;

  /************** Initialize SD card **************/

  if (disk_access_ioctl(disk_pdrv,DISK_IOCTL_CTRL_INIT, NULL) != 0) 
  {
    LOG_ERR("Storage init ERROR!");
    return 1;
  }

  if (disk_access_ioctl(disk_pdrv,
      DISK_IOCTL_GET_SECTOR_COUNT, &block_count)) {
    LOG_ERR("Unable to get sector count");
    return 1;
  }
  LOG_INF("Block count %u", block_count);

  if (disk_access_ioctl(disk_pdrv,
      DISK_IOCTL_GET_SECTOR_SIZE, &block_size)) {
    LOG_ERR("Unable to get sector size");
    return 1;
  }
  printk("Sector size %u\n", block_size);

  memory_size_mb = (uint64_t)block_count * block_size;
  printk("Memory Size(MB) %u\n", (uint32_t)(memory_size_mb >> 20));

  if (disk_access_ioctl(disk_pdrv,
      DISK_IOCTL_CTRL_DEINIT, NULL) != 0) {
    LOG_ERR("Storage deinit ERROR!");
    return 1;
  }

	mp.mnt_point = disk_mount_pt;

	int res = fs_mount(&mp);

	if (res == FS_RET_OK) 
  {
		printk("Disk mounted.\n");

    /*might requires to create settings directory whever the sd is formated*/

    //  res = fs_mkdir(CONFIG_SETTINGS_FILE_DIR);
    // if (res != 0 && res != -EEXIST) {
    //     printk("Failed to create settings directory (err %d)\n", res);
    //     return res;
    // }

    if (lsdir(disk_mount_pt) == 0){
      printk("Error mounting disk.\n");
    }



    /************** Initialize settings subsystem **************/

    err = settings_subsys_init();
    if (err) {
        printk("Settings subsystem init failed (err %d)\n", err);
        return err;
    }
    LOG_INF("Settings subsystem init done\n");
    
  }

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
      printk("Settings save failed (err %d)\n", err);
  } else {
      LOG_DBG("foo_val updated and saved: %d\n", foo_val);
      printk("foo_val updated and saved: %d\n", foo_val);
  }

  k_msleep(1000);
    

/************** Blinck & sensor initialization **************/

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

/**
 * @brief List the contents of a directory.
 *
 * This function opens a directory specified by the given path, reads its
 * contents, and prints the names of the files and directories it contains.
 * It counts the number of entries and returns this count. If an error occurs
 * during opening or reading the directory, an error code is returned.
 *
 * @param path The path to the directory to be listed.
 * @return int The number of entries in the directory, or a negative error code
 *         if an error occurs.
 */
static int lsdir(const char *path)
{
	int res;
	struct fs_dir_t dirp;
	static struct fs_dirent entry;
	int count = 0;

	fs_dir_t_init(&dirp);

	/* Verify fs_opendir() */
	res = fs_opendir(&dirp, path);
	if (res) {
		printk("Error opening dir %s [%d]\n", path, res);
		return res;
	}

	printk("\nListing dir %s ...\n", path);
	for (;;) {
		/* Verify fs_readdir() */
		res = fs_readdir(&dirp, &entry);

		/* entry.name[0] == 0 means end-of-dir */
		if (res || entry.name[0] == 0) {
			break;
		}

		if (entry.type == FS_DIR_ENTRY_DIR) {
			printk("[DIR ] %s\n", entry.name);
		} else {
			printk("[FILE] %s (size = %zu)\n",
				entry.name, entry.size);
		}
		count++;
	}

	/* Verify fs_closedir() */
	fs_closedir(&dirp);
	if (res == 0) {
		res = count;
	}

	return res;
}
