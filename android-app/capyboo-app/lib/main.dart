import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:provider/provider.dart';
import 'services/ble_service.dart';
import 'screens/home_screen.dart';

void main() {
  WidgetsFlutterBinding.ensureInitialized();

  // Set system UI overlay style
  SystemChrome.setSystemUIOverlayStyle(
    const SystemUiOverlayStyle(
      statusBarColor: Colors.transparent,
      statusBarIconBrightness: Brightness.light,
      systemNavigationBarColor: Color(0xFF1A1A2E),
      systemNavigationBarIconBrightness: Brightness.light,
    ),
  );

  runApp(const CapybooApp());
}

class CapybooApp extends StatelessWidget {
  const CapybooApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MultiProvider(
      providers: [ChangeNotifierProvider(create: (_) => BleService())],
      child: MaterialApp(
        title: 'Capyboo',
        debugShowCheckedModeBanner: false,
        theme: ThemeData(
          useMaterial3: true,
          brightness: Brightness.dark,
          scaffoldBackgroundColor: const Color(0xFF1A1A2E),
          colorScheme: const ColorScheme.dark(
            primary: Color(0xFF00D9FF),
            secondary: Color(0xFFE94560),
            surface: Color(0xFF16213E),
            onPrimary: Color(0xFF1A1A2E),
            onSecondary: Colors.white,
            onSurface: Color(0xFFF5E6D3),
          ),
          appBarTheme: const AppBarTheme(
            backgroundColor: Colors.transparent,
            elevation: 0,
            centerTitle: true,
            titleTextStyle: TextStyle(
              fontFamily: 'Georgia',
              fontSize: 22,
              fontWeight: FontWeight.bold,
              color: Color(0xFFF5E6D3),
              letterSpacing: 1.0,
            ),
          ),
          elevatedButtonTheme: ElevatedButtonThemeData(
            style: ElevatedButton.styleFrom(
              elevation: 8,
              shape: RoundedRectangleBorder(
                borderRadius: BorderRadius.circular(16),
              ),
              padding: const EdgeInsets.symmetric(horizontal: 24, vertical: 16),
            ),
          ),
          inputDecorationTheme: InputDecorationTheme(
            filled: true,
            fillColor: const Color(0xFF16213E),
            border: OutlineInputBorder(
              borderRadius: BorderRadius.circular(16),
              borderSide: BorderSide.none,
            ),
            focusedBorder: OutlineInputBorder(
              borderRadius: BorderRadius.circular(16),
              borderSide: const BorderSide(color: Color(0xFF00D9FF), width: 2),
            ),
            labelStyle: const TextStyle(color: Color(0xFFF5E6D3)),
          ),
          snackBarTheme: SnackBarThemeData(
            backgroundColor: const Color(0xFF16213E),
            contentTextStyle: const TextStyle(color: Color(0xFFF5E6D3)),
            behavior: SnackBarBehavior.floating,
            shape: RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(12),
            ),
          ),
        ),
        home: const HomeScreen(),
      ),
    );
  }
}
