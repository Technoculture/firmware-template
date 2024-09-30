# Firmware Template

<a href="https://github.com/technoculture/firmware-template/actions/workflows/build.yml?query=branch%3Amain">
  <img src="https://github.com/technoculture/firmware-template/actions/workflows/build.yml/badge.svg?event=push">
</a>
<!-- <a href="https://github.com/technoculture/firmware-template/actions/workflows/docs.yml?query=branch%3Amain">
  <img src="https://github.com/technoculture/firmware-template/actions/workflows/docs.yml/badge.svg?event=push">
</a>
<a href="https://technoculture.github.io/firmware-template/">
  <img alt="Documentation" src="https://img.shields.io/badge/documentation-3D578C?logo=sphinx&logoColor=white">
</a>
<a href="https://technoculture.github.io/firmware-template/doxygen">
  <img alt="API Documentation" src="https://img.shields.io/badge/API-documentation-3D578C?logo=c&logoColor=white">
</a> -->
<br />
<img width="360" alt="shape_3FWSg8mxSBF3motfyvrGN at 24-08-13 17 33 10" src="https://github.com/user-attachments/assets/c300b66f-24a9-4c4f-a74e-2d8c7c23c4da">

This repository contains a Zephyr firmware template. The main purpose of this
repository is to serve as a reference on how to structure Zephyr-based
applications. Some of the features demonstrated in this example are:

- Basic [Zephyr application][app_dev] skeleton
- [Zephyr workspace applications][workspace_app]
- [Zephyr modules][modules]
- [West T2 topology][west_t2]
- [Custom boards][board_porting]
- Custom [devicetree bindings][bindings]
- Out-of-tree [drivers][drivers]
- Out-of-tree libraries
- Example CI configuration (using GitHub Actions)
- Custom [west extension][west_ext]
- Doxygen and Sphinx documentation boilerplate

This repository is versioned together with the [Zephyr main tree][zephyr]. This
means that every time that Zephyr is tagged, this repository is tagged as well
with the same version number, and the [manifest](west.yml) entry for `zephyr`
will point to the corresponding Zephyr tag. For example, the `example-application`
v2.6.0 will point to Zephyr v2.6.0. Note that the `main` branch always
points to the development branch of Zephyr, also `main`.

[app_dev]: https://docs.zephyrproject.org/latest/develop/application/index.html
[workspace_app]: https://docs.zephyrproject.org/latest/develop/application/index.html#zephyr-workspace-app
[modules]: https://docs.zephyrproject.org/latest/develop/modules.html
[west_t2]: https://docs.zephyrproject.org/latest/develop/west/workspaces.html#west-t2
[board_porting]: https://docs.zephyrproject.org/latest/guides/porting/board_porting.html
[bindings]: https://docs.zephyrproject.org/latest/guides/dts/bindings.html
[drivers]: https://docs.zephyrproject.org/latest/reference/drivers/index.html
[zephyr]: https://github.com/zephyrproject-rtos/zephyr
[west_ext]: https://docs.zephyrproject.org/latest/develop/west/extensions.html

## Getting Started

Before getting started, make sure you have a proper Zephyr development
environment. Follow the official
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html).

### Initialization

The first step is to initialize the workspace folder (``my-workspace``) where
the ``firmware-template`` and all Zephyr modules will be cloned. Run the following
command:

```shell
# initialize my-workspace for the panomic-application (main branch)
west init -m https://github.com/Technoculture/firmware-template --mr main panomic-workspace
# update Zephyr modules
cd panomic-workspace
west update
```

### Building the application and running

To build the application, run the following command:

```shell
cd panomic-workspace
export BOARD=panomic_board
west build -p auto -b $BOARD app -d build-slot0
```

where `$BOARD` is the target board.

You can use the `panomic_board` board found in this
repository. Note that Zephyr sample boards may be used if an
appropriate overlay is provided (see `app/boards`).

A sample debug configuration is also provided. To apply it, run the following
command:

```shell
west build -b $BOARD app -- -DOVERLAY_CONFIG=debug.conf
```

Once you have built the application, run the following command to flash it:

```shell
west flash -r jlink
```

In order to debug the program using gdb, run the following command:

```shell
west debug -r jlink
```

When accessing the shell or console, use USART3 on the Olimexe407 board
<br />
<img width="360" alt="uart" src="https://github.com/user-attachments/assets/81d9f513-b26d-4965-b07d-c114b8efe631">


### Building the MCUBoot BootLoader

To build the bootloader, run the following command:

```shell
cd panomic-workspace
export BOARD=panomic_board
west build -p always -b $BOARD ./bootloader/mcuboot/boot/zephyr -d build-mcuboot -DDTC_OVERLAY_FILE='../../../app/boards/panomic_board_bootloader.overlay'
```

where `$BOARD` is the target board.


### Building the application on build-slot1 folder

To build the application, run the following command:

```shell
cd panomic-workspace
export BOARD=panomic_board
west build -p auto -b $BOARD app -d build-slot1
```

where `$BOARD` is the target board.

Note: the build command and configurations for the slot1 are exactly the same used for slot0 but the building path. This because the ratio is that you continue to dev for slot0 and use it in future DFU.
The existance of slot1 build directory is for testing purposes only.


### Testing

To execute Twister integration tests, run the following command:

```shell
west twister -T tests --integration
```

### Documentation

A minimal documentation setup is provided for Doxygen and Sphinx. To build the
documentation first change to the ``docs`` folder:

```shell
cd docs
```

Before continuing, check if you have Doxygen installed. It is recommended to
use the same Doxygen version used in [CI](.github/workflows/docs.yml). To
install Sphinx, make sure you have a Python installation in place and run:

```shell
pip install -r requirements.txt
```

API documentation (Doxygen) can be built using the following command:

```shell
doxygen
```

The output will be stored in the ``_build_doxygen`` folder. Similarly, the
Sphinx documentation (HTML) can be built using the following command:

```shell
make html
```

The output will be stored in the ``_build_sphinx`` folder. You may check for
other output formats other than HTML by running ``make help``.
