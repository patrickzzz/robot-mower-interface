# Changelog

All notable changes to this project are documented in this file.

## [0.3.1]
### Changed
- Visualize Log message in webinterface as mono spaced text
- Load mowing plan on boot up again

## [0.3.0]
### Added
- Ability to update the firmware+filesystem of the mower interface over the air (OTA) in Webinterface or API Endpoint.

## [0.2.2]
### Added
- Automatically reconnect to Wifi if connection is lost

## [0.2.1]
### Changed
- Forced serving of html+js gzipped
- Increased wdt to 60s for handling bigger requests (hopefully not needed)

## [0.2.0]
### Added
- Documentation added.

### Changed
- Changed license from MIT to CC0 1.0 Universal.
- Minor improvements in frontend.
- Backend code restructuring.
- WiFi scanning now asynchronous.

### Fixed
- Issue with logging (log file truncation error corrected).
- Time setting now expects new JSON payload.

## [0.1.0]
### Initial Release
- Initial version of the project.