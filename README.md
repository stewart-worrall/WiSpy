# WiSpy

WiSpy is an educational WiFi security demonstration tool built for ESP32 microcontrollers. It creates a WiFi access point (hotspot) that demonstrates common network security vulnerabilities, including password interception through fake login pages and DNS hijacking attacks.

## What is WiSpy?

WiSpy is designed as a hands-on educational tool to teach network security concepts. When deployed on an ESP32, it:

- Creates a WiFi access point that devices can connect to
- Serves fake login pages mimicking popular websites (banks, PayPal, Amazon, Facebook, Google, GitHub, Stack Overflow)
- Demonstrates DNS hijacking by redirecting domain name lookups to the ESP32's web server
- Captures and displays passwords submitted through the fake login forms
- Shows how HTTP (unencrypted) traffic can be easily intercepted and manipulated

** IMPORTANT: This project is for educational purposes only. Use only in controlled environments with proper authorization. Never use this for malicious purposes.**

## How It Works

### WiFi Access Point
The ESP32 creates a WiFi hotspot with a configurable name (default: "University_Guest"). Devices can connect to this network without a password (open WiFi), making it easy to demonstrate security vulnerabilities.

### Web Server
Once connected, devices are automatically assigned an IP address in the 192.168.4.x range, with the ESP32 serving as the gateway at 192.168.4.1. The ESP32 runs an HTTP web server on port 80 that serves:

1. **Main Login Page**: A generic login form that demonstrates password interception
2. **Fake Website Pages**: Realistic-looking login pages for popular websites
3. **DNS Test Page**: Tools to test DNS hijacking functionality
4. **Captured Credentials Page**: Displays all intercepted passwords and usernames

### DNS Hijacking
The ESP32 implements a basic DNS server that intercepts DNS queries for specific domains (mybank.com, paypal.com, amazon.com, etc.) and redirects them to the ESP32's IP address (192.168.4.1). This demonstrates how attackers can redirect users to fake websites even when they type legitimate domain names.

### Password Capture
When users submit credentials through any of the fake login forms, the ESP32:
- Captures the username and password
- Stores the client's IP address
- Displays the captured credentials on a special `/captured` page
- Shows how easily unencrypted HTTP traffic can be intercepted

## Hardware Requirements

- **ESP32C3 Dev Module** (tested on this board)
- USB cable for programming and power
- Computer with Arduino IDE installed

## Software Requirements

- Arduino IDE (version 1.8.x or 2.x)
- ESP32 board support package for Arduino IDE

## Installation & Setup

### 1. Install Arduino IDE

Download and install the Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software).

### 2. Install ESP32 Board Support

1. Open Arduino IDE
2. Go to **File** → **Preferences**
3. In "Additional Board Manager URLs", add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools** → **Board** → **Boards Manager**
5. Search for "ESP32" and install the "esp32" package by Espressif Systems
6. Wait for installation to complete

### 3. Configure Board Settings

1. Connect your ESP32C3 to your computer via USB
2. In Arduino IDE, go to **Tools** → **Board** → **ESP32 Arduino** → **ESP32C3 Dev Module**
3. Configure the following settings:
   - **USB CDC On Boot**: **Enabled** (required for USB serial debugging)
   - **Port**: Select the COM port for your ESP32C3

### 4. Select Serial Port

1. Go to **Tools** → **Port**
2. Select the port corresponding to your ESP32C3 (usually shows as "USB Serial" or similar)

### 5. Upload the Code

1. Open `WiSpy.ino` in Arduino IDE
2. (Optional) Modify the hotspot name by changing the `HOTSPOT_NAME` define at the top of the file
3. Click the **Upload** button (→ arrow icon) or press **Ctrl+U** (Windows/Linux) or **Cmd+U** (Mac)
4. Wait for compilation and upload to complete
5. You should see "Hard resetting via RTS pin..." when upload is successful

### 6. Monitor Serial Output

1. Open the Serial Monitor: **Tools** → **Serial Monitor** or press **Ctrl+Shift+M**
2. Set baud rate to **115200**
3. You should see output showing:
   - Hotspot name and IP address (192.168.4.1)
   - Connected device count
   - DNS hijacking status
   - Captured credentials

## Configuration

### Changing Hotspot Name

Edit the `HOTSPOT_NAME` define at the top of the `.ino` file. Available options:
- 1: "FreeCafeWiFi"
- 2: "Airport_WiFi_Free"
- 3: "Hotel_Guest_Network"
- 4: "Starbucks_WiFi"
- 5: "McDonald's_Free_WiFi"
- 6: "Library_Public_WiFi"
- 7: "CoffeeShop_Guest"
- 8: "Mall_Free_Internet"
- 9: "University_Guest" (default)
- 10: "Restaurant_WiFi"

### Customizing Hijacked Domains

Edit the `hijacked_domains` array in the code to add or remove domains that should be hijacked by the DNS server.

## Usage

1. **Power on the ESP32** with the uploaded code
2. **Connect a device** (phone, laptop, etc.) to the WiFi network created by the ESP32
3. **Open a web browser** and navigate to `http://192.168.4.1` or visit any of the hijacked domains
4. **Submit test credentials** through the fake login forms
5. **View captured credentials** at `http://192.168.4.1/captured`
6. **Test DNS hijacking** at `http://192.168.4.1/test`

## Troubleshooting

### USB Serial Not Working

- Ensure "USB CDC On Boot" is enabled in board settings
- Try a different USB cable (data-capable, not charge-only)
- Check device manager (Windows) or `lsusb` (Linux) to verify the device is recognized
- Install USB-to-Serial drivers if needed (CH340, CP2102, etc.)

### Upload Fails

- Hold the BOOT button while clicking Upload, then release after upload starts
- Try a lower upload speed (115200 or 460800)
- Ensure the correct port is selected
- Try a different USB port or cable
- Check that no other program is using the serial port

### WiFi Not Appearing

- Check Serial Monitor for error messages
- Verify the ESP32 has sufficient power (use a good USB cable/port)
- Try resetting the ESP32
- Check if WiFi is enabled in the code (should be by default)

### DNS Hijacking Not Working

- Ensure devices are connected to the ESP32's WiFi network
- Try manually setting DNS to 192.168.4.1 on the connected device
- Clear browser cache or use incognito mode
- Check Serial Monitor for DNS query logs

## Educational Use Cases

- **Network Security Courses**: Demonstrate man-in-the-middle attacks
- **Cybersecurity Workshops**: Show how easy it is to intercept unencrypted traffic
- **Ethical Hacking Training**: Hands-on experience with common attack vectors
- **Security Awareness**: Visual demonstration of why HTTPS is important

## Security Notes

- This tool uses **HTTP only** (no encryption) to demonstrate vulnerabilities
- All traffic is transmitted in plain text
- Real-world applications should **always use HTTPS**
- This project is intended for **educational purposes only**
- **Never use this on networks you don't own or without explicit permission**

## License

This project is provided as-is for educational purposes.

## Credits

This project was codesigned with [Cursor.ai](https://cursor.ai)

