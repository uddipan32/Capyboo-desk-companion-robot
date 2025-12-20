import 'package:flutter/material.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:provider/provider.dart';
import '../services/ble_service.dart';
import 'wifi_setup_screen.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> with SingleTickerProviderStateMixin {
  late AnimationController _pulseController;
  late Animation<double> _pulseAnimation;

  @override
  void initState() {
    super.initState();
    _pulseController = AnimationController(
      duration: const Duration(milliseconds: 1500),
      vsync: this,
    )..repeat(reverse: true);
    
    _pulseAnimation = Tween<double>(begin: 1.0, end: 1.1).animate(
      CurvedAnimation(parent: _pulseController, curve: Curves.easeInOut),
    );
    
    _initializeBle();
  }

  @override
  void dispose() {
    _pulseController.dispose();
    super.dispose();
  }

  Future<void> _initializeBle() async {
    // Request permissions
    await _requestPermissions();
    
    // Initialize BLE service
    final bleService = context.read<BleService>();
    await bleService.initialize();
  }

  Future<void> _requestPermissions() async {
    await [
      Permission.bluetooth,
      Permission.bluetoothScan,
      Permission.bluetoothConnect,
      Permission.location,
    ].request();
  }

  Future<void> _scanAndConnect() async {
    final bleService = context.read<BleService>();
    
    // Start scanning
    await bleService.startScan(timeout: const Duration(seconds: 8));
    
    // Auto-connect if Capyboo found
    if (bleService.scannedDevices.isNotEmpty) {
      await bleService.connect(bleService.scannedDevices.first);
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFF1A1A2E),
      body: SafeArea(
        child: Consumer<BleService>(
          builder: (context, bleService, _) {
            return CustomScrollView(
              slivers: [
                // App Bar
                SliverAppBar(
                  expandedHeight: 120,
                  floating: true,
                  backgroundColor: Colors.transparent,
                  flexibleSpace: FlexibleSpaceBar(
                    title: const Text(
                      'Capyboo',
                      style: TextStyle(
                        fontFamily: 'Georgia',
                        fontWeight: FontWeight.bold,
                        fontSize: 28,
                        color: Color(0xFFF5E6D3),
                        letterSpacing: 1.5,
                      ),
                    ),
                    centerTitle: true,
                    background: Container(
                      decoration: BoxDecoration(
                        gradient: LinearGradient(
                          begin: Alignment.topCenter,
                          end: Alignment.bottomCenter,
                          colors: [
                            const Color(0xFF0F3460).withValues(alpha: 0.8),
                            Colors.transparent,
                          ],
                        ),
                      ),
                    ),
                  ),
                ),

                // Content
                SliverToBoxAdapter(
                  child: Padding(
                    padding: const EdgeInsets.all(24),
                    child: Column(
                      children: [
                        // Connection Status Card
                        _buildConnectionCard(bleService),
                        
                        const SizedBox(height: 24),
                        
                        // Action Buttons
                        _buildActionButtons(bleService),
                        
                        const SizedBox(height: 24),
                        
                        // Quick Commands (when connected)
                        if (bleService.isConnected)
                          _buildQuickCommands(bleService),
                      ],
                    ),
                  ),
                ),
              ],
            );
          },
        ),
      ),
    );
  }

  Widget _buildConnectionCard(BleService bleService) {
    final isConnected = bleService.isConnected;
    final isScanning = bleService.connectionState == BleConnectionState.scanning;
    final isConnecting = bleService.connectionState == BleConnectionState.connecting;

    return AnimatedBuilder(
      animation: _pulseAnimation,
      builder: (context, child) {
        return Transform.scale(
          scale: (isScanning || isConnecting) ? _pulseAnimation.value : 1.0,
          child: Container(
            padding: const EdgeInsets.all(32),
            decoration: BoxDecoration(
              gradient: LinearGradient(
                begin: Alignment.topLeft,
                end: Alignment.bottomRight,
                colors: isConnected
                    ? [const Color(0xFF1B4332), const Color(0xFF2D6A4F)]
                    : [const Color(0xFF16213E), const Color(0xFF0F3460)],
              ),
              borderRadius: BorderRadius.circular(28),
              boxShadow: [
                BoxShadow(
                  color: (isConnected ? const Color(0xFF4CAF50) : const Color(0xFFE94560))
                      .withValues(alpha: 0.3),
                  blurRadius: 24,
                  offset: const Offset(0, 12),
                ),
              ],
              border: Border.all(
                color: (isConnected ? const Color(0xFF4CAF50) : const Color(0xFFE94560))
                    .withValues(alpha: 0.3),
                width: 1.5,
              ),
            ),
            child: Column(
              children: [
                // Capyboo Avatar
                Container(
                  width: 100,
                  height: 100,
                  decoration: BoxDecoration(
                    shape: BoxShape.circle,
                    color: const Color(0xFF1A1A2E),
                    border: Border.all(
                      color: isConnected
                          ? const Color(0xFF4CAF50)
                          : const Color(0xFFE94560),
                      width: 3,
                    ),
                    boxShadow: [
                      BoxShadow(
                        color: (isConnected
                                ? const Color(0xFF4CAF50)
                                : const Color(0xFFE94560))
                            .withValues(alpha: 0.4),
                        blurRadius: 20,
                        spreadRadius: 2,
                      ),
                    ],
                  ),
                  child: Center(
                    child: Text(
                      isConnected ? '🦫' : '💤',
                      style: const TextStyle(fontSize: 48),
                    ),
                  ),
                ),

                const SizedBox(height: 20),

                // Status Text
                Text(
                  isConnected
                      ? 'Connected'
                      : isScanning
                          ? 'Searching...'
                          : isConnecting
                              ? 'Connecting...'
                              : 'Disconnected',
                  style: TextStyle(
                    fontSize: 24,
                    fontWeight: FontWeight.bold,
                    color: isConnected
                        ? const Color(0xFF4CAF50)
                        : const Color(0xFFF5E6D3),
                    fontFamily: 'Georgia',
                    letterSpacing: 0.5,
                  ),
                ),

                const SizedBox(height: 8),

                // Status Message
                Text(
                  bleService.statusMessage,
                  textAlign: TextAlign.center,
                  style: TextStyle(
                    fontSize: 14,
                    color: const Color(0xFFF5E6D3).withValues(alpha: 0.7),
                    height: 1.4,
                  ),
                ),

                // Loading indicator
                if (isScanning || isConnecting) ...[
                  const SizedBox(height: 20),
                  SizedBox(
                    width: 32,
                    height: 32,
                    child: CircularProgressIndicator(
                      strokeWidth: 3,
                      valueColor: AlwaysStoppedAnimation(
                        isConnected
                            ? const Color(0xFF4CAF50)
                            : const Color(0xFF00D9FF),
                      ),
                    ),
                  ),
                ],
              ],
            ),
          ),
        );
      },
    );
  }

  Widget _buildActionButtons(BleService bleService) {
    final isConnected = bleService.isConnected;
    final isBusy = bleService.connectionState == BleConnectionState.scanning ||
        bleService.connectionState == BleConnectionState.connecting;

    return Column(
      children: [
        // Connect/Disconnect Button
        SizedBox(
          width: double.infinity,
          child: ElevatedButton.icon(
            onPressed: isBusy
                ? null
                : isConnected
                    ? () => bleService.disconnect()
                    : _scanAndConnect,
            icon: Icon(
              isConnected ? Icons.bluetooth_disabled : Icons.bluetooth_searching,
              size: 24,
            ),
            label: Text(
              isConnected ? 'Disconnect' : 'Connect to Capyboo',
              style: const TextStyle(
                fontSize: 16,
                fontWeight: FontWeight.bold,
                letterSpacing: 0.5,
              ),
            ),
            style: ElevatedButton.styleFrom(
              backgroundColor: isConnected
                  ? const Color(0xFFE94560)
                  : const Color(0xFF00D9FF),
              foregroundColor: isConnected ? Colors.white : const Color(0xFF1A1A2E),
              padding: const EdgeInsets.symmetric(vertical: 18),
              shape: RoundedRectangleBorder(
                borderRadius: BorderRadius.circular(16),
              ),
              elevation: 8,
              shadowColor: (isConnected
                      ? const Color(0xFFE94560)
                      : const Color(0xFF00D9FF))
                  .withValues(alpha: 0.4),
            ),
          ),
        ),

        const SizedBox(height: 16),

        // WiFi Setup Button
        SizedBox(
          width: double.infinity,
          child: OutlinedButton.icon(
            onPressed: () {
              Navigator.push(
                context,
                MaterialPageRoute(
                  builder: (_) => const WifiSetupScreen(),
                ),
              );
            },
            icon: const Icon(Icons.wifi, size: 22),
            label: const Text(
              'WiFi Setup',
              style: TextStyle(
                fontSize: 16,
                fontWeight: FontWeight.w600,
              ),
            ),
            style: OutlinedButton.styleFrom(
              foregroundColor: const Color(0xFF00D9FF),
              side: const BorderSide(color: Color(0xFF00D9FF), width: 2),
              padding: const EdgeInsets.symmetric(vertical: 16),
              shape: RoundedRectangleBorder(
                borderRadius: BorderRadius.circular(16),
              ),
            ),
          ),
        ),
      ],
    );
  }

  Widget _buildQuickCommands(BleService bleService) {
    final commands = [
      {'icon': Icons.wb_sunny, 'label': 'Wakeup', 'command': 'wakeup'},
      {'icon': Icons.arrow_forward, 'label': 'Look Right', 'command': 'lookright'},
      {'icon': Icons.arrow_back, 'label': 'Look Left', 'command': 'lookleft'},
      {'icon': Icons.tag_faces, 'label': 'Funny Eyes', 'command': 'funnyeyes'},
      {'icon': Icons.sentiment_very_satisfied, 'label': 'Tongue', 'command': 'tongue'},
    ];

    return Container(
      padding: const EdgeInsets.all(20),
      decoration: BoxDecoration(
        color: const Color(0xFF16213E),
        borderRadius: BorderRadius.circular(20),
        border: Border.all(
          color: const Color(0xFF00D9FF).withValues(alpha: 0.2),
        ),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Row(
            children: [
              Icon(
                Icons.flash_on,
                color: Color(0xFFFFD700),
                size: 22,
              ),
              SizedBox(width: 8),
              Text(
                'Quick Commands',
                style: TextStyle(
                  fontSize: 18,
                  fontWeight: FontWeight.bold,
                  color: Color(0xFFF5E6D3),
                  fontFamily: 'Georgia',
                ),
              ),
            ],
          ),
          const SizedBox(height: 16),
          Wrap(
            spacing: 10,
            runSpacing: 10,
            children: commands.map((cmd) {
              return ActionChip(
                avatar: Icon(
                  cmd['icon'] as IconData,
                  size: 18,
                  color: const Color(0xFF00D9FF),
                ),
                label: Text(
                  cmd['label'] as String,
                  style: const TextStyle(
                    color: Color(0xFFF5E6D3),
                    fontWeight: FontWeight.w500,
                  ),
                ),
                backgroundColor: const Color(0xFF0F3460),
                side: BorderSide.none,
                shape: RoundedRectangleBorder(
                  borderRadius: BorderRadius.circular(12),
                ),
                onPressed: () async {
                  final success = await bleService.sendCommand(cmd['command'] as String);
                  if (mounted) {
                    ScaffoldMessenger.of(context).showSnackBar(
                      SnackBar(
                        content: Text(
                          success
                              ? '✓ Sent: ${cmd['label']}'
                              : '✗ Failed to send command',
                        ),
                        duration: const Duration(seconds: 1),
                        behavior: SnackBarBehavior.floating,
                        shape: RoundedRectangleBorder(
                          borderRadius: BorderRadius.circular(12),
                        ),
                      ),
                    );
                  }
                },
              );
            }).toList(),
          ),
        ],
      ),
    );
  }
}

