// src/mockService.js
import axios from 'axios';
import MockAdapter from 'axios-mock-adapter';

const mock = new MockAdapter(axios);


mock.onGet('/status').reply(function(config) {
    return new Promise(function(resolve, reject) {
        setTimeout(function() {
            resolve([200, {
                //"date": "2024-10-23",
                //"time": "14:30",
                "date": null,
                "time": null,
                "isCharging": false,
                "isLocked": false,
                "isEmergency": false,
                "isIdle": true,
                "isAccessPoint": true,
                "ssid": "MyWifiSSID",
                "ip": "192.168.1.100",
                "mowingPlanActive": true
            }]);
        }, 1500);
    });
});

mock.onPost('/start').reply(200);
mock.onPost('/stop').reply(200);
mock.onPost('/home').reply(200);
mock.onPost('/lock').reply(200);
mock.onPost('/unlock').reply(200);


mock.onGet('/mowing-plan').reply(function(config) {
    return new Promise(function(resolve, reject) {
        setTimeout(function() {
            resolve([200, {
                "customMowingPlanActive": true,
                "days": [
                    true,
                    true,
                    true,
                    true,
                    true,
                    false,
                    false
                ],
                "startTime": "08:00",
                "endTime": "10:00"
            }]);
        }, 1500);
    });
});

mock.onPost('/mowing-plan').reply(200);
mock.onGet('/wifis').reply(function(config) {
    return new Promise(function(resolve, reject) {
        setTimeout(function() {
            resolve([200, [
                'FRITZ!Box 7590 XYZ',
                'FRITZ!Box 7590 ABC',
                'FRITZ!Box 7590 DEF',
                'Speedport W724V XYZ',
                'Speedport W724V XYZ'
            ]]);
        }, 1500);
    });
});
mock.onPost('/wifi').reply(200);
mock.onPost('/date-and-time').reply(200);
mock.onGet('/log-messages').reply(
    200,
    '[Time not available] Scanning wifis..\n' +
    '[Time not available] Try to connect with Wifi:\n' +
    '[Time not available] FRITZ!Box 7590 XYZ\n' +
    '[Time not available] Password:\n' +
    '[Time not available] ********************\n' +
    '[Time not available] Connected!\n' +
    '[Time not available] Trying to synchronize time from NTP server\n' +
    '[Wed, 24/10/23 11:08:53] HTTP-Server started\n' +
    '[Wed, 24/10/23 11:08:53] Mowing Plan:\n' +
    '[Wed, 24/10/23 11:08:53] Failed to read file /mowing_plan.json, using default settings\n' +
    '[Wed, 24/10/23 11:08:53] Checking automatic start or sending home required\n' +
    '[Wed, 24/10/23 11:08:53] No custom mowing plan active'
);
