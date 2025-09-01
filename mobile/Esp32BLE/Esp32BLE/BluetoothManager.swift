//
//  BluetoothManager.swift
//  Esp32BLE
//
//  Created by Alon Gerby on 01/09/2025.
//
import Foundation
import CoreBluetooth
import Combine

final class BluetoothManager: NSObject, ObservableObject {
    // === Reactive state (Combine) ===
    @Published var isPoweredOn = false
    @Published var devices: [CBPeripheral] = []
    @Published var connected: CBPeripheral?
    @Published var status: String = "Idle"

    // === Internals ===
    private var central: CBCentralManager!
    private var txChar: CBCharacteristic?

    // === Lifecycle ===
    override init() {
        super.init()
        // Create the CoreBluetooth central manager and set self as its delegate
        central = CBCentralManager(delegate: self, queue: .main)
    }

    // === Public API ===
    func startScan() {
        guard isPoweredOn else { return }
        devices.removeAll()
        status = "Scanning…"
        central.scanForPeripherals(withServices: [BleUUID.service], options: [
            CBCentralManagerScanOptionAllowDuplicatesKey: false
        ])
    }

    func stopScan() {
        central.stopScan()
        status = "Stopped scanning"
    }

    func connect(_ p: CBPeripheral) {
        stopScan()
        p.delegate = self
        central.connect(p, options: nil)
        status = "Connecting to \(p.name ?? "device")…"
    }

    func sendJSON(_ dict: [String: Any]) {
        guard let p = connected, let tx = txChar else { return }
        if let data = try? JSONSerialization.data(withJSONObject: dict) {
            p.writeValue(data, for: tx, type: .withResponse)
            status = "Sent \(dict)"
        }
    }
    func sendMsg(_ text: String, appendNewline: Bool = false) {
        guard let p = connected, let tx = txChar else {
            status = "Not connected"
            return
        }
        var payload = text
        if appendNewline { payload += "\n" }           // handy if ESP32 reads line-based frames

        guard let data = payload.data(using: .utf8) else {
            status = "UTF-8 encode failed"
            return
        }

        // pick write type based on the characteristic’s capabilities
        let writeType: CBCharacteristicWriteType =
            tx.properties.contains(.writeWithoutResponse) ? .withoutResponse : .withResponse

        // ensure the characteristic is actually writable
        guard tx.properties.contains(.write) || tx.properties.contains(.writeWithoutResponse) else {
            status = "TX characteristic not writable"
            return
        }

        p.writeValue(data, for: tx, type: writeType)
        status = "Sent text (\(min(text.count, 32)) chars)"
    }
}

// MARK: - CBCentralManagerDelegate
extension BluetoothManager: CBCentralManagerDelegate {
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        isPoweredOn = (central.state == .poweredOn)
        status = isPoweredOn ? "Bluetooth On" : "Bluetooth Off/\(central.state.rawValue)"
    }

    func centralManager(_ central: CBCentralManager,
                        didDiscover peripheral: CBPeripheral,
                        advertisementData: [String : Any],
                        rssi RSSI: NSNumber) {
        // Keep unique list of peripherals
        if !devices.contains(where: { $0.identifier == peripheral.identifier }) {
            devices.append(peripheral)
        }
    }

    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        connected = peripheral
        status = "Discovering services…"
        peripheral.discoverServices([BleUUID.service])
    }

    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: Error?) {
        status = "Failed to connect: \(error?.localizedDescription ?? "unknown")"
    }

    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        status = "Disconnected"
        connected = nil
        txChar = nil
    }
}

// MARK: - CBPeripheralDelegate
extension BluetoothManager: CBPeripheralDelegate {
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        guard error == nil else { status = "Svc error \(error!)"; return }
        for svc in peripheral.services ?? [] {
            peripheral.discoverCharacteristics([BleUUID.txCharacteristic, BleUUID.rxCharacteristic], for: svc)
        }
    }

    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        guard error == nil else { status = "Char error \(error!)"; return }
        for ch in service.characteristics ?? [] {
            if ch.uuid == BleUUID.txCharacteristic { txChar = ch }
            if ch.uuid == BleUUID.rxCharacteristic { peripheral.setNotifyValue(true, for: ch) }
        }
        status = "Ready"
    }

    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        if let data = characteristic.value, let text = String(data: data, encoding: .utf8) {
            status = "RX: \(text)"
        }
    }
}
