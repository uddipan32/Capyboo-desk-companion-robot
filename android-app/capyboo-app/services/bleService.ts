import { BleManager, Device, Characteristic } from 'react-native-ble-plx';
import { Buffer } from 'buffer';

// BLE Service UUIDs (matching ESP32 firmware)
const BLE_SERVICE_UUID = '0000fff0-0000-1000-8000-00805f9b34fb';
const BLE_CHAR_UUID_RX = '0000fff2-0000-1000-8000-00805f9b34fb'; // For sending data
const BLE_CHAR_UUID_TX = '0000fff1-0000-1000-8000-00805f9b34fb'; // For receiving data

class BLEService {
  private manager: BleManager;
  private device: Device | null = null;
  private isConnected: boolean = false;

  constructor() {
    this.manager = new BleManager();
  }

  // Initialize BLE manager
  async initialize(): Promise<void> {
    return new Promise((resolve, reject) => {
      const subscription = this.manager.onStateChange((state) => {
        if (state === 'PoweredOn') {
          subscription.remove();
          resolve();
        } else if (state === 'PoweredOff') {
          subscription.remove();
          reject(new Error('Bluetooth is turned off. Please enable Bluetooth.'));
        }
      }, true);
    });
  }

  // Scan for Capyboo device
  async scanForDevice(deviceName: string = 'Capyboo'): Promise<Device | null> {
    return new Promise((resolve, reject) => {
      const devices: Device[] = [];
      
      const subscription = this.manager.onStateChange((state) => {
        if (state === 'PoweredOn') {
          this.manager.startDeviceScan(null, null, (error, device) => {
            if (error) {
              subscription.remove();
              this.manager.stopDeviceScan();
              reject(error);
              return;
            }

            if (device && device.name === deviceName) {
              devices.push(device);
              this.manager.stopDeviceScan();
              subscription.remove();
              resolve(device);
            }
          });
        }
      }, true);
    });
  }

  // Connect to device
  async connect(device: Device): Promise<void> {
    try {
      this.device = await device.connect();
      await this.device.discoverAllServicesAndCharacteristics();
      this.isConnected = true;
    } catch (error) {
      this.isConnected = false;
      throw error;
    }
  }

  // Send data to device
  async sendData(data: string): Promise<void> {
    if (!this.device || !this.isConnected) {
      throw new Error('Not connected to device');
    }

    try {
      // Convert string to base64 (required by react-native-ble-plx)
      const base64Data = Buffer.from(data, 'utf8').toString('base64');
      
      await this.device.writeCharacteristicWithResponseForService(
        BLE_SERVICE_UUID,
        BLE_CHAR_UUID_RX,
        base64Data
      );
    } catch (error: any) {
      throw new Error(`Failed to send data: ${error.message || error}`);
    }
  }

  // Disconnect from device
  async disconnect(): Promise<void> {
    if (this.device) {
      await this.device.cancelConnection();
      this.device = null;
      this.isConnected = false;
    }
  }

  // Check if connected
  getConnected(): boolean {
    return this.isConnected;
  }

  // Get connected device
  getDevice(): Device | null {
    return this.device;
  }

  // Cleanup
  destroy(): void {
    if (this.device) {
      this.disconnect();
    }
    this.manager.destroy();
  }
}

export default new BLEService();

