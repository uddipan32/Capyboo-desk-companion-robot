import 'dart:async';
import 'dart:convert';
import 'package:flutter/foundation.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

/// BLE Service UUIDs matching the ESP32 firmware
class BleUuids {
  // Full UUIDs
  static const String serviceUuid = "0000fff0-0000-1000-8000-00805f9b34fb";
  static const String txCharUuid =
      "0000fff1-0000-1000-8000-00805f9b34fb"; // ESP32 TX (we read)
  static const String rxCharUuid =
      "0000fff2-0000-1000-8000-00805f9b34fb"; // ESP32 RX (we write)

  // Short UUIDs (16-bit) for matching
  static const String serviceShort = "fff0";
  static const String txCharShort = "fff1";
  static const String rxCharShort = "fff2";
}

/// Connection state enum
enum BleConnectionState { disconnected, scanning, connecting, connected, error }

/// BLE Service for communicating with Capyboo ESP32
class BleService extends ChangeNotifier {
  static const String deviceName = "Capyboo";

  BluetoothDevice? _connectedDevice;
  BluetoothCharacteristic? _rxCharacteristic;
  BluetoothCharacteristic? _txCharacteristic;
  StreamSubscription<List<int>>? _notificationSubscription;
  StreamSubscription<BluetoothConnectionState>? _connectionSubscription;
  StreamSubscription<List<ScanResult>>? _scanSubscription;

  BleConnectionState _connectionState = BleConnectionState.disconnected;
  String _statusMessage = "Not connected";
  String _lastResponse = "";
  final List<BluetoothDevice> _scannedDevices = [];

  // Getters
  BleConnectionState get connectionState => _connectionState;
  String get statusMessage => _statusMessage;
  String get lastResponse => _lastResponse;
  List<BluetoothDevice> get scannedDevices => _scannedDevices;
  bool get isConnected => _connectionState == BleConnectionState.connected;
  BluetoothDevice? get connectedDevice => _connectedDevice;

  /// Initialize BLE and check adapter state
  Future<bool> initialize() async {
    try {
      // Check if Bluetooth is supported
      if (await FlutterBluePlus.isSupported == false) {
        _updateState(BleConnectionState.error, "Bluetooth not supported");
        return false;
      }

      // Check adapter state
      final adapterState = await FlutterBluePlus.adapterState.first;
      if (adapterState != BluetoothAdapterState.on) {
        _updateState(BleConnectionState.error, "Please turn on Bluetooth");
        return false;
      }

      _updateState(BleConnectionState.disconnected, "Ready to scan");
      return true;
    } catch (e) {
      _updateState(BleConnectionState.error, "Init error: $e");
      return false;
    }
  }

  /// Scan for Capyboo devices (scan by name, not service UUID)
  Future<void> startScan({
    Duration timeout = const Duration(seconds: 10),
  }) async {
    try {
      _scannedDevices.clear();
      _updateState(BleConnectionState.scanning, "Scanning for Capyboo...");

      // Cancel any existing scan subscription
      await _scanSubscription?.cancel();

      // Listen for scan results
      _scanSubscription = FlutterBluePlus.scanResults.listen((results) {
        for (ScanResult result in results) {
          if (!_scannedDevices.any(
            (d) => d.remoteId == result.device.remoteId,
          )) {
            final name = result.device.platformName.isNotEmpty
                ? result.device.platformName
                : result.advertisementData.advName;

            debugPrint("Found device: $name (${result.device.remoteId})");

            // Check if it's a Capyboo device (case-insensitive)
            if (name.toLowerCase().contains(deviceName.toLowerCase())) {
              debugPrint(">>> Found Capyboo! Adding to list");
              _scannedDevices.add(result.device);
              notifyListeners();
            }
          }
        }
      });

      // Start scanning WITHOUT service UUID filter
      // This allows finding devices that don't advertise their service UUIDs
      await FlutterBluePlus.startScan(
        timeout: timeout,
        androidUsesFineLocation: true,
      );

      // Wait for scan to complete
      await Future.delayed(timeout);

      // Clean up scan subscription
      await _scanSubscription?.cancel();
      _scanSubscription = null;

      if (_scannedDevices.isEmpty) {
        _updateState(
          BleConnectionState.disconnected,
          "No Capyboo found. Make sure it's powered on.",
        );
      } else {
        _updateState(
          BleConnectionState.disconnected,
          "Found ${_scannedDevices.length} device(s)",
        );
      }
    } catch (e) {
      _updateState(BleConnectionState.error, "Scan error: $e");
    }
  }

  /// Stop scanning
  Future<void> stopScan() async {
    await FlutterBluePlus.stopScan();
    await _scanSubscription?.cancel();
    _scanSubscription = null;
    if (_connectionState == BleConnectionState.scanning) {
      _updateState(BleConnectionState.disconnected, "Scan stopped");
    }
  }

  /// Check if UUID matches (handles both full and short UUID formats)
  bool _uuidMatches(String uuid, String targetShort) {
    final uuidLower = uuid.toLowerCase();
    final targetLower = targetShort.toLowerCase();
    return uuidLower.contains(targetLower);
  }

  /// Connect to a Capyboo device
  Future<bool> connect(BluetoothDevice device) async {
    try {
      final deviceDisplayName = device.platformName.isNotEmpty
          ? device.platformName
          : device.remoteId.toString();

      _updateState(
        BleConnectionState.connecting,
        "Connecting to $deviceDisplayName...",
      );

      // Connect to device
      await device.connect(timeout: const Duration(seconds: 15));
      _connectedDevice = device;

      // Listen for disconnection
      _connectionSubscription = device.connectionState.listen((state) {
        if (state == BluetoothConnectionState.disconnected) {
          _handleDisconnection();
        }
      });

      // Discover services
      _updateState(BleConnectionState.connecting, "Discovering services...");
      List<BluetoothService> services = await device.discoverServices();

      // Debug: Print all discovered services
      debugPrint("=== Discovered ${services.length} services ===");
      for (var service in services) {
        debugPrint("Service: ${service.uuid}");
        for (var char in service.characteristics) {
          debugPrint("  - Characteristic: ${char.uuid}");
          debugPrint(
            "    Properties: read=${char.properties.read}, write=${char.properties.write}, notify=${char.properties.notify}",
          );
        }
      }

      // Find our service (match by short UUID "fff0")
      BluetoothService? targetService;
      for (var service in services) {
        if (_uuidMatches(service.uuid.toString(), BleUuids.serviceShort)) {
          targetService = service;
          debugPrint(">>> Found target service: ${service.uuid}");
          break;
        }
      }

      if (targetService == null) {
        await disconnect();
        _updateState(
          BleConnectionState.error,
          "Capyboo service not found. Check ESP32 is running.",
        );
        return false;
      }

      // Find characteristics (match by short UUID)
      for (var characteristic in targetService.characteristics) {
        String uuid = characteristic.uuid.toString();
        debugPrint("Checking characteristic: $uuid");

        if (_uuidMatches(uuid, BleUuids.rxCharShort)) {
          _rxCharacteristic = characteristic;
          debugPrint(">>> Found RX characteristic: $uuid");
        } else if (_uuidMatches(uuid, BleUuids.txCharShort)) {
          _txCharacteristic = characteristic;
          debugPrint(">>> Found TX characteristic: $uuid");
        }
      }

      if (_rxCharacteristic == null) {
        await disconnect();
        _updateState(BleConnectionState.error, "RX characteristic not found");
        return false;
      }

      // Subscribe to notifications from TX characteristic (ESP32 responses)
      if (_txCharacteristic != null && _txCharacteristic!.properties.notify) {
        await _txCharacteristic!.setNotifyValue(true);
        _notificationSubscription = _txCharacteristic!.lastValueStream.listen((
          value,
        ) {
          if (value.isNotEmpty) {
            _lastResponse = utf8.decode(value);
            debugPrint("Received from ESP32: $_lastResponse");
            notifyListeners();
          }
        });
      }

      _updateState(
        BleConnectionState.connected,
        "Connected to $deviceDisplayName",
      );
      return true;
    } catch (e) {
      debugPrint("Connection error: $e");
      await disconnect();
      _updateState(BleConnectionState.error, "Connection failed: $e");
      return false;
    }
  }

  /// Disconnect from current device
  Future<void> disconnect() async {
    try {
      await _notificationSubscription?.cancel();
      _notificationSubscription = null;

      await _connectionSubscription?.cancel();
      _connectionSubscription = null;

      if (_connectedDevice != null) {
        await _connectedDevice!.disconnect();
      }
    } catch (e) {
      debugPrint("Disconnect error: $e");
    } finally {
      _connectedDevice = null;
      _rxCharacteristic = null;
      _txCharacteristic = null;
      _updateState(BleConnectionState.disconnected, "Disconnected");
    }
  }

  /// Handle unexpected disconnection
  void _handleDisconnection() {
    _connectedDevice = null;
    _rxCharacteristic = null;
    _txCharacteristic = null;
    _notificationSubscription?.cancel();
    _notificationSubscription = null;
    _updateState(BleConnectionState.disconnected, "Device disconnected");
  }

  /// Send WiFi credentials to ESP32
  Future<bool> sendWifiCredentials(String ssid, String password) async {
    if (!isConnected || _rxCharacteristic == null) {
      _updateState(_connectionState, "Not connected to device");
      return false;
    }

    try {
      // Format: wifi:SSID:password
      String command = "wifi:$ssid:$password";
      _updateState(BleConnectionState.connected, "Sending WiFi credentials...");

      debugPrint("Sending command: wifi:$ssid:****");

      // Send command
      await _rxCharacteristic!.write(
        utf8.encode(command),
        withoutResponse: _rxCharacteristic!.properties.writeWithoutResponse,
      );

      _updateState(BleConnectionState.connected, "WiFi credentials sent!");
      return true;
    } catch (e) {
      debugPrint("Send error: $e");
      _updateState(BleConnectionState.connected, "Send failed: $e");
      return false;
    }
  }

  /// Send a generic command to ESP32
  Future<bool> sendCommand(String command) async {
    if (!isConnected || _rxCharacteristic == null) {
      return false;
    }

    try {
      debugPrint("Sending command: $command");
      await _rxCharacteristic!.write(
        utf8.encode(command),
        withoutResponse: _rxCharacteristic!.properties.writeWithoutResponse,
      );
      return true;
    } catch (e) {
      debugPrint("Send command error: $e");
      return false;
    }
  }

  /// Clear WiFi credentials on ESP32
  Future<bool> clearWifiCredentials() async {
    return await sendCommand("clearwifi");
  }

  /// Update state and notify listeners
  void _updateState(BleConnectionState state, String message) {
    _connectionState = state;
    _statusMessage = message;
    debugPrint("BLE State: $state - $message");
    notifyListeners();
  }

  @override
  void dispose() {
    disconnect();
    _scanSubscription?.cancel();
    super.dispose();
  }
}
