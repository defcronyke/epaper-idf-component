# epaper-idf - epaper-idf-component

## An ESP-IDF component and project for e-paper displays

[![world soup remastered](https://defcronyke.gitlab.io/epaper-idf/world-soup-remastered-bw.png)](https://defcronyke.gitlab.io/epaper-idf/world-soup-remastered-bw.png)

[![pipeline status development branch](https://gitlab.com/defcronyke/epaper-idf/badges/master/pipeline.svg)](https://gitlab.com/defcronyke/epaper-idf/-/pipelines) [![pipeline status release branch](https://gitlab.com/defcronyke/epaper-idf/badges/v0.1/pipeline.svg)](https://gitlab.com/defcronyke/epaper-idf/-/commits/v0.1) [![sponsor the project](https://img.shields.io/static/v1?label=Sponsor&message=%E2%9D%A4&logo=GitHub&link=https://github.com/sponsors/defcronyke)](https://github.com/sponsors/defcronyke)

---

## [ This is a work in progress! Check back later for updates... ]

_You can test this project at your own risk if you want, but it's not ready for release yet, so please don't expect all the listed features to be available or working properly yet. Some things may not even be implemented at all yet. Check back later for new developments and updates..._

---

## How to clone this project's git repository and submodules

```shell
# Clone the current stable version (well it's not really stable yet though):
git clone -b v0.1 --recursive https://gitlab.com/defcronyke/epaper-idf.git

# (Optional) Or clone the current development version instead:
#git clone --recursive https://gitlab.com/defcronyke/epaper-idf.git
```

## How to update this project if you've already cloned it before

```shell
# You can use this bash script for convenience:
./update-git-repos.sh

# (Optional) Or use the git commands instead:
#git pull; \
#git submodule update --init --recursive
```

---

## Project Links

### Git Repository

- epaper-idf
  - [https://gitlab.com/defcronyke/epaper-idf](https://gitlab.com/defcronyke/epaper-idf)
  - [https://github.com/defcronyke/epaper-idf](https://github.com/defcronyke/epaper-idf)

### Dependencies (git submodules)

1. epaper-idf-component

   - [https://gitlab.com/defcronyke/epaper-idf-component](https://gitlab.com/defcronyke/epaper-idf-component)
   - [https://github.com/defcronyke/epaper-idf-component](https://github.com/defcronyke/epaper-idf-component)

2. Adafruit-GFX-Component (3rd-party)
   - [https://gitlab.com/defcronyke/Adafruit-GFX-Component](https://gitlab.com/defcronyke/Adafruit-GFX-Component)
   - [https://github.com/defcronyke/Adafruit-GFX-Component](https://github.com/defcronyke/Adafruit-GFX-Component)
   - Forked from this original 3rd-party Arduino-based library to add ESP-IDF support:
     - [https://github.com/adafruit/Adafruit-GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library)

## License

[Copyright © 2021](https://defcronyke.gitlab.io/epaper-idf/jeremy-profile-paint-bw.png) [Jeremy Carter](https://eternalvoid.net) `<`[jeremy@jeremycarter.ca](mailto:Jeremy%20Carter%20<jeremy@jeremycarter.ca>?subject=epaper-idf)`>`

This project is primarily released under the terms of the license contained in the file named [`LICENSE`](https://gitlab.com/defcronyke/epaper-idf/-/blob/master/LICENSE), which can be found [`in the top-level folder of this project`](https://gitlab.com/defcronyke/epaper-idf/-/blob/master/LICENSE). It also uses a bit of 3rd-party code, which is in turn primarily licensed under whichever licenses are referenced in each header (.h) or source (.c, .cpp, etc.) file, as per the original authors' preferences. A possibly non-exhaustive set of these [`other licenses`](https://gitlab.com/defcronyke/epaper-idf) is included in [`the top-level folder of this project`](https://gitlab.com/defcronyke/epaper-idf) in the files with names beginning with "`LICENSE-`".

## Details

- An [`ESP32 ESP-IDF`](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/) [`component`](https://gitlab.com/defcronyke/epaper-idf-component) for [`Espressif ESP32`](https://wikipedia.org/wiki/ESP32) [`microcontroller-based e-paper display`](https://www.waveshare.com/wiki/7.5inch_e-Paper_HAT) [`firmware projects`](https://gitlab.com/defcronyke/epaper-idf).
- Has streamlined support for WiFi HTTPS over-the-air (OTA) firmware updates.
- Aims to solve some of the usability problems that some of the other ESP32 e-paper display libraries have.
- I currently only have one model of e-paper screen to test with: [`WaveShare 7.5" 640x384 b/w Gdew075T8`](https://www.waveshare.com/wiki/7.5inch_e-Paper_HAT)
- Only ESP32 ESP-IDF (Espressif's official FreeRTOS SDK) support is planned. This will not work for Arduino framework-based projects.

I am making this as an attempt to eventually replace some existing solutions which are floating around online currently (at least replace them for my own usage), since everything I could find had too many outstanding issues, and I didn't like the way their code was organized personally.

---

## Acknowledgements

- Some of the code in this project is borrowed (and heavily modified/improved) from the OG e-paper library known as: [`ZinggJM/GxEPD`](https://github.com/ZinggJM/GxEPD)
- Some of the code is loosely inspired by this other ESP IDF component project for e-paper displays: [`martinberlin/cale-idf`](https://github.com/martinberlin/cale-idf)

A big thanks to the authors of the above projects for releasing their code with permissive licensing, so I could derive some ideas from their existing work. Both of those projects have too many bugs though, and I wanted to make my own thing instead of trying to convince them to let me patch up their projects. Maybe at some point this will be a worthwhile alternative to the above, but until then, check out those projects because they are much more mature than mine.

---

## Basic Method - Install the official pre-built version of the firmware

### How to install unmodified firmware that's already built. This method is fastest and easiest

### Prerequisites (Basic)

1. You need to have [`python`](https://www.python.org/downloads/) and the [`pip`](https://pypi.org/project/pip/) utility installed. Install those however they're meant to be installed on your OS first.
1. Run the following command to install Espressif's `esptool.py` firmware flashing utility:

   ```shell
   # Install the python-based firmware flashing tool:
   pip install esptool

   # (Optional) If the above command doesn't work, try this one:
   #python -m pip install esptool
   ```

### Instructions (Basic)

1. Plug your ESP32 device into your computer's USB port, then run one of the following commands to flash the pre-built firmware onto the device

   ```shell
   # install the release version:
   bash <(curl -sL https://tinyurl.com/epaper-idf-flash)

   # (Optional) Or install the development version:
   #bash <(curl -sL https://tinyurl.com/epaper-idf-flash) master
   ```

---

## ESP-IDF-based Method (recommended, OTA update supported) - Build custom firmware locally and install with ESP-IDF

### The usual ESP-IDF way of building and installing your own custom version of the firmware. This method is recommended because it's well-tested, but you might prefer a different method listed in another section

### Prerequisites (ESP-IDF-based)

1. Install the current stable version of Espressif's ESP32 IDF:  
   [https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html)
1. If on ESP32 IDF v4.2, their release is bugged, so you have to do the following additional steps to fix it:

   ```shell
   cd ~/esp/esp-idf && \
   git checkout remotes/origin/release/v4.2 && \
   git submodule update --init --recursive && \
   ./install.sh
   ```

### To set up the project for OTA firmware updating ability, do this once

```shell
# Generate the DH param (this takes a really long time,
# sometimes dozens of minutes):
./gen-dhparam.sh

# Generate the auth certificates. Change the "esprog" argument below to
# your dev computer's DNS hostname, or leave it as-is and set your network's
# DNS config to recognise your dev computer at the hostname "esprog":
./gen-certs.sh esprog

# Copy the new certificates onto your development machine, wherever you'll
# be hosting the OTA firmware updating procedure from. The below command's
# argument is a valid scp destination path, and the destination should be
# somewhere containing an up-to-date cloned copy of this project's git
# repository:
./copy-certs.sh $USER@dev-machine:~/epaper-idf
```

### To configure the firmware

```shell
# Source the esp-idf each time you open a new terminal instance:
. idf.env

# Open the firmware configuration (Kconfig) menu:
idf.py menuconfig
```

1. Make sure you configure the settings inside the menus named "`[<>] Project ...`".
1. You might want to change the device's "Local netif hostname" (default is "epaper") in the "Component config -> LWIP" menu as well.
1. At a minimum you'll have to select your e-paper device from the menu, otherwise compiling will give an error. You should definitely check the GPIO pin mappings while you're at it, since it's critical that you get those mappings correct if you don't want to break your e-paper display, and the defaults are likely not going to be correct for the way you wired up your devices.

### To build the firmware

```shell
# Build the firmware. The version will be set as "v0.1.0" by default:
./build.sh

# (Optional) Specify the firmware short version when building. If
# the major or minor version changes, it breaks backwards-compatibility
# on purpose by changing the name of the EpaperIDF class:
#./build.sh v0.2

# (Optional) Specify the full firmware version number instead and it
# will use this exact version number for the built firmware:
#./build.sh v0.2.0
```

### To install the firmware onto the ESP32 device

```shell
# Install the firmware you just built onto the device, and begin
# monitoring with a serial console. The firmware will be built first
# if necessary The version will be set as "v0.1.0" by default:
./flash.sh

# (Optional) Specify the firmware short version when building and
# installing. If the major or minor version changes, it breaks
# backwards-compatibility on purpose by changing the name of the
# EpaperIDF class:
#./flash.sh v0.2

# (Optional) Specify the full firmware version number instead and it
# will use this exact version number for the built firmware:
#./flash.sh v0.2.0
```

### To view the ESP32 device's serial console

```shell
# View the serial console output to see what the device is doing, for
# monitoring and debugging:
idf.py monitor
```

---

## Git Fork-based CI/CD Method (beta, no OTA support yet) - Remote build and install customized firmware

### How to fork this project with CI/CD support to remotely build your own custom firmware

### Prerequisites (Git Fork-based CI/CD)

1. You need to have [`python`](https://www.python.org/downloads/) and the [`pip`](https://pypi.org/project/pip/) utility installed. Install those however they're meant to be installed on your OS first.
1. Run the following command to install Espressif's `esptool.py` firmware flashing utility:

   ```shell
   # Install the python-based firmware flashing tool:
   pip install esptool

   # (Optional) If the above command doesn't work, try this one:
   #python -m pip install esptool
   ```

### Instructions (Git Fork-based CI/CD)

1. Fork these git repositories into your own [`GitLab`](https://gitlab.com) account, by clicking each of the following links:

   - [https://gitlab.com/defcronyke/epaper-idf](https://gitlab.com/defcronyke/epaper-idf/-/forks/new)

   - [https://gitlab.com/defcronyke/epaper-idf-component](https://gitlab.com/defcronyke/epaper-idf-component/-/forks/new)

   - [https://gitlab.com/defcronyke/Adafruit-GFX-Component](https://gitlab.com/defcronyke/Adafruit-GFX-component/-/forks/new)

1. Clone the first repo you forked in the previous step onto your machine:

   ```shell
   # Set you GitLab username and branches here:
   GITLAB_USER="defcronyke-fork"
   GIT_REPO_VERSION_BRANCH="v0.1"
   GIT_REPO_BRANCH="master"

   # Clone your GitLab repo fork:
   git clone -b $GIT_REPO_BRANCH --recursive git@gitlab.com:$GITLAB_USER/epaper-idf.git; \
   cd epaper-idf; \
   git remote add upstream https://gitlab.com/defcronyke/epaper-idf.git; \
   sed -i "s#https://github.com/defcronyke#git@gitlab.com:$GITLAB_USER#g" .gitmodules; \
   sed -i "s@gitlab.com/defcronyke/epaper-idf/badges/master@gitlab.com/$GITLAB_USER/epaper-idf/badges/$GIT_REPO_BRANCH@g" README.md; \
   sed -i "s@gitlab.com/defcronyke/epaper-idf/badges/v0.1@gitlab.com/$GITLAB_USER/epaper-idf/badges/$GIT_REPO_VERSION_BRANCH@g" README.md; \
   sed -i "s@gitlab.com/defcronyke/epaper-idf/-/pipelines@gitlab.com/$GITLAB_USER/epaper-idf/-/pipelines@g" README.md; \
   sed -i "s@gitlab.com/defcronyke/epaper-idf/-/commits/v0.1@gitlab.com/$GITLAB_USER/epaper-idf/-/commits/$GIT_REPO_VERSION_BRANCH@g" README.md; \
   cd components/epaper-idf-component; \
   git remote set-url origin git@gitlab.com:$GITLAB_USER/epaper-idf-component.git; \
   git remote add upstream https://gitlab.com/defcronyke/epaper-idf-component.git; \
   git checkout $GIT_REPO_BRANCH; \
   cp ../../README.md .; \
   cd ../Adafruit-GFX-Component; \
   git remote set-url origin git@gitlab.com:$GITLAB_USER/Adafruit-GFX-Component.git; \
   git remote add upstream https://gitlab.com/defcronyke/Adafruit-GFX-Component.git; \
   git checkout $GIT_REPO_BRANCH; \
   cd ../..
   ```

1. Modify something, then commit and push the changes to your forked repo to trigger the remote building of the firmware:

   ```shell
   # (Optional) Uncomment the following lines to override
   # the default git main and version branches:
   #GIT_REPO_BRANCH="master"
   #GIT_REPO_VERSION_BRANCH="v0.1"
   #GIT_REPO_VERSION_BRANCH_ADAFRUIT="1.10"

   # (Optional) Uncomment the following lines to create
   # a tagged release version:
   #GIT_REPO_VERSION_TAG="v0.1.0"
   #GIT_REPO_VERSION_TAG_ADAFRUIT="1.10.0"

   # Commit the changes and push them, triggering a
   # CI/CD build:
   ./commit-git-repos.sh Some changes.
   ```

1. Install the epaper-idf firmware onto the ESP32 device:

   ```shell
   # Set your GitLab username, repo, and branch here:
   GITLAB_USER="defcronyke-fork"
   GITLAB_REPO="epaper-idf"
   GIT_REPO_BRANCH="v0.1"       # release version
   #GIT_REPO_BRANCH="master"    # (Optional) or development version
   #SERIAL_PORT="/dev/ttyUSB0"  # (Optional) port to flash
   #BAUD_RATE="115200"          # (Optional) speed to flash
   #FLASH_SIZE="4MB"            # (Optional) size to flash

   # Install your forked version of the firmware that you just
   # built. You may need to wait a few minutes for the build to
   # finish first.
   bash <(curl -sL https://gitlab.com/$GITLAB_USER/$GITLAB_REPO/-/raw/$GIT_REPO_BRANCH/flash-firmware-online.sh)
   ```

1. Once the firmware is installed, the device will begin hosting a WiFi access point (AP) which you can connect to and access a configuration web page to set the WiFi `SSID` and `password` that the device will use to connect to your LAN WiFi. By default the access point will only allow one connection at a time for added safety, and it will shut down once the device is able to successfully connect to a WiFi network:

   Default WiFi Access Point Details:

   - SSID: `wifi-net-15455`
   - Password: `T3oD cOneTioN! 143 2 psS@wRiDDd$i$^s`
   - Config URL (work in progress...): [`https://192.168.4.1`](https://192.168.4.1)

---

## OTA firmware updating instructions

1. Configure your LAN's DNS to point the hostname "`esprog`" at the IP address of your firmware dev computer, or change the "`esprog`" argument to your hostname when running the "`./gen-certs.sh esprog`" script, as mentioned in an earlier section.
1. Make sure you put the auth certificates in place on your dev computer first, using the "`./copy-certs.sh`" script [`as mentioned above`](https://gitlab.com/defcronyke/epaper-idf#to-set-up-the-project-for-ota-firmware-updating-ability-do-this-once). You should have copied them into a folder containing an up-to-date version of [`this project's git repository`](https://gitlab.com/defcronyke/epaper-idf). Re-read [`the earlier instructions`](https://gitlab.com/defcronyke/epaper-idf#to-set-up-the-project-for-ota-firmware-updating-ability-do-this-once) if [`the OTA updates`](https://gitlab.com/defcronyke/epaper-idf-component/-/blob/master/epaper-idf-ota.c) aren't working properly for you.
1. Run the following script on your dev computer to build the new version of your firmware, and then begin hosting it for the device to do an OTA update:

   ```shell
   # Build the firmware if necessary, and begin serving it on an OTA
   # firmware updates HTTPS server. By default the version number will
   # start as "v0.1.0". To trigger the OTA update, the micro version
   # will be auto-incremented by +1 to the value in the file
   # version-micro.txt, for example "v0.1.0" -> "v0.1.1":
   # -----
   # KNOWN BUG: This always uses "v0.1" as the short version, so if you
   # don't want that, for now you'll have to include your desired short
   # version as an argument to this command, as in the first "(Optional)"
   # example below:
   ./serve.sh

   # (Optional) Specify the firmware short version number when building
   # and serving. If the major or minor version changes, it purposely
   # breaks backwards-compatibility by changing the name of the EpaperIDF
   # class. To trigger the OTA update, the micro version will be
   # auto-incremented by +1 to the value in the file version-micro.txt,
   # for example "v0.2.0" -> "v0.2.1":
   #./serve.sh v0.2

   # (Optional) Specify the full firmware version number when building
   # and serving. If the device is already running this version, it
   # won't do an OTA update during startup. Note that with this type
   # of invocation the micro version won't be auto-incremented, to
   # prevent an unnecessary OTA firmware update from happening:
   #./serve.sh v0.2.0
   ```

1. After the above script is finished building the firmware, it will start waiting for OTA update requests from the device. Reboot your ESP32 device to get it to connect and update itself with the new firmware version (or just wait for the deep sleep wakeup timer to fire if you're using deep sleep).

Whenever you want to load new firmware, run the "`./serve.sh`" script and wait for the firmware to finish building, then reboot the ESP32 device to load the new firmware onto it (or just wait for the deep sleep wakeup timer to fire if you're using deep sleep).

---

## Latest Highlights

_Some things listed in this section may not be fully implemented, tested, or working at all yet, but many of them are._

1. WiFi access point for hosting a WiFi connection settings web page:

   ```text
   [*] Enable WiFi Access Point (AP)
   ***  ----- WiFi Access Point Settings -----  ***
   WiFi AP Startup (After Connection Retries)  --->
   (1)     WiFi AP Connection Retries Before Starting
   (wifi-net-15455) WiFi AP SSID
   (T3oD cOneTioN! 143 2 psS@wRiDDd$i$^s) WiFi AP Password
   (192.168.4.1) WiFi AP IP Address
   (1)     WiFi AP Channel
   (1)     WiFi Maximum connections AP
   ***  ----- End WiFi Access Point Settings -----  ***
   ```

   By default the access point only comes on if the device is unable to connect to its configured WiFi network after the configured number of connection retries. You can have the access point running all the time by changing the "`WiFi Access Point Startup`" option to "`Always On`", but note that this option will disable deep sleep if it was enabled.

1. [`CI/CD Pipeline`](https://gitlab.com/defcronyke/epaper-idf/-/pipelines) added to [`the GitLab project`](https://gitlab.com/defcronyke/epaper-idf), with an easy to use [`pre-built firmware flashing method`](https://gitlab.com/defcronyke/epaper-idf#quickstart-install-the-pre-built-firmware):

   ```shell
   pip install esptool
   bash <(curl -sL https://tinyurl.com/epaper-idf-flash)
   ```

1. Lots of project-specific settings in the [`esp-idf Kconfig menu`](https://gitlab.com/defcronyke/epaper-idf-component/-/blob/master/Kconfig.projbuild), so barely anything needs to be hard-coded in [`the C and C++ source code files`](https://gitlab.com/defcronyke/epaper-idf-component):

   ```text
   [<>] Project connection config --->
   [<>] Project display config --->
   [<>] Project OTA firmware config --->
   [<>] Project task config --->
   ```

1. [`Over The Air (OTA) HTTPS updates`](https://gitlab.com/defcronyke/epaper-idf-component/-/blob/master/epaper-idf-ota.c) feature has been [`streamlined with auto-incrementing firmware micro version ( v0.1[.0] )`](https://gitlab.com/defcronyke/epaper-idf/-/blob/master/serve.sh), so it's really easy and painless to [`deploy new versions to the device`](https://gitlab.com/defcronyke/epaper-idf#ota-firmware-updating-instructions) over the air, [`using a few helper scripts`](https://gitlab.com/defcronyke/epaper-idf/-/blob/master/serve.sh):

   ```shell
   # Initial setup:
   ./gen-dhparam.sh
   ./gen-certs.sh [ota-server-hostname]
   ./copy-certs.sh [$USER@ota-server-hostname:]~/epaper-idf

   # Auto-increment micro version, build, then serve OTA:
   ./serve.sh [v0.1[.0]]
   ```

1. [`E-paper display device`](https://gitlab.com/defcronyke/epaper-idf-component/-/tree/master/include/device), [`its connections`](https://gitlab.com/defcronyke/epaper-idf-component/-/blob/master/Kconfig.projbuild#L414), and [`the desired program to run`](https://gitlab.com/defcronyke/epaper-idf-component/-/tree/master/task) on it are selected in the esp-idf [`Kconfig menu`](https://gitlab.com/defcronyke/epaper-idf-component/-/blob/master/Kconfig.projbuild):

   ```text
   Select e-paper device (Gdew075T8) --->
   (device/Gdew075T8.h) e-paper device override
   *** ----- Display Settings ----- ***
   (0) Display rotation: 0 = 0°, 1 = 90° cw, 2 = 180° 3 = 270°
   (23) SPI GPIO for MOSI (MOSI or DIN)
   (18) SPI GPIO for Clock (CLK)
   ...
   *** ----- End Display Settings ----- ***
   ```

1. The minimal required set of [`header (.h) files`](https://gitlab.com/defcronyke/epaper-idf-component/-/tree/master/include) for the selected [`e-paper device`](https://gitlab.com/defcronyke/epaper-idf-component/-/tree/master/include/device) and [`main task`](https://gitlab.com/defcronyke/epaper-idf-component/-/tree/master/include/task) are included automatically based on [`your Kconfig selections`](https://gitlab.com/defcronyke/epaper-idf-component/-/blob/master/Kconfig.projbuild), and [`the header paths/filenames can be overridden`](https://gitlab.com/defcronyke/epaper-idf-component/-/blob/master/Kconfig.projbuild#L388) in the [`Kconfig menu`](https://gitlab.com/defcronyke/epaper-idf-component/-/blob/master/Kconfig.projbuild) if you want to do something custom or weird:

   ```text
   Select project main task (http-slideshow) --->
   (task/http-slideshow.h) Project main task override
   ```

1. Easy to add your own new programs (a.k.a. "[`main tasks`](https://gitlab.com/defcronyke/epaper-idf-component/-/tree/master/task)") as options in [`the Kconfig menu`](https://gitlab.com/defcronyke/epaper-idf-component/-/blob/master/Kconfig.projbuild):

   ```text
   Select project main task (user)  --->
   (task/none.h) Project main task override (NEW)
   ***  ----- Task Settings -----  ***
   (-10) Deep sleep after refresh [delay_secs: ilong_min - -15 || deep_sleep_secs: 15 - ilong_max]
   ***  ----- End Task Settings -----  ***
   ```

1. You can [`choose to deep sleep`](https://gitlab.com/defcronyke/epaper-idf-component/-/blob/master/Kconfig.projbuild#L609) between screen refreshes to save power if you want, or you can keep everything running and use a regular task delay instead:

   - deep sleep for 15 seconds:

     ```text
     (15) Deep sleep seconds between screen refreshes [deep_sleep_secs: 15 - ilong_max]
     ```

   - (Optional) delay for 15 seconds instead, specify the number as negative:

     ```text
     (-15) Deep sleep seconds between screen refreshes [delay_secs: ilong_min - -15]
     ```

1. The first example program "[`http-slideshow`](https://gitlab.com/defcronyke/epaper-idf-component/-/blob/master/task/http-slideshow.cpp)" (work in progress) connects to [`an HTTPS web server`](https://defcronyke.gitlab.io/epaper-idf/http-slideshow/index.json) to fetch [`bitmap (.bmp) images`](https://defcronyke.gitlab.io/epaper-idf/http-slideshow/1.bmp), which will be displayed on the e-paper screen in sequence, as [`a slideshow`](https://gitlab.com/defcronyke/epaper-idf-component/-/blob/master/task/http-slideshow.cpp):

   ```text
   Select project main task (http-slideshow)  --->
   (task/http-slideshow.h) Project main task override
   ***  ----- Task Settings -----  ***
   (-10) Deep sleep after refresh [delay_secs: ilong_min - -15 || deep_sleep_secs: 15 - ilong_max]
   (https://defcronyke.gitlab.io/epaper-idf/http-slideshow/index.json) URL to a JSON object of paths to images (8-bit max .bmp)
   (defcronyke.gitlab.io) HTTP Host header value for above URL
   ***  ----- End Task Settings -----  ***
   ```

1. Adding a new e-paper [`device`](https://gitlab.com/defcronyke/epaper-idf-component/-/tree/master/device) is made easier with the help of some [`C preprocessor macros`](https://gitlab.com/defcronyke/epaper-idf-component/-/blob/master/include/epaper-idf-device.h). You can look at [`components/epaper-idf-component/include/device/Gdew075T8.h`](https://gitlab.com/defcronyke/epaper-idf-component/-/blob/master/include/device/Gdew075T8.h) for [`an example of a real device`](https://gitlab.com/defcronyke/epaper-idf-component/-/blob/master/include/device/Gdew075T8.h), and notice that you can refer to every device as "[`class EpaperIDFDevice`](https://gitlab.com/defcronyke/epaper-idf-component/-/blob/master/include/device/Gdew075T8.h#L34)", which will be properly expanded to its full name under-the-hood.

1. The idea is that you'll add more [`devices`](https://gitlab.com/defcronyke/epaper-idf-component/-/tree/master/include/device) and [`main tasks`](https://gitlab.com/defcronyke/epaper-idf-component/-/tree/master/include/task) as per the included examples, and then they'll be selected and configured through the esp-idf [`Kconfig menu`](https://gitlab.com/defcronyke/epaper-idf-component/-/blob/master/Kconfig.projbuild).
