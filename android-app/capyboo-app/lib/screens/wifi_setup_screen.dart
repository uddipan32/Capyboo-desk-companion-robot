import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../services/ble_service.dart';

class WifiSetupScreen extends StatefulWidget {
  const WifiSetupScreen({super.key});

  @override
  State<WifiSetupScreen> createState() => _WifiSetupScreenState();
}

class _WifiSetupScreenState extends State<WifiSetupScreen> {
  final _formKey = GlobalKey<FormState>();
  final _ssidController = TextEditingController();
  final _passwordController = TextEditingController();
  bool _obscurePassword = true;
  bool _isSending = false;

  @override
  void dispose() {
    _ssidController.dispose();
    _passwordController.dispose();
    super.dispose();
  }

  Future<void> _sendWifiCredentials() async {
    if (!_formKey.currentState!.validate()) return;

    setState(() => _isSending = true);

    final bleService = context.read<BleService>();
    final success = await bleService.sendWifiCredentials(
      _ssidController.text.trim(),
      _passwordController.text,
    );

    setState(() => _isSending = false);

    if (mounted) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text(
            success
                ? '✓ WiFi credentials sent! Check Capyboo display.'
                : '✗ Failed to send credentials',
          ),
          backgroundColor: success ? const Color(0xFF4CAF50) : const Color(0xFFE57373),
          behavior: SnackBarBehavior.floating,
          shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
        ),
      );
    }
  }

  Future<void> _clearWifiCredentials() async {
    final bleService = context.read<BleService>();
    final success = await bleService.clearWifiCredentials();

    if (mounted) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text(
            success ? '✓ WiFi credentials cleared!' : '✗ Failed to clear',
          ),
          backgroundColor: success ? const Color(0xFF4CAF50) : const Color(0xFFE57373),
          behavior: SnackBarBehavior.floating,
          shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
        ),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFF1A1A2E),
      appBar: AppBar(
        title: const Text(
          'WiFi Setup',
          style: TextStyle(
            fontFamily: 'Georgia',
            fontWeight: FontWeight.w600,
            letterSpacing: 0.5,
          ),
        ),
        backgroundColor: Colors.transparent,
        elevation: 0,
        foregroundColor: const Color(0xFFF5E6D3),
      ),
      body: Consumer<BleService>(
        builder: (context, bleService, _) {
          if (!bleService.isConnected) {
            return _buildNotConnectedView();
          }
          return _buildWifiForm(bleService);
        },
      ),
    );
  }

  Widget _buildNotConnectedView() {
    return Center(
      child: Padding(
        padding: const EdgeInsets.all(32),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Container(
              padding: const EdgeInsets.all(24),
              decoration: BoxDecoration(
                color: const Color(0xFF16213E),
                borderRadius: BorderRadius.circular(20),
                border: Border.all(
                  color: const Color(0xFFE94560).withValues(alpha: 0.3),
                ),
              ),
              child: const Icon(
                Icons.bluetooth_disabled,
                size: 64,
                color: Color(0xFFE94560),
              ),
            ),
            const SizedBox(height: 24),
            const Text(
              'Not Connected',
              style: TextStyle(
                fontSize: 24,
                fontWeight: FontWeight.bold,
                color: Color(0xFFF5E6D3),
                fontFamily: 'Georgia',
              ),
            ),
            const SizedBox(height: 12),
            Text(
              'Please connect to Capyboo first\nfrom the home screen',
              textAlign: TextAlign.center,
              style: TextStyle(
                fontSize: 16,
                color: const Color(0xFFF5E6D3).withValues(alpha: 0.7),
                height: 1.5,
              ),
            ),
            const SizedBox(height: 32),
            ElevatedButton.icon(
              onPressed: () => Navigator.pop(context),
              icon: const Icon(Icons.arrow_back),
              label: const Text('Go Back'),
              style: ElevatedButton.styleFrom(
                backgroundColor: const Color(0xFFE94560),
                foregroundColor: Colors.white,
                padding: const EdgeInsets.symmetric(horizontal: 32, vertical: 16),
                shape: RoundedRectangleBorder(
                  borderRadius: BorderRadius.circular(12),
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildWifiForm(BleService bleService) {
    return SingleChildScrollView(
      padding: const EdgeInsets.all(24),
      child: Form(
        key: _formKey,
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: [
            // Header illustration
            Container(
              padding: const EdgeInsets.all(32),
              decoration: BoxDecoration(
                gradient: LinearGradient(
                  begin: Alignment.topLeft,
                  end: Alignment.bottomRight,
                  colors: [
                    const Color(0xFF16213E),
                    const Color(0xFF0F3460),
                  ],
                ),
                borderRadius: BorderRadius.circular(24),
                boxShadow: [
                  BoxShadow(
                    color: const Color(0xFFE94560).withValues(alpha: 0.2),
                    blurRadius: 20,
                    offset: const Offset(0, 10),
                  ),
                ],
              ),
              child: Column(
                children: [
                  const Icon(
                    Icons.wifi,
                    size: 64,
                    color: Color(0xFF00D9FF),
                  ),
                  const SizedBox(height: 16),
                  const Text(
                    'Configure WiFi',
                    style: TextStyle(
                      fontSize: 22,
                      fontWeight: FontWeight.bold,
                      color: Color(0xFFF5E6D3),
                      fontFamily: 'Georgia',
                    ),
                  ),
                  const SizedBox(height: 8),
                  Text(
                    'Enter your WiFi credentials to connect\nCapyboo to the internet',
                    textAlign: TextAlign.center,
                    style: TextStyle(
                      fontSize: 14,
                      color: const Color(0xFFF5E6D3).withValues(alpha: 0.7),
                      height: 1.5,
                    ),
                  ),
                ],
              ),
            ),

            const SizedBox(height: 32),

            // SSID Field
            TextFormField(
              controller: _ssidController,
              style: const TextStyle(color: Color(0xFFF5E6D3)),
              decoration: InputDecoration(
                labelText: 'WiFi Network Name (SSID)',
                labelStyle: TextStyle(
                  color: const Color(0xFFF5E6D3).withValues(alpha: 0.7),
                ),
                prefixIcon: const Icon(
                  Icons.router,
                  color: Color(0xFF00D9FF),
                ),
                filled: true,
                fillColor: const Color(0xFF16213E),
                border: OutlineInputBorder(
                  borderRadius: BorderRadius.circular(16),
                  borderSide: BorderSide.none,
                ),
                focusedBorder: OutlineInputBorder(
                  borderRadius: BorderRadius.circular(16),
                  borderSide: const BorderSide(
                    color: Color(0xFF00D9FF),
                    width: 2,
                  ),
                ),
                errorBorder: OutlineInputBorder(
                  borderRadius: BorderRadius.circular(16),
                  borderSide: const BorderSide(
                    color: Color(0xFFE94560),
                  ),
                ),
              ),
              validator: (value) {
                if (value == null || value.trim().isEmpty) {
                  return 'Please enter the WiFi name';
                }
                return null;
              },
            ),

            const SizedBox(height: 20),

            // Password Field
            TextFormField(
              controller: _passwordController,
              obscureText: _obscurePassword,
              style: const TextStyle(color: Color(0xFFF5E6D3)),
              decoration: InputDecoration(
                labelText: 'WiFi Password',
                labelStyle: TextStyle(
                  color: const Color(0xFFF5E6D3).withValues(alpha: 0.7),
                ),
                prefixIcon: const Icon(
                  Icons.lock_outline,
                  color: Color(0xFF00D9FF),
                ),
                suffixIcon: IconButton(
                  icon: Icon(
                    _obscurePassword ? Icons.visibility_off : Icons.visibility,
                    color: const Color(0xFFF5E6D3).withValues(alpha: 0.5),
                  ),
                  onPressed: () {
                    setState(() => _obscurePassword = !_obscurePassword);
                  },
                ),
                filled: true,
                fillColor: const Color(0xFF16213E),
                border: OutlineInputBorder(
                  borderRadius: BorderRadius.circular(16),
                  borderSide: BorderSide.none,
                ),
                focusedBorder: OutlineInputBorder(
                  borderRadius: BorderRadius.circular(16),
                  borderSide: const BorderSide(
                    color: Color(0xFF00D9FF),
                    width: 2,
                  ),
                ),
                errorBorder: OutlineInputBorder(
                  borderRadius: BorderRadius.circular(16),
                  borderSide: const BorderSide(
                    color: Color(0xFFE94560),
                  ),
                ),
              ),
              validator: (value) {
                if (value == null || value.isEmpty) {
                  return 'Please enter the WiFi password';
                }
                return null;
              },
            ),

            const SizedBox(height: 32),

            // Send Button
            ElevatedButton(
              onPressed: _isSending ? null : _sendWifiCredentials,
              style: ElevatedButton.styleFrom(
                backgroundColor: const Color(0xFF00D9FF),
                foregroundColor: const Color(0xFF1A1A2E),
                padding: const EdgeInsets.symmetric(vertical: 18),
                shape: RoundedRectangleBorder(
                  borderRadius: BorderRadius.circular(16),
                ),
                elevation: 8,
                shadowColor: const Color(0xFF00D9FF).withValues(alpha: 0.4),
              ),
              child: _isSending
                  ? const SizedBox(
                      height: 24,
                      width: 24,
                      child: CircularProgressIndicator(
                        strokeWidth: 2,
                        valueColor: AlwaysStoppedAnimation(Color(0xFF1A1A2E)),
                      ),
                    )
                  : const Row(
                      mainAxisAlignment: MainAxisAlignment.center,
                      children: [
                        Icon(Icons.send, size: 22),
                        SizedBox(width: 12),
                        Text(
                          'Send to Capyboo',
                          style: TextStyle(
                            fontSize: 16,
                            fontWeight: FontWeight.bold,
                            letterSpacing: 0.5,
                          ),
                        ),
                      ],
                    ),
            ),

            const SizedBox(height: 16),

            // Clear WiFi Button
            OutlinedButton.icon(
              onPressed: _clearWifiCredentials,
              icon: const Icon(Icons.delete_outline),
              label: const Text('Clear Stored WiFi'),
              style: OutlinedButton.styleFrom(
                foregroundColor: const Color(0xFFE94560),
                side: const BorderSide(color: Color(0xFFE94560)),
                padding: const EdgeInsets.symmetric(vertical: 16),
                shape: RoundedRectangleBorder(
                  borderRadius: BorderRadius.circular(16),
                ),
              ),
            ),

            const SizedBox(height: 24),

            // Response display
            if (bleService.lastResponse.isNotEmpty)
              Container(
                padding: const EdgeInsets.all(16),
                decoration: BoxDecoration(
                  color: const Color(0xFF16213E),
                  borderRadius: BorderRadius.circular(16),
                  border: Border.all(
                    color: const Color(0xFF4CAF50).withValues(alpha: 0.3),
                  ),
                ),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Row(
                      children: [
                        const Icon(
                          Icons.message,
                          color: Color(0xFF4CAF50),
                          size: 20,
                        ),
                        const SizedBox(width: 8),
                        Text(
                          'Response from Capyboo:',
                          style: TextStyle(
                            color: const Color(0xFFF5E6D3).withValues(alpha: 0.7),
                            fontSize: 12,
                            fontWeight: FontWeight.w600,
                          ),
                        ),
                      ],
                    ),
                    const SizedBox(height: 8),
                    Text(
                      bleService.lastResponse,
                      style: const TextStyle(
                        color: Color(0xFF4CAF50),
                        fontSize: 14,
                        fontFamily: 'monospace',
                      ),
                    ),
                  ],
                ),
              ),
          ],
        ),
      ),
    );
  }
}

