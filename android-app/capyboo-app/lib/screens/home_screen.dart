import 'dart:convert';
import 'dart:math';
import 'package:flutter/material.dart';
import 'package:geolocator/geolocator.dart';
import 'package:geocoding/geocoding.dart';
import 'package:http/http.dart' as http;
import 'package:permission_handler/permission_handler.dart';
import 'package:provider/provider.dart';
import '../services/ble_service.dart';
import '../constants/credentials.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> with TickerProviderStateMixin {
  late AnimationController _floatController;
  late AnimationController _pulseController;
  late Animation<double> _floatAnimation;
  late Animation<double> _pulseAnimation;
  bool _isAnimating = false;

  @override
  void initState() {
    super.initState();

    // Floating animation for the avatar
    _floatController = AnimationController(
      duration: const Duration(milliseconds: 2000),
      vsync: this,
    )..repeat(reverse: true);

    _floatAnimation = Tween<double>(begin: -8, end: 8).animate(
      CurvedAnimation(parent: _floatController, curve: Curves.easeInOut),
    );

    // Pulse animation for scanning
    _pulseController = AnimationController(
      duration: const Duration(milliseconds: 1500),
      vsync: this,
    );

    _pulseAnimation = Tween<double>(begin: 1.0, end: 1.08).animate(
      CurvedAnimation(parent: _pulseController, curve: Curves.easeInOut),
    );

    _initializeBle();
  }

  @override
  void dispose() {
    _floatController.dispose();
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
    await _requestPermissions();
    final bleService = context.read<BleService>();
    final initialized = await bleService.initialize();
    if (initialized) {
      await bleService.startScan(autoConnect: true);
    }
  }

  Future<void> _sendTime(BleService bleService) async {
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
      _showCuteSnackBar(
        success ? '⏰ Time synced!' : '😢 Failed to sync time',
        success,
      );
    }
  }

  Future<void> _sendWeather(BleService bleService) async {
    try {
      final location = await _getCurrentLocation();
      if (location == null) {
        if (mounted) _showCuteSnackBar('📍 Could not get location', false);
        return;
      }

      final cityName = await _getCityName(
        location.latitude,
        location.longitude,
      );
      final weatherData = await _getWeatherData(cityName);

      final temp = weatherData?['main']?['temp']?.round() ?? 20;
      final humidity = weatherData?['main']?['humidity'] ?? 50;
      final feelsLike = weatherData?['main']?['feels_like']?.round() ?? temp;
      final description =
          weatherData?['weather']?[0]?['description'] ?? 'Unknown';

      final weatherCommand =
          'weather:$cityName:$temp:$feelsLike:$humidity:$description';
      final success = await bleService.sendCommand(weatherCommand);
      if (mounted) {
        _showCuteSnackBar(
          success ? '☀️ Weather sent for $cityName!' : '😢 Failed',
          success,
        );
      }
    } catch (e) {
      if (mounted) _showCuteSnackBar('😢 Error: $e', false);
    }
  }

  void _showCuteSnackBar(String message, bool success) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Row(
          children: [
            Text(message, style: const TextStyle(fontSize: 15)),
            if (success) ...[
              const SizedBox(width: 8),
              const Text('✨', style: TextStyle(fontSize: 18)),
            ],
          ],
        ),
        behavior: SnackBarBehavior.floating,
        backgroundColor: success
            ? const Color(0xFF81C784)
            : const Color(0xFFE57373),
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(20)),
        margin: const EdgeInsets.all(16),
      ),
    );
  }

  Future<Position?> _getCurrentLocation() async {
    bool serviceEnabled = await Geolocator.isLocationServiceEnabled();
    if (!serviceEnabled) {
      if (mounted)
        _showCuteSnackBar('📍 Please enable location services', false);
      return null;
    }

    LocationPermission permission = await Geolocator.checkPermission();
    if (permission == LocationPermission.denied) {
      permission = await Geolocator.requestPermission();
      if (permission == LocationPermission.denied) {
        if (mounted) _showCuteSnackBar('📍 Location permission denied', false);
        return null;
      }
    }

    if (permission == LocationPermission.deniedForever) {
      if (mounted)
        _showCuteSnackBar('📍 Please enable location in settings', false);
      return null;
    }

    return await Geolocator.getCurrentPosition(
      locationSettings: const LocationSettings(
        accuracy: LocationAccuracy.medium,
        timeLimit: Duration(seconds: 10),
      ),
    );
  }

  Future<String> _getCityName(double latitude, double longitude) async {
    try {
      List<Placemark> placemarks = await placemarkFromCoordinates(
        latitude,
        longitude,
      );
      if (placemarks.isNotEmpty) {
        final place = placemarks.first;
        return place.locality ??
            place.subAdministrativeArea ??
            place.administrativeArea ??
            'Unknown';
      }
    } catch (e) {
      debugPrint('Geocoding error: $e');
    }
    return 'Unknown';
  }

  Future<Map<String, dynamic>?> _getWeatherData(String cityName) async {
    const apiKey = WEATHER_API_KEY;
    final url =
        'https://api.openweathermap.org/data/2.5/weather?q=$cityName&appid=$apiKey&units=metric';

    try {
      final response = await http.get(Uri.parse(url));
      if (response.statusCode == 200) {
        return json.decode(response.body);
      }
    } catch (e) {
      debugPrint('Weather fetch error: $e');
    }
    return null;
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
    await bleService.startScan(autoConnect: true);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Container(
        decoration: const BoxDecoration(
          gradient: LinearGradient(
            begin: Alignment.topCenter,
            end: Alignment.bottomCenter,
            colors: [
              Color(0xFF87CEEB), // Sky blue
              Color(0xFFB8E4F0), // Lighter blue
              Color(0xFFE8F6F8), // Very light blue
            ],
          ),
        ),
        child: SafeArea(
          child: Consumer<BleService>(
            builder: (context, bleService, _) {
              return Stack(
                children: [
                  // Cute floating clouds/bubbles
                  ..._buildFloatingDecorations(),

                  // Main content
                  SingleChildScrollView(
                    padding: const EdgeInsets.all(20),
                    child: Column(
                      children: [
                        const SizedBox(height: 20),

                        // Cute title
                        _buildTitle(),

                        const SizedBox(height: 30),

                        // Avatar card
                        _buildAvatarCard(bleService),

                        const SizedBox(height: 24),

                        // Connect button
                        _buildConnectButton(bleService),

                        const SizedBox(height: 28),

                        // Mode cards or waiting message
                        if (bleService.isConnected)
                          _buildModeSection(bleService)
                        else
                          _buildWaitingSection(bleService),
                      ],
                    ),
                  ),
                ],
              );
            },
          ),
        ),
      ),
    );
  }

  List<Widget> _buildFloatingDecorations() {
    return [
      // Cloud 1
      Positioned(top: 60, left: 20, child: _buildCloud(0.6)),
      // Cloud 2
      Positioned(top: 120, right: 30, child: _buildCloud(0.4)),
      // Sparkle
      Positioned(top: 180, left: 60, child: _buildSparkle()),
      // Another sparkle
      Positioned(bottom: 200, right: 40, child: _buildSparkle()),
    ];
  }

  Widget _buildCloud(double opacity) {
    return Opacity(
      opacity: opacity,
      child: Container(
        width: 80,
        height: 40,
        decoration: BoxDecoration(
          color: Colors.white,
          borderRadius: BorderRadius.circular(20),
          boxShadow: [
            BoxShadow(
              color: Colors.white.withValues(alpha: 0.5),
              blurRadius: 10,
              spreadRadius: 2,
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildSparkle() {
    return AnimatedBuilder(
      animation: _floatController,
      builder: (context, child) {
        return Transform.rotate(
          angle: _floatController.value * 2 * pi,
          child: const Text('✨', style: TextStyle(fontSize: 20)),
        );
      },
    );
  }

  Widget _buildTitle() {
    return Column(
      children: [
        ShaderMask(
          shaderCallback: (bounds) => const LinearGradient(
            colors: [Color(0xFF5EB5F7), Color(0xFF4ECDC4)],
          ).createShader(bounds),
          child: const Text(
            '🤖 Capyboo',
            style: TextStyle(
              fontSize: 36,
              fontWeight: FontWeight.bold,
              color: Colors.white,
              letterSpacing: 1.5,
            ),
          ),
        ),
        const SizedBox(height: 4),
        Text(
          'Your cute desk buddy! 💕',
          style: TextStyle(
            fontSize: 16,
            color: Colors.blueGrey.shade600,
            fontWeight: FontWeight.w500,
          ),
        ),
      ],
    );
  }

  Widget _buildAvatarCard(BleService bleService) {
    final isConnected = bleService.isConnected;
    final isScanning =
        bleService.connectionState == BleConnectionState.scanning;
    final isConnecting =
        bleService.connectionState == BleConnectionState.connecting;
    final shouldAnimate = isScanning || isConnecting;

    WidgetsBinding.instance.addPostFrameCallback((_) {
      _updateAnimation(shouldAnimate);
    });

    return AnimatedBuilder(
      animation: Listenable.merge([_floatAnimation, _pulseAnimation]),
      builder: (context, child) {
        return Transform.translate(
          offset: Offset(0, _floatAnimation.value),
          child: Transform.scale(
            scale: shouldAnimate ? _pulseAnimation.value : 1.0,
            child: Container(
              padding: const EdgeInsets.all(28),
              decoration: BoxDecoration(
                color: Colors.white,
                borderRadius: BorderRadius.circular(32),
                boxShadow: [
                  BoxShadow(
                    color:
                        (isConnected
                                ? const Color(0xFF81C784)
                                : const Color(0xFF5EB5F7))
                            .withValues(alpha: 0.3),
                    blurRadius: 30,
                    offset: const Offset(0, 15),
                    spreadRadius: 5,
                  ),
                ],
                border: Border.all(
                  color: isConnected
                      ? const Color(0xFF81C784)
                      : const Color(0xFF5EB5F7),
                  width: 3,
                ),
              ),
              child: Column(
                children: [
                  // Avatar with glow
                  Container(
                    padding: const EdgeInsets.all(8),
                    decoration: BoxDecoration(
                      shape: BoxShape.circle,
                      gradient: RadialGradient(
                        colors: [
                          (isConnected
                                  ? const Color(0xFF81C784)
                                  : const Color(0xFF5EB5F7))
                              .withValues(alpha: 0.3),
                          Colors.transparent,
                        ],
                      ),
                    ),
                    child: Container(
                      width: 120,
                      height: 120,
                      decoration: BoxDecoration(
                        shape: BoxShape.circle,
                        border: Border.all(
                          color: isConnected
                              ? const Color(0xFF81C784)
                              : const Color(0xFF5EB5F7),
                          width: 4,
                        ),
                        boxShadow: [
                          BoxShadow(
                            color:
                                (isConnected
                                        ? const Color(0xFF81C784)
                                        : const Color(0xFF5EB5F7))
                                    .withValues(alpha: 0.5),
                            blurRadius: 20,
                            spreadRadius: 2,
                          ),
                        ],
                      ),
                      child: ClipOval(
                        child: Image.asset(
                          'assets/logo.png',
                          fit: BoxFit.cover,
                          opacity: AlwaysStoppedAnimation(
                            isConnected ? 1.0 : 0.7,
                          ),
                        ),
                      ),
                    ),
                  ),

                  const SizedBox(height: 20),

                  // Status with emoji
                  Row(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      Text(
                        isConnected
                            ? '💚'
                            : isScanning
                            ? '🔍'
                            : isConnecting
                            ? '🔗'
                            : '💤',
                        style: const TextStyle(fontSize: 24),
                      ),
                      const SizedBox(width: 8),
                      Text(
                        isConnected
                            ? 'Connected!'
                            : isScanning
                            ? 'Searching...'
                            : isConnecting
                            ? 'Connecting...'
                            : 'Sleeping',
                        style: TextStyle(
                          fontSize: 22,
                          fontWeight: FontWeight.bold,
                          color: isConnected
                              ? const Color(0xFF4CAF50)
                              : Colors.blueGrey.shade700,
                        ),
                      ),
                    ],
                  ),

                  const SizedBox(height: 8),

                  // Status message
                  Text(
                    bleService.statusMessage,
                    textAlign: TextAlign.center,
                    style: TextStyle(
                      fontSize: 13,
                      color: Colors.blueGrey.shade500,
                    ),
                  ),

                  // Loading animation
                  if (shouldAnimate) ...[
                    const SizedBox(height: 16),
                    SizedBox(
                      width: 28,
                      height: 28,
                      child: CircularProgressIndicator(
                        strokeWidth: 3,
                        valueColor: const AlwaysStoppedAnimation(
                          Color(0xFF5EB5F7),
                        ),
                        backgroundColor: Colors.blue.shade100,
                      ),
                    ),
                  ],
                ],
              ),
            ),
          ),
        );
      },
    );
  }

  Widget _buildConnectButton(BleService bleService) {
    final isConnected = bleService.isConnected;
    final isBusy =
        bleService.connectionState == BleConnectionState.scanning ||
        bleService.connectionState == BleConnectionState.connecting;

    return GestureDetector(
      onTap: isBusy
          ? null
          : (isConnected ? () => bleService.disconnect() : _scanAndConnect),
      child: AnimatedContainer(
        duration: const Duration(milliseconds: 300),
        width: double.infinity,
        padding: const EdgeInsets.symmetric(vertical: 18),
        decoration: BoxDecoration(
          gradient: LinearGradient(
            colors: isConnected
                ? [const Color(0xFFFF8A80), const Color(0xFFFF5252)]
                : [const Color(0xFF5EB5F7), const Color(0xFF4ECDC4)],
          ),
          borderRadius: BorderRadius.circular(25),
          boxShadow: [
            BoxShadow(
              color:
                  (isConnected
                          ? const Color(0xFFFF5252)
                          : const Color(0xFF5EB5F7))
                      .withValues(alpha: 0.4),
              blurRadius: 15,
              offset: const Offset(0, 8),
            ),
          ],
        ),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Icon(
              isConnected
                  ? Icons.link_off_rounded
                  : Icons.bluetooth_searching_rounded,
              color: Colors.white,
              size: 24,
            ),
            const SizedBox(width: 10),
            Text(
              isConnected ? 'Disconnect 👋' : 'Connect to Capyboo 🔗',
              style: const TextStyle(
                fontSize: 17,
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

  Widget _buildWaitingSection(BleService bleService) {
    final isScanning =
        bleService.connectionState == BleConnectionState.scanning;
    final isConnecting =
        bleService.connectionState == BleConnectionState.connecting;

    return Container(
      padding: const EdgeInsets.all(24),
      decoration: BoxDecoration(
        color: Colors.white.withValues(alpha: 0.7),
        borderRadius: BorderRadius.circular(24),
        border: Border.all(
          color: const Color(0xFF5EB5F7).withValues(alpha: 0.3),
          width: 2,
        ),
      ),
      child: Column(
        children: [
          Text(
            isScanning
                ? '🔍'
                : isConnecting
                ? '🔗'
                : '💭',
            style: const TextStyle(fontSize: 48),
          ),
          const SizedBox(height: 16),
          Text(
            isScanning
                ? 'Looking for Capyboo...'
                : isConnecting
                ? 'Almost there...'
                : 'Waiting for connection',
            style: TextStyle(
              fontSize: 18,
              fontWeight: FontWeight.bold,
              color: Colors.blueGrey.shade700,
            ),
          ),
          const SizedBox(height: 8),
          Text(
            isScanning
                ? 'Make sure Capyboo is powered on! 🔌'
                : isConnecting
                ? 'Establishing connection... ✨'
                : 'Tap the button above to connect! 👆',
            textAlign: TextAlign.center,
            style: TextStyle(fontSize: 14, color: Colors.blueGrey.shade500),
          ),
          const SizedBox(height: 20),
          // Cute tips
          Container(
            padding: const EdgeInsets.all(16),
            decoration: BoxDecoration(
              color: const Color(0xFF5EB5F7).withValues(alpha: 0.1),
              borderRadius: BorderRadius.circular(16),
            ),
            child: Column(
              children: [
                Row(
                  children: [
                    const Text('💡', style: TextStyle(fontSize: 20)),
                    const SizedBox(width: 8),
                    Text(
                      'Quick Tips',
                      style: TextStyle(
                        fontSize: 14,
                        fontWeight: FontWeight.bold,
                        color: Colors.blueGrey.shade700,
                      ),
                    ),
                  ],
                ),
                const SizedBox(height: 12),
                _buildTipRow('🔋', 'Ensure Capyboo is powered on'),
                const SizedBox(height: 8),
                _buildTipRow('📶', 'Stay within Bluetooth range'),
                const SizedBox(height: 8),
                _buildTipRow('📱', 'Enable Bluetooth on your phone'),
              ],
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildTipRow(String emoji, String text) {
    return Row(
      children: [
        Text(emoji, style: const TextStyle(fontSize: 16)),
        const SizedBox(width: 10),
        Expanded(
          child: Text(
            text,
            style: TextStyle(fontSize: 13, color: Colors.blueGrey.shade600),
          ),
        ),
      ],
    );
  }

  Widget _buildModeSection(BleService bleService) {
    final modes = [
      {
        'emoji': '🎬',
        'label': 'Animations',
        'command': 'mode:animation',
        'colors': [const Color(0xFFFF6B6B), const Color(0xFFFF8E53)],
      },
      {
        'emoji': '🎮',
        'label': 'Game',
        'command': 'mode:game',
        'colors': [const Color(0xFF4ECDC4), const Color(0xFF44A08D)],
      },
      {
        'emoji': '☁️',
        'label': 'Weather',
        'command': 'mode:weather',
        'colors': [const Color(0xFF667EEA), const Color(0xFF764BA2)],
      },
      {
        'emoji': '⏰',
        'label': 'Clock',
        'command': 'mode:clock',
        'colors': [const Color(0xFFFFD93D), const Color(0xFFFF6B6B)],
      },
    ];

    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Row(
          children: [
            Text('✨', style: TextStyle(fontSize: 22)),
            SizedBox(width: 8),
            Text(
              'Choose a Mode',
              style: TextStyle(
                fontSize: 20,
                fontWeight: FontWeight.bold,
                color: Color(0xFF37474F),
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
            crossAxisSpacing: 14,
            mainAxisSpacing: 14,
            childAspectRatio: 1.15,
          ),
          itemCount: modes.length,
          itemBuilder: (context, index) {
            final mode = modes[index];
            return _CuteModeCard(
              emoji: mode['emoji'] as String,
              label: mode['label'] as String,
              command: mode['command'] as String,
              colors: mode['colors'] as List<Color>,
              onTap: () async {
                if (mode['command'] == 'mode:weather') {
                  await _sendWeather(bleService);
                } else if (mode['command'] == 'mode:clock') {
                  await _sendTime(bleService);
                }
                await Future.delayed(const Duration(milliseconds: 500));
                final success = await bleService.sendCommand(
                  mode['command'] as String,
                );
                if (mounted) {
                  _showCuteSnackBar(
                    success
                        ? '${mode['emoji']} ${mode['label']} activated!'
                        : '😢 Failed',
                    success,
                  );
                }
              },
            );
          },
        ),
      ],
    );
  }
}

class _CuteModeCard extends StatefulWidget {
  final String emoji;
  final String label;
  final String command;
  final List<Color> colors;
  final VoidCallback onTap;

  const _CuteModeCard({
    required this.emoji,
    required this.label,
    required this.command,
    required this.colors,
    required this.onTap,
  });

  @override
  State<_CuteModeCard> createState() => _CuteModeCardState();
}

class _CuteModeCardState extends State<_CuteModeCard>
    with SingleTickerProviderStateMixin {
  bool _isPressed = false;

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTapDown: (_) => setState(() => _isPressed = true),
      onTapUp: (_) {
        setState(() => _isPressed = false);
        widget.onTap();
      },
      onTapCancel: () => setState(() => _isPressed = false),
      child: AnimatedScale(
        scale: _isPressed ? 0.95 : 1.0,
        duration: const Duration(milliseconds: 100),
        child: Container(
          decoration: BoxDecoration(
            gradient: LinearGradient(
              begin: Alignment.topLeft,
              end: Alignment.bottomRight,
              colors: widget.colors,
            ),
            borderRadius: BorderRadius.circular(24),
            boxShadow: [
              BoxShadow(
                color: widget.colors[0].withValues(alpha: 0.4),
                blurRadius: 15,
                offset: const Offset(0, 8),
              ),
            ],
          ),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Text(widget.emoji, style: const TextStyle(fontSize: 40)),
              const SizedBox(height: 8),
              Text(
                widget.label,
                style: const TextStyle(
                  fontSize: 15,
                  fontWeight: FontWeight.bold,
                  color: Colors.white,
                  letterSpacing: 0.5,
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
