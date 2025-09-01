//
//  ContentView.swift
//  Esp32BLE
//
//  Created by Alon Gerby on 01/09/2025.
//

import SwiftUI

struct ContentView: View {
    @StateObject private var ble = BluetoothManager()

    var body: some View {
        VStack(spacing: 16) {
            Text(ble.status).font(.footnote).foregroundStyle(.secondary)

            HStack {
                Button("Scan") { ble.startScan() }.buttonStyle(.borderedProminent)
                Button("Stop") { ble.stopScan() }.buttonStyle(.bordered)
            }

            List(ble.devices, id: \.identifier) { p in
                Button(action: { ble.connect(p) }) {
                    HStack {
                        Text(p.name ?? "Unnamed")
                        Spacer()
                        Text(p.identifier.uuidString.prefix(8) + "…").foregroundStyle(.secondary)
                    }
                }
            }

            HStack {
                Button("ON") { ble.sendMsg("ON") }
                Button("OFF") { ble.sendMsg("OFF") }
            }
            .buttonStyle(.borderedProminent)
        }
        .padding()
    }
}

