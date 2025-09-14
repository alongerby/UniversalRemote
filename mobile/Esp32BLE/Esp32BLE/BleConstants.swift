//
//  BLEConstants.swift
//  Esp32BLE
//
//  Created by Alon Gerby on 01/09/2025.
//
import Foundation
import CoreBluetooth

enum BleUUID {
    static let service = CBUUID(string: "6E400001-B5A3-F393-E0A9-E50E24DCCA9E")   
    static let txCharacteristic = CBUUID(string: "6E400002-B5A3-F393-E0A9-E50E24DCCA9E") // ESP32 <- iPhone
    static let rxCharacteristic = CBUUID(string: "6E400003-B5A3-F393-E0A9-E50E24DCCA9E") // iPhone <- ESP32 (optional)
    static let customBut = CBUUID(string: "6E400004-B5A3-F393-E0A9-E50E24DCCA9E") // custom button
}
