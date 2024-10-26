To implement the settings storage on the SD card, replace the 'mkdir_for_file'  with the  following function  that can be found in the  Zephyr core file - `settings_file.c`, located at `zephyr/subsys/settings/src/settings_file.c`.


static int mkdir_for_file(const char *file_path)
{
    char dir_path[SETTINGS_FILE_NAME_MAX];
    int err;

    for (size_t i = 0; file_path[i] != '\0'; i++) {
        if (i > 0 && file_path[i] == '/') {
            dir_path[i] = '\0';

            // Skip mkdir for FatFS root directories (ending with ':')
            if (strrchr(dir_path, ':') == &dir_path[strlen(dir_path) - 1]) {
                LOG_DBG("FatFS root directory detected, skipping mkdir for path: %s",
                        dir_path);
            } else {
                err = mkdir_if_not_exists(dir_path);
                if (err) {
                    return err;
                }
            }
        }
        dir_path[i] = file_path[i];
    }

    return 0;
}