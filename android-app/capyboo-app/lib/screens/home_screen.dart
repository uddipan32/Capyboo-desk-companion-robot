import 'package:flutter/material.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:provider/provider.dart';
import '../services/ble_service.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen>
    with SingleTickerProviderStateMixin {
  late AnimationController _pulseController;
  late Animation<double> _pulseAnimation;
  bool _isAnimating = false;

  @override
  void initState() {
    super.initState();
    _pulseController = AnimationController(
      duration: const Duration(milliseconds: 1500),
      vsync: this,
    );

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

  void _updateAnimation(bool shouldAnimate) {
    if (shouldAnimate && !_isAnimating) {
      _pulseController.repeat(reverse: true);
      _isAnimating = true;
    } else if (!shouldAnimate && _isAnimating) {
      _pulseController.stop();
      _pulseController.reset();
      _isAnimating = false;
    }
  }

  Future<void> _initializeBle() async {
    // Request permissions
    await _requestPermissions();

    // Initialize BLE service and auto-connect
    final bleService = context.read<BleService>();
    final initialized = await bleService.initialize();

    // Auto-scan and connect on app start
    if (initialized) {
      await bleService.startScan(autoConnect: true);
    }
  }

  Future<void> _sendTime(BleService bleService) async {
    // get current time
    final now = DateTime.now();
    final hour = now.hour.toString().padLeft(2, '0');
    final minute = now.minute.toString().padLeft(2, '0');
    final second = now.second.toString().padLeft(2, '0');

    final date = now.day.toString().padLeft(2, '0');
    final month = now.month.toString().padLeft(2, '0');
    final year = now.year.toString();

    final timeCommand = 'time:$hour:$minute:$second $date/$month/$year';
    final success = await bleService.sendCommand(timeCommand);
    if (mounted) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text(success ? '✓ Time sent' : '✗ Failed to send time'),
        ),
      );
    }
  }

  Future<void> _sendWeather(BleService bleService) async {
    final weatherCmmand = 'weather:London:20:15:50';

    // get current weather
    final success = await bleService.sendCommand(weatherCmmand);
    if (mounted) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text(
            success ? '✓ Weather sent' : '✗ Failed to send weather',
          ),
        ),
      );
    }
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

    // Start scanning with auto-connect enabled
    await bleService.startScan(autoConnect: true);
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
                // SliverAppBar(
                //   expandedHeight: 120,
                //   floating: true,
                //   backgroundColor: Colors.transparent,
                //   flexibleSpace: FlexibleSpaceBar(
                //     title: const Text(
                //       'Capyboo',
                //       style: TextStyle(
                //         fontFamily: 'Georgia',
                //         fontWeight: FontWeight.bold,
                //         fontSize: 28,
                //         color: Color(0xFFF5E6D3),
                //         letterSpacing: 1.5,
                //       ),
                //     ),
                //     centerTitle: true,
                //     background: Container(
                //       decoration: BoxDecoration(
                //         gradient: LinearGradient(
                //           begin: Alignment.topCenter,
                //           end: Alignment.bottomCenter,
                //           colors: [
                //             const Color(0xFF0F3460).withValues(alpha: 0.8),
                //             Colors.transparent,
                //           ],
                //         ),
                //       ),
                //     ),
                //   ),
                // ),

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

                        // Mode Selection (when connected)
                        if (bleService.isConnected)
                          _buildModeSelection(bleService),
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
    final isScanning =
        bleService.connectionState == BleConnectionState.scanning;
    final isConnecting =
        bleService.connectionState == BleConnectionState.connecting;

    // Only animate when scanning or connecting
    final shouldAnimate = isScanning || isConnecting;
    WidgetsBinding.instance.addPostFrameCallback((_) {
      _updateAnimation(shouldAnimate);
    });

    return AnimatedBuilder(
      animation: _pulseAnimation,
      builder: (context, child) {
        return Transform.scale(
          scale: shouldAnimate ? _pulseAnimation.value : 1.0,
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
                  color:
                      (isConnected
                              ? const Color(0xFF4CAF50)
                              : const Color(0xFFE94560))
                          .withValues(alpha: 0.3),
                  blurRadius: 24,
                  offset: const Offset(0, 12),
                ),
              ],
              border: Border.all(
                color:
                    (isConnected
                            ? const Color(0xFF4CAF50)
                            : const Color(0xFFE94560))
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
                        color:
                            (isConnected
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
    final isBusy =
        bleService.connectionState == BleConnectionState.scanning ||
        bleService.connectionState == BleConnectionState.connecting;

    return SizedBox(
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
          shadowColor:
              (isConnected ? const Color(0xFFE94560) : const Color(0xFF00D9FF))
                  .withValues(alpha: 0.4),
        ),
      ),
    );
  }

  Widget _buildModeSelection(BleService bleService) {
    final modes = [
      {
        'icon': Icons.animation,
        'label': 'Animations',
        'command': 'mode:animation',
        'color': const Color(0xFFFF6B6B),
        'gradient': [const Color(0xFFFF6B6B), const Color(0xFFFF8E53)],
      },
      {
        'icon': Icons.sports_esports,
        'label': 'Game',
        'command': 'mode:game',
        'color': const Color(0xFF4ECDC4),
        'gradient': [const Color(0xFF4ECDC4), const Color(0xFF44A08D)],
      },
      {
        'icon': Icons.cloud,
        'label': 'Weather',
        'command': 'mode:weather',
        'color': const Color(0xFF667EEA),
        'gradient': [const Color(0xFF667EEA), const Color(0xFF764BA2)],
      },
      {
        'icon': Icons.access_time,
        'label': 'Clock',
        'command': 'mode:clock',
        'color': const Color(0xFFFFD93D),
        'gradient': [const Color(0xFFFFD93D), const Color(0xFFFF6B6B)],
      },
    ];

    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Row(
          children: [
            Icon(Icons.dashboard, color: Color(0xFFFFD700), size: 22),
            SizedBox(width: 8),
            Text(
              'Modes',
              style: TextStyle(
                fontSize: 20,
                fontWeight: FontWeight.bold,
                color: Color(0xFFF5E6D3),
                fontFamily: 'Georgia',
              ),
            ),
          ],
        ),
        const SizedBox(height: 16),
        GridView.builder(
          shrinkWrap: true,
          physics: const NeverScrollableScrollPhysics(),
          gridDelegate: const SliverGridDelegateWithFixedCrossAxisCount(
            crossAxisCount: 2,
            crossAxisSpacing: 16,
            mainAxisSpacing: 16,
            childAspectRatio: 1.1,
          ),
          itemCount: modes.length,
          itemBuilder: (context, index) {
            final mode = modes[index];
            return _buildModeCard(
              icon: mode['icon'] as IconData,
              label: mode['label'] as String,
              command: mode['command'] as String,
              gradient: mode['gradient'] as List<Color>,
              bleService: bleService,
            );
          },
        ),
      ],
    );
  }

  Widget _buildModeCard({
    required IconData icon,
    required String label,
    required String command,
    required List<Color> gradient,
    required BleService bleService,
  }) {
    return GestureDetector(
      onTap: () async {
        if (command == 'mode:weather') {
          await _sendWeather(bleService);
        } else if (command == 'mode:clock') {
          await _sendTime(bleService);
        }
        // delay
        await Future.delayed(const Duration(seconds: 1));
        final success = await bleService.sendCommand(command);
        // delay for 1 second
        await Future.delayed(const Duration(seconds: 1));

        if (mounted) {
          ScaffoldMessenger.of(context).showSnackBar(
            SnackBar(
              content: Text(
                success ? '✓ Mode: $label activated' : '✗ Failed to set mode',
              ),
              duration: const Duration(seconds: 1),
              behavior: SnackBarBehavior.floating,
              backgroundColor: success
                  ? const Color(0xFF4CAF50)
                  : const Color(0xFFE94560),
              shape: RoundedRectangleBorder(
                borderRadius: BorderRadius.circular(12),
              ),
            ),
          );
        }
      },
      child: Container(
        decoration: BoxDecoration(
          gradient: LinearGradient(
            begin: Alignment.topLeft,
            end: Alignment.bottomRight,
            colors: gradient,
          ),
          borderRadius: BorderRadius.circular(20),
          boxShadow: [
            BoxShadow(
              color: gradient[0].withValues(alpha: 0.4),
              blurRadius: 12,
              offset: const Offset(0, 6),
            ),
          ],
        ),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Icon(icon, size: 48, color: Colors.white),
            const SizedBox(height: 12),
            Text(
              label,
              style: const TextStyle(
                fontSize: 16,
                fontWeight: FontWeight.bold,
                color: Colors.white,
                letterSpacing: 0.5,
              ),
            ),
          ],
        ),
      ),
    );
  }
}
