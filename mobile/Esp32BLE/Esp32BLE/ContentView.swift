import SwiftUI
import CoreBluetooth

struct ContentView: View {
    @StateObject private var ble = BluetoothManager()
    @State private var customMsg = ""

    var body: some View {
        NavigationStack {
            VStack(spacing: 12) {
                StatusBar(text: ble.status, connected: ble.connected != nil)

                ControlsBar(
                    isConnected: ble.connected != nil,
                    onScan: ble.startScan,
                    onStop: ble.stopScan,
                    onDisconnect: { ble.disconnect() }
                )

                ScrollView {
                    VStack(alignment: .leading, spacing: 16) {
                        DevicesSectionContainer(
                            devices: ble.devices,
                            connectedId: ble.connected?.identifier,
                            onConnect: ble.connect
                        )

                        CommandsSectionContainer(
                            customMsg: $customMsg,
                            onOn: { ble.sendMsg("ON") },
                            onOff: { ble.sendMsg("{type:'AC', cmd:'OFF'}") },
                            onSend: { msg in ble.sendMsg(msg, appendNewline: true) }
                        )
                    }
                    .padding(.vertical, 8)
                }
            }
            .padding(.horizontal, 8)
            .navigationTitle("ESP32 Remote")
        }
    }
}

// MARK: - Sections / Components

private enum OpenPanel {
    case none, ac, tv
}

// MARK: - TV types
private enum TVBrand: String, CaseIterable, Identifiable {
    case lg, samsung
    var id: Self { self }
    var label: String { rawValue.uppercased() }
    var jsonValue: String { rawValue.uppercased() }
}
private enum TVInput: String, CaseIterable, Identifiable {
    case tv = "TV"
    case hdmi1 = "HDMI1"
    case hdmi2 = "HDMI2"
    case av = "AV"
    var id: Self { self }
    var label: String { rawValue }
}

// MARK: - AC types
private enum ACMode: Int, CaseIterable, Identifiable {
    case cool = 1
    case hot = 4
    var id: Self { self }
    var label: String {
        switch self {
        case .cool: "Cool"
        case .hot:  "Hot"
        }
    }
    var intValue: Int { rawValue }
}

private enum ACFan: Int, CaseIterable, Identifiable {
    case auto = 0
    case min = 1
    case med = 2
    case max = 3
    var id: Self { self }
    var label: String {
        switch self {
        case .auto: "Auto"
        case .min:  "Min"
        case .med:  "Mid"
        case .max:  "Max"
        }
    }
    var intValue: Int { rawValue }
}

// MARK: - Combined AC + TV section
private struct CommandsSection: View {
    @Binding var customMsg: String
    let onOn: () -> Void
    let onOff: () -> Void
    let onSend: (String) -> Void
    
    @State private var open: OpenPanel = .none

    // AC state
    @State private var temp: Int = 24
    @State private var mode: ACMode = .cool
    @State private var fan: ACFan = .auto

    // TV state
    @State private var tvBrand: TVBrand = .lg
    @State private var channelText: String = ""

    var body: some View {
        Section("Commands") {
            // Top row: quick power + expanders
            HStack(spacing: 8) {
                Button {
                    withAnimation { open = (open == .ac ? .none : .ac) }
                } label: {
                    Label("AC", systemImage: open == .ac ? "chevron.down.circle.fill" : "chevron.right.circle")
                        .labelStyle(.titleAndIcon)
                }
                .buttonStyle(Primary())

                Button {
                    withAnimation { open = (open == .tv ? .none : .tv) }
                } label: {
                    Label("TV", systemImage: open == .tv ? "chevron.down.circle.fill" : "chevron.right.circle")
                        .labelStyle(.titleAndIcon)
                }
                .buttonStyle(Primary())
            }

            // ---- AC PANEL ----
            if open == .ac {
                VStack(alignment: .leading, spacing: 12) {
                    Text("Air Conditioner").font(.headline)

                    // Temp
                    HStack {
                        Label("Temp", systemImage: "thermometer")
                        Spacer()
                        Text("\(temp)°C")
                            .font(.system(.body, design: .monospaced))
                    }
                    Slider(
                        value: Binding(get: { Double(temp) }, set: { temp = Int($0) }),
                        in: 16...30, step: 1
                    )

                    // Mode
                    VStack(alignment: .leading, spacing: 6) {
                        Label("Mode", systemImage: "snowflake")
                        Picker("Mode", selection: $mode) {
                            ForEach(ACMode.allCases) { m in Text(m.label).tag(m) }
                        }
                        .pickerStyle(.segmented)
                    }

                    // Fan
                    VStack(alignment: .leading, spacing: 6) {
                        Label("Fan", systemImage: "wind")
                        Picker("Fan", selection: $fan) {
                            ForEach(ACFan.allCases) { f in Text(f.label).tag(f) }
                        }
                        .pickerStyle(.segmented)
                    }

                    HStack {
                        Button {
                            onOff()
                        } label: {
                            Label {
                                Text("Turn Off")
                            } icon: {
                                Image(systemName: "power.circle.fill")
                                    .foregroundStyle(.red)
                            }
                        }
                        .buttonStyle(Primary())
                        Spacer()
                        Button {
                            sendAC()
                        } label: {
                            Label("Send", systemImage: "paperplane.fill")
                        }
                        .buttonStyle(Primary())
                    }
                }
                .padding(10)
                .background(.thinMaterial, in: RoundedRectangle(cornerRadius: 12))
                .transition(.opacity.combined(with: .move(edge: .top)))
            }

            // ---- TV PANEL ----
            if open == .tv {
                VStack(alignment: .leading, spacing: 12) {
                    Text("Television").font(.headline)

                    // Brand
                    VStack(alignment: .leading, spacing: 6) {
                        Label("Brand", systemImage: "display")
                        Picker("Brand", selection: $tvBrand) {
                            ForEach(TVBrand.allCases) { b in Text(b.label).tag(b) }
                        }
                        .pickerStyle(.segmented)
                    }

                    // Quick actions (consistent single-line buttons)
                    VStack(spacing: 8) {
                        HStack(spacing: 8) {
                            ActionButton(title: "Power", systemImage: "power.circle.fill") { sendTV("POWER_TOGGLE") }
                            ActionButton(title: "Mute",  systemImage: "speaker.slash.circle.fill") { sendTV("MUTE") }
                            ActionButton(title: "Back",  systemImage: "arrow.uturn.left.circle.fill") { sendTV("BACK") }
                        }
                        HStack(spacing: 8) {
                            ActionButton(title: "Input", systemImage: "rectangle.connected.to.line.below") { sendTV("INPUT") }
                            ActionButton(title: "HDMI 1", systemImage: "h.square.fill.on.square.fill") { sendTV("HDMI_1") }
                            ActionButton(title: "HDMI 2", systemImage: "h.square.fill.on.square.fill") { sendTV("HDMI_2") }
                        }
                        HStack(spacing: 8) {
                            ActionButton(title: "Vol +", systemImage: "speaker.wave.3.fill") { sendTV("VOL_UP") }
                            ActionButton(title: "Vol −", systemImage: "speaker.wave.1.fill") { sendTV("VOL_DOWN") }
                            ActionButton(title: "Chan +", systemImage: "arrow.up.circle.fill") { sendTV("CH_UP") }
                        }
                        HStack(spacing: 8) {
                            ActionButton(title: "Chan −", systemImage: "arrow.down.circle.fill") { sendTV("CH_DOWN") }
                            Spacer(minLength: 0)
                        }
                    }
                    
                    // D-Pad around OK
                    RemoteDPad(
                        onUp:    { sendTV("NAV_UP") },
                        onDown:  { sendTV("NAV_DOWN") },
                        onLeft:  { sendTV("NAV_LEFT") },
                        onRight: { sendTV("NAV_RIGHT") },
                        onOK:    { sendTV("OK") }
                    )
                    
                    // Channel set
//                    HStack(spacing: 8) {
//                        Label("Channel", systemImage: "number")
//                        TextField("e.g. 12", text: $channelText)
//                            .keyboardType(.numberPad)
//                            .textFieldStyle(.roundedBorder)
//                            .frame(width: 90)
//                        Spacer()
//                        Button("Go") {
//                            if let ch = Int(channelText) { sendTV("CH_SET", extra: ["channel": ch]) }
//                        }
//                        .buttonStyle(Primary())
//                    }
                }
                .padding(10)
                .background(.thinMaterial, in: RoundedRectangle(cornerRadius: 12))
                .transition(.opacity.combined(with: .move(edge: .top)))
            }
        }
    }

    // ---- Send helpers ----
    private func sendAC() {
        let dict: [String: Any] = [
            "type": "AC",
            "brand": "TADIRAN",
            "cmd": "ON",
            "temp_c": temp,
            "mode": mode.intValue,
            "fan": fan.intValue
        ]
        if let data = try? JSONSerialization.data(withJSONObject: dict),
           let json = String(data: data, encoding: .utf8) {
            onSend(json + "\n")
        }
    }

    private func sendTV(_ cmd: String, extra: [String: Any] = [:]) {
        var dict: [String: Any] = [
            "type": "TV",
            "brand": tvBrand.jsonValue,
            "cmd": cmd
        ]
        extra.forEach { dict[$0.key] = $0.value }

        if let data = try? JSONSerialization.data(withJSONObject: dict),
           let json = String(data: data, encoding: .utf8) {
            onSend(json + "\n")
        }
    }
}

private struct ControlsBar: View {
    let isConnected: Bool
    let onScan: () -> Void
    let onStop: () -> Void
    let onDisconnect: () -> Void

    var body: some View {
        HStack(spacing: 10) {
            Button { onScan() } label: {
                Label("Scan", systemImage: "dot.radiowaves.left.and.right")
            }.buttonStyle(Primary())
            Button("Stop", action: onStop).buttonStyle(Secondary())
            if isConnected {
                Button { onDisconnect() } label: {
                    Label("Disconnect", systemImage: "xmark.circle")
                }.buttonStyle(Destructive())
            }
        }
    }
}

private struct DevicesSection: View {
    let devices: [CBPeripheral]
    let connectedId: UUID?
    let onConnect: (CBPeripheral) -> Void

    var body: some View {
        Section("Devices") {
            ForEach(devices, id: \.identifier) { p in
                Button { onConnect(p) } label: {
                    HStack {
                        VStack(alignment: .leading, spacing: 2) {
                            Text(p.name ?? "Unnamed").fontWeight(.medium)
                            Text("\(p.identifier.uuidString.prefix(8))…")
                                .font(.caption).foregroundStyle(.secondary)
                        }
                        Spacer()
                        if p.identifier == connectedId {
                            Image(systemName: "link.circle.fill").foregroundStyle(.green)
                        } else {
                            Image(systemName: "link").foregroundStyle(.secondary)
                        }
                    }
                }
            }
        }
    }
}


private struct StatusBar: View {
    let text: String
    let connected: Bool
    var body: some View {
        HStack(spacing: 8) {
            Circle().fill(connected ? .green : .gray).frame(width: 10, height: 10)
            Text(text).font(.footnote).foregroundStyle(.secondary)
            Spacer()
        }
        .padding(10)
        .background(.thinMaterial, in: RoundedRectangle(cornerRadius: 12))
    }
}

private struct Primary: ButtonStyle {
    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .padding(.horizontal, 14).padding(.vertical, 8)
            .background(.blue.opacity(configuration.isPressed ? 0.6 : 0.9),
                        in: RoundedRectangle(cornerRadius: 12))
            .foregroundStyle(.white)
    }
}
private struct Secondary: ButtonStyle {
    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .padding(.horizontal, 14).padding(.vertical, 8)
            .background(.thinMaterial, in: RoundedRectangle(cornerRadius: 12))
    }
}
private struct Destructive: ButtonStyle {
    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .padding(.horizontal, 14).padding(.vertical, 8)
            .background(Color.red.opacity(configuration.isPressed ? 0.6 : 0.9),
                        in: RoundedRectangle(cornerRadius: 12))
            .foregroundStyle(.white)
    }
}

private struct SectionHeader: View {
    let title: String
    var body: some View {
        Text(title)
            .font(.headline)
            .frame(maxWidth: .infinity, alignment: .leading)
            .padding(.horizontal, 4)
            .padding(.top, 4)
    }
}

private struct DevicesSectionContainer: View {
    let devices: [CBPeripheral]
    let connectedId: UUID?
    let onConnect: (CBPeripheral) -> Void

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            SectionHeader(title: "Devices")
            VStack(spacing: 0) {
                ForEach(devices, id: \.identifier) { p in
                    Button { onConnect(p) } label: {
                        HStack {
                            VStack(alignment: .leading, spacing: 2) {
                                Text(p.name ?? "Unnamed").fontWeight(.medium)
                                Text("\(p.identifier.uuidString.prefix(8))…")
                                    .font(.caption).foregroundStyle(.secondary)
                            }
                            Spacer()
                            if p.identifier == connectedId {
                                Image(systemName: "link.circle.fill").foregroundStyle(.green)
                            } else {
                                Image(systemName: "link").foregroundStyle(.secondary)
                            }
                        }
                        .padding(.vertical, 10)
                        .padding(.horizontal, 12)
                    }
                    .background(.ultraThinMaterial)
                    .clipShape(RoundedRectangle(cornerRadius: 10))
                }
            }
        }
    }
}

private struct CommandsSectionContainer: View {
    @Binding var customMsg: String
    let onOn: () -> Void
    let onOff: () -> Void
    let onSend: (String) -> Void

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            SectionHeader(title: "")
            // reuse your combined CommandsSection but without Section{ }
            CommandsSection(customMsg: $customMsg, onOn: onOn, onOff: onOff, onSend: onSend)
                .padding(10)
                .background(.ultraThinMaterial, in: RoundedRectangle(cornerRadius: 12))
        }
    }
}

// Round icon buttons for the D-pad
private struct RoundIconButton: View {
    let systemImage: String
    let action: () -> Void
    var body: some View {
        Button(action: action) {
            Image(systemName: systemImage)
                .font(.title2)
                .frame(width: 44, height: 44)
                .background(.ultraThinMaterial, in: Circle())
        }
        .buttonStyle(Secondary())
    }
}

// 3×3 D-pad with OK in the center
private struct RemoteDPad: View {
    let onUp: () -> Void
    let onDown: () -> Void
    let onLeft: () -> Void
    let onRight: () -> Void
    let onOK: () -> Void

    var body: some View {
        VStack(spacing: 8) {
            HStack { Spacer(); RoundIconButton(systemImage: "chevron.up.circle.fill", action: onUp); Spacer() }
            HStack(spacing: 8) {
                RoundIconButton(systemImage: "chevron.left.circle.fill", action: onLeft)
                Button(action: onOK) {
                    Text("OK").fontWeight(.semibold)
                        .frame(width: 64, height: 64)
                        .background(.ultraThinMaterial, in: Circle())
                }
                .buttonStyle(Secondary())
                RoundIconButton(systemImage: "chevron.right.circle.fill", action: onRight)
            }
            HStack { Spacer(); RoundIconButton(systemImage: "chevron.down.circle.fill", action: onDown); Spacer() }
        }
    }
}

// Single-line action button (icon + text, same row)
private struct ActionButton: View {
    let title: String
    let systemImage: String
    let action: () -> Void
    var body: some View {
        Button(action: action) {
            HStack(spacing: 8) {
                Image(systemName: systemImage)
                Text(title).lineLimit(1).minimumScaleFactor(0.8)
            }
            .frame(maxWidth: .infinity, minHeight: 44)
        }
        .buttonStyle(Secondary())
    }
}
