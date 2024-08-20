/**
 *  @file   wifi_signal.swift
 *  @brief  Get WiFi Signal Strength
 *  @author KrizTioaN (christiaanboersma@hotmail.com)
 *  @date   2024-08-04
 *  @note   BSD-3 licensed
 *
 *  Compile with:
 *    swiftc -o wifi_signal wifi_signal.swift
 *
 ***********************************************/

import Foundation
import CoreWLAN

let interface = CWWiFiClient.shared().interface()

let signal = Int(interface!.transmitRate())
let ssid = interface!.ssid() ?? "-"

print("{\"ssid\": \"\(ssid)\", \"signal\": \(signal), \"error\": 0}")
