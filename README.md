# Robot Mower Interface

## Overview
This project provides an interface for controlling and managing a robot mower.

Initially this is done for a Ferrex R800Easy+ robot mower (meaning, it will be probably easily adaptable to other models from Ferrex, Yard Force (Yardforce), Garden Feelings and so on). 

## Original Goals
- The mower lacks the possibility to mow less than 4 hours a day. Also it has no app to control it (other models may have an app, but only closed source). These are the main reasons for this project.
- The main goal is a personal use case. There still might be errors in the project and/or in the documentation.

## Features
- **Control**: Execute Start, Home, Stop, and Lock on the mower.
- **Configuration**: Configure Wi-Fi settings (either connected to a Wifi, or as Access Point).
- **Scheduling**: Set mowing schedules.
- **Status**: View mower status
- **Logs**: See logs from the mower
- **API** Access all features via API
- **Web Interface** Access all features via a web interface

## Demo of first running version
[![Demo](https://img.youtube.com/vi/VGW71knz1Dk/0.jpg)](https://www.youtube.com/watch?v=VGW71knz1Dk)

## License
This project is licensed under the Creative Commons Zero v1.0 Universal (CC0 1.0) - see the [LICENSE](LICENSE) file for details.

It's important to note that the original firmware of the mower is not open source, and this project is not affiliated with the manufacturer.
The original firmware of the mower is not changed in any way, but the warranty of the mower will be voided modifying the mower in this way.
I am not responsible for any damage to the mower or any other device.

## API Endpoint Documentation

### 1. `/start`
- **Method:** `POST`
- **Description:** Starts the mower.
- **Parameters:** None
- **Response:** `200 OK` if successful

### 2. `/home`
- **Method:** `POST`
- **Description:** Sends the mower back to the charging station.
- **Parameters:** None
- **Response:** `200 OK` if successful

### 3. `/stop`
- **Method:** `POST`
- **Description:** Stops the mower.
- **Parameters:** None
- **Response:** `200 OK` if successful

### 4. `/lock`
- **Method:** `POST`
- **Description:** Locks the mower.
- **Parameters:** None
- **Response:** `200 OK` if successful

### 5. `/unlock`
- **Method:** `POST`
- **Description:** Unlocks the mower.
- **Parameters:** None
- **Response:** `200 OK` if successful

### 6. `/status`
- **Method:** `GET`
- **Description:** Returns the current status of the mower.
- **Parameters:** None
- **Response:** JSON object with the following fields:
    - `date`: Current date on the mower
    - `time`: Current time on the mower
    - `isCharging`: Charging status
    - `isLocked`: Lock status
    - `isEmergency`: Emergency status
    - `isIdle`: Idle status
    - `isAccessPoint`: Access Point mode status
    - `hostname`: Hostname of the mower
    - `ssid`: Current WiFi SSID
    - `ip`: IP address of the mower
    - `mowingPlanActive`: Indicates if the mowing plan is active

### 7. `/mowing-plan`
- **Method:** `GET`
- **Description:** Retrieves the current mowing plan.
- **Parameters:** None
- **Response:** JSON file of the mowing plan if exists, or `204 No Content` if not found

### 8. `/mowing-plan`
- **Method:** `POST`
- **Description:** Sets a new mowing plan.
- **Payload:** JSON object with the following fields:
    - `customMowingPlanActive` (boolean): Activates or deactivates the custom mowing plan
    - `days` (array of booleans): Specifies active days (Mon-Sun)
    - `planTimeStart` (string): Mowing start time (e.g., `"09:00"`)
    - `planTimeEnd` (string): Mowing end time (e.g., `"17:00"`)
- **Response:** `200 OK` if successful, `400 Bad Request` if parameters are missing

### 9. `/wifis`
- **Method:** `GET`
- **Description:** Returns a list of available WiFi networks.
- **Parameters:** None
- **Response:** JSON array with SSIDs of the detected networks

### 10. `/wifi`
- **Method:** `POST`
- **Description:** Sets the WiFi credentials for the mower to connect to.
- **Payload:** JSON object with the following fields:
    - `ssid` (string): WiFi SSID
    - `password` (string): WiFi password
- **Response:** `200 OK` if successful, `400 Bad Request` if parameters are missing

### 11. `/date-time`
- **Method:** `POST`
- **Description:** Sets the date and time on the mower.
- **Payload:** JSON object with the following fields:
    - `date` (string): Date in the format `YYYY-MM-DD`
    - `time` (string): Time in the format `HH:MM`
- **Response:** `200 OK` if successful, `400 Bad Request` if parameters are missing

### 12. `/update`
- **Method:** `POST`
- **Description:** Allows for simultaneous upload and update of both firmware and filesystem files on the device. This endpoint manages both types of updates, determining the file type based on the filename.
- **Payload:** Multipart form-data with up to two files:
  - `firmware.bin`: Firmware binary file for updating the main program.
  - `filesystem.bin`: Filesystem binary file (e.g., SPIFFS) for updating the file system.
- **Response:**
  - **`200 OK`**: Returns `"Update Success!"` if the upload and update process are successful for both files.
  - **`500 Internal Server Error`**: Returns `"Update Failed!"` if an error occurs during either update.
- **Usage Notes:**
  - **File Recognition**: The update type is determined by the filename—`firmware.bin` for firmware updates and `filesystem.bin` for filesystem updates.
  - **Restart**: Upon successful updates, the device automatically restarts to apply changes.

## Needed parts
- Ferrex R800Easy+ robot mower (or similar)
- ESP32 (e.g., ESP32 DevKitC)
- USB cable
- DC-DC step-down converter (~32V -> 5V)

## Software Installation Instructions
Follow these steps to install the mower control software on your ESP32:

- **Connect** your ESP32 to your computer via USB.
- **Clone** this Repository
- **Install** PlatformIO (if not installed)
- **Upload the File System:** In the project/backend directory, upload the filesystem to the ESP32 by running:
  ```bash
  pio run --target uploadfs
  ```
- **Upload the Firmware:** In the project/backend directory, upload the firmware to the ESP32:
  ```bash
  pio run --target upload
  ```

- **Verify Installation:** Once complete, the ESP32 should be ready to run with the mower control software, creating a Wifi Access Point with SSID "Robot Mower" and password: "MowerInterface".
- **While still connected with USB to the ESP32, open a Serial Monitor** and you should see the IP address of the ESP32. Use this IP address to access the web interface.
  ```bash
  pio device monitor
  ```
  

## Hardware Installation Instructions
- For now the ESP32 has to be powered from the mainboard, and several pins of the ESP32 have to be connected to the Cover-User-Interface-Board (CoverUI).
- The power (24V) will come from red J18 connector. This is constant and is not powered down in idle mode. Connect it to the DC-DC step-down converter and set this to 5V.
- The ESP32 will be powered from the 5V output of the DC-DC step-down converter.
- The Stop1 (J8) cable on the main board for now has to be disconnected. The bottom Stop button of the mower will not work anymore. The top Stop button will still work.
- The connection of the pins of the ESP32 will for now not be described here, but hopefully can be seen in the following pictures, or derived from the code.

## Photos
![1](documentation/assets/IMG_0177.jpg)
![1](documentation/assets/IMG_0178.jpg)
![1](documentation/assets/IMG_0179.jpg)
![1](documentation/assets/IMG_0182.jpg)


## Thanks To
Special thanks to the following projects and contributors:
- **[apehaenger/CoverUI](https://github.com/apehaenger/CoverUI)** — for extensive help on the mower and detailed documentation.
- **[ClemensElflein/OpenMower](https://github.com/ClemensElflein/OpenMower)** — for the inspiration behind this project.
- **[cedbossneo/Mowgli](https://github.com/cedbossneo/Mowgli)** — for mainboard documentation and insights.

And a big thanks to all contributors from these projects, as well as the developers of the hardware and software used in this project.

Last but not least, to my girlfriend and family for their patience and support during my development time.