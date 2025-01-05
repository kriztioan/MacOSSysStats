/**
 *  @file   wifi.swift
 *  @brief  Get WiFi Information
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2024-08-04
 *  @note   BSD-3 licensed
 *
 *  Pre-compile:
 *    mkdir -p WiFi.app/Contents/MacOS
 *
 *  Compile with:
 *    swiftc -o WiFi.app/Contents/MacOS/wifi wifi.swift
 *
 *  Post-compile:
 *    Allow Location Services for WiFI in System
 *    Preferences
 *
 ***********************************************/

import Cocoa
import Foundation
import CoreWLAN
import CoreLocation


class AppDelegate: NSObject, NSApplicationDelegate, CLLocationManagerDelegate {

    private var locationManager:CLLocationManager?

    func applicationDidFinishLaunching(_ aNotification: Notification) {

        locationManager = CLLocationManager()

        locationManager?.delegate = self

        locationManager?.requestAlwaysAuthorization()
    }

    func locationManager(_ manager: CLLocationManager, didFailWithError error: Error) {

        print("Error: \(error.localizedDescription)")
    }

    func locationManager(_ manager: CLLocationManager, didChangeAuthorization status: CLAuthorizationStatus) {

        var data: [String: String] = [:]

        if status == .authorizedAlways || status == .authorized {

            if let interface = CWWiFiClient.shared().interface() {

                if let ssid = interface.ssid() {

                    data["ssid"] = ssid
                } else {

                    data["ssid"] = "-"
                }

                if let bssid = interface.bssid() {

                    data["bssid"] = bssid
                } else {

                    data["bssid"] = "-"
                }

                data["rssi"] = String(format:"%d dBm", interface.rssiValue())

                data["tx"] = String(format:"%.0f Mbps", interface.transmitRate())

                data["error"] = "0"
            } else {

              data["error"] = "2"
            }
        } else {

            data["error"] = "1"
        }

        if let json = try? JSONSerialization.data(withJSONObject: data),

            let output = String(data: json, encoding: .utf8) {

                print(output)
        }

        NSApp.terminate(nil)
   }
}

let app = NSApplication.shared

let delegate = AppDelegate()

app.delegate = delegate

app.run()
