import React, { useState, useEffect } from 'react';
import {
  StyleSheet,
  TextInput,
  TouchableOpacity,
  Alert,
  ActivityIndicator,
  ScrollView,
  Platform,
} from 'react-native';
import { ThemedText } from '@/components/themed-text';
import { ThemedView } from '@/components/themed-view';
import { useRouter } from 'expo-router';
import bleService from '@/services/bleService';
import { StatusBar } from 'expo-status-bar';

export default function WiFiSetupScreen() {
  const router = useRouter();
  const [ssid, setSSID] = useState('');
  const [password, setPassword] = useState('');
  const [isScanning, setIsScanning] = useState(false);
  const [isConnecting, setIsConnecting] = useState(false);
  const [isSending, setIsSending] = useState(false);
  const [isConnected, setIsConnected] = useState(false);
  const [deviceName, setDeviceName] = useState('Capyboo');

  useEffect(() => {
    // Initialize BLE on mount
    initializeBLE();
    
    return () => {
      // Cleanup on unmount
      if (bleService.getConnected()) {
        bleService.disconnect();
      }
    };
  }, []);

  const initializeBLE = async () => {
    try {
      await bleService.initialize();
    } catch (error: any) {
      Alert.alert('Bluetooth Error', error.message);
    }
  };

  const scanForDevice = async () => {
    setIsScanning(true);
    try {
      const device = await bleService.scanForDevice(deviceName);
      if (device) {
        Alert.alert('Device Found', `Found ${device.name}. Connecting...`);
        await connectToDevice(device);
      } else {
        Alert.alert('Device Not Found', `Could not find ${deviceName}. Make sure it's powered on and nearby.`);
      }
    } catch (error: any) {
      Alert.alert('Scan Error', error.message);
    } finally {
      setIsScanning(false);
    }
  };

  const connectToDevice = async (device: any) => {
    setIsConnecting(true);
    try {
      await bleService.connect(device);
      setIsConnected(true);
      Alert.alert('Connected', 'Successfully connected to Capyboo!');
    } catch (error: any) {
      Alert.alert('Connection Error', error.message);
      setIsConnected(false);
    } finally {
      setIsConnecting(false);
    }
  };

  const sendWiFiCredentials = async () => {
    if (!ssid.trim()) {
      Alert.alert('Error', 'Please enter WiFi SSID');
      return;
    }

    if (ssid.length > 32) {
      Alert.alert('Error', 'SSID must be 32 characters or less');
      return;
    }

    if (password.length > 63) {
      Alert.alert('Error', 'Password must be 63 characters or less');
      return;
    }

    if (!isConnected) {
      Alert.alert('Not Connected', 'Please connect to Capyboo first');
      return;
    }

    setIsSending(true);
    try {
      // Format: wifi:SSID:password
      const command = `wifi:${ssid}:${password}`;
      await bleService.sendData(command);
      
      Alert.alert(
        'Success',
        'WiFi credentials sent to Capyboo!\n\nThe robot will now attempt to connect to the WiFi network.',
        [
          {
            text: 'OK',
            onPress: () => {
              setSSID('');
              setPassword('');
              router.back();
            },
          },
        ]
      );
    } catch (error: any) {
      Alert.alert('Send Error', error.message);
    } finally {
      setIsSending(false);
    }
  };

  const disconnect = async () => {
    try {
      await bleService.disconnect();
      setIsConnected(false);
      Alert.alert('Disconnected', 'Disconnected from Capyboo');
    } catch (error: any) {
      Alert.alert('Error', error.message);
    }
  };

  return (
    <ThemedView style={styles.container}>
      <StatusBar style="auto" />
      <ScrollView contentContainerStyle={styles.scrollContent}>
        <ThemedView style={styles.header}>
          <ThemedText type="title">WiFi Setup</ThemedText>
          <ThemedText style={styles.subtitle}>
            Connect Capyboo to your WiFi network
          </ThemedText>
        </ThemedView>

        <ThemedView style={styles.section}>
          <ThemedText type="subtitle">Device Connection</ThemedText>
          
          <TextInput
            style={styles.input}
            placeholder="Device name (default: Capyboo)"
            placeholderTextColor="#999"
            value={deviceName}
            onChangeText={setDeviceName}
            editable={!isScanning && !isConnecting}
          />

          {!isConnected ? (
            <TouchableOpacity
              style={[styles.button, styles.primaryButton]}
              onPress={scanForDevice}
              disabled={isScanning || isConnecting}
            >
              {isScanning || isConnecting ? (
                <ActivityIndicator color="#fff" />
              ) : (
                <ThemedText style={styles.buttonText}>Scan & Connect</ThemedText>
              )}
            </TouchableOpacity>
          ) : (
            <ThemedView style={styles.connectionStatus}>
              <ThemedText style={styles.connectedText}>✓ Connected to {deviceName}</ThemedText>
              <TouchableOpacity
                style={[styles.button, styles.secondaryButton]}
                onPress={disconnect}
              >
                <ThemedText style={styles.buttonText}>Disconnect</ThemedText>
              </TouchableOpacity>
            </ThemedView>
          )}
        </ThemedView>

        <ThemedView style={styles.section}>
          <ThemedText type="subtitle">WiFi Credentials</ThemedText>
          
          <ThemedText style={styles.label}>WiFi Network Name (SSID)</ThemedText>
          <TextInput
            style={styles.input}
            placeholder="Enter WiFi SSID"
            placeholderTextColor="#999"
            value={ssid}
            onChangeText={setSSID}
            autoCapitalize="none"
            autoCorrect={false}
            editable={isConnected && !isSending}
          />

          <ThemedText style={styles.label}>Password</ThemedText>
          <TextInput
            style={styles.input}
            placeholder="Enter WiFi password"
            placeholderTextColor="#999"
            value={password}
            onChangeText={setPassword}
            secureTextEntry
            autoCapitalize="none"
            autoCorrect={false}
            editable={isConnected && !isSending}
          />

          <TouchableOpacity
            style={[
              styles.button,
              styles.primaryButton,
              (!isConnected || !ssid.trim() || isSending) && styles.buttonDisabled,
            ]}
            onPress={sendWiFiCredentials}
            disabled={!isConnected || !ssid.trim() || isSending}
          >
            {isSending ? (
              <ActivityIndicator color="#fff" />
            ) : (
              <ThemedText style={styles.buttonText}>Send to Capyboo</ThemedText>
            )}
          </TouchableOpacity>
        </ThemedView>

        <ThemedView style={styles.infoSection}>
          <ThemedText style={styles.infoText}>
            • Make sure Capyboo is powered on and nearby{'\n'}
            • SSID: 1-32 characters{'\n'}
            • Password: 0-63 characters{'\n'}
            • The robot will save credentials and connect automatically
          </ThemedText>
        </ThemedView>
      </ScrollView>
    </ThemedView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
  },
  scrollContent: {
    padding: 20,
    paddingBottom: 40,
  },
  header: {
    marginBottom: 30,
    alignItems: 'center',
  },
  subtitle: {
    marginTop: 8,
    fontSize: 16,
    opacity: 0.7,
    textAlign: 'center',
  },
  section: {
    marginBottom: 30,
  },
  label: {
    fontSize: 14,
    marginBottom: 8,
    marginTop: 16,
    fontWeight: '600',
  },
  input: {
    backgroundColor: '#f5f5f5',
    borderRadius: 8,
    padding: 12,
    fontSize: 16,
    marginBottom: 16,
    borderWidth: 1,
    borderColor: '#e0e0e0',
    color: '#000',
  },
  button: {
    borderRadius: 8,
    padding: 16,
    alignItems: 'center',
    justifyContent: 'center',
    marginTop: 8,
    minHeight: 50,
  },
  primaryButton: {
    backgroundColor: '#007AFF',
  },
  secondaryButton: {
    backgroundColor: '#8E8E93',
    marginTop: 12,
  },
  buttonDisabled: {
    backgroundColor: '#ccc',
    opacity: 0.6,
  },
  buttonText: {
    color: '#fff',
    fontSize: 16,
    fontWeight: '600',
  },
  connectionStatus: {
    marginTop: 12,
  },
  connectedText: {
    color: '#34C759',
    fontSize: 16,
    fontWeight: '600',
    marginBottom: 12,
  },
  infoSection: {
    marginTop: 20,
    padding: 16,
    backgroundColor: '#f9f9f9',
    borderRadius: 8,
  },
  infoText: {
    fontSize: 14,
    lineHeight: 20,
    opacity: 0.7,
  },
});

