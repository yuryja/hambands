# HamBands - Solar Conditions for Pebble

**HamBands** is an elegant and efficient Pebble watch app designed for HAM Radio operators. It provides real-time solar conditions and DX propagation metrics directly on your wrist, fetching data from [hamqsl.com](https://www.hamqsl.com/solarxml.php).

![Author](https://img.shields.io/badge/Author-YY3BIG-blue)
![Platform](https://img.shields.io/badge/Platform-Pebble-orange)
![License](https://img.shields.io/badge/License-MIT-green)

## 🌟 Features

- **Real-time Solar Metrics**: Displays current SFI (Solar Flux Index) and K-Index.
- **Propagation Table**: Comprehensive view of band conditions (80m to 10m) for both **Day** and **Night**.
- **Multi-language Support**: Automatically detects or allows manual selection of:
  - English 🇺🇸
  - Español 🇪🇸
  - Français 🇫🇷
  - Italiano 🇮🇹
  - Português 🇵🇹
- **On-Device Settings**: Built-in configuration page (Pebble-Clay) to override language preferences.
- **Intelligent Formatting**: Adapts date formats based on regional settings (e.g., Month Day for US, Day Month for others).
- **Elegant & Readable UI**: High-contrast design optimized for Pebble's e-paper display with perfectly aligned columns.

## 📸 Screenshots

*(Add your screenshots here)*

## 🛠️ Installation

### For Users
1. Download the latest `hambands.pbw` from the [Releases](https://github.com/yuryja/hambands/releases) section.
2. Sideload it to your Pebble using the Pebble App or a Rebble-compatible tool.

### For Developers
If you want to build the project yourself:
1. Ensure you have the **Pebble SDK** installed.
2. Clone the repository:
   ```bash
   git clone https://github.com/yuryja/hambands.git
   cd hambands
   ```
3. Install dependencies:
   ```bash
   pebble package install pebble-clay
   ```
4. Build the project:
   ```bash
   pebble build
   ```

## ⚙️ Configuration

The app includes a settings page accessible via the Pebble mobile app. You can:
- Keep the **System Default** language.
- Manually select a preferred language for the UI.

## 📡 Data Source

All solar and propagation data is provided by the excellent XML service at [hamqsl.com](https://www.hamqsl.com) by N0NBH.

## ☕ Support / Donation

If you find this app useful and would like to support its development, you can buy me a coffee!

[![Donate](https://img.shields.io/badge/Donate-PayPal-blue.svg)](https://paypal.me/yuryja)

Your support is greatly appreciated!

## 🤝 Contributing

Contributions, issues, and feature requests are welcome! Feel free to check the [issues page](https://github.com/yuryja/hambands/issues).

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---
Developed with ❤️ by **Yury Jajitzky (YY3BIG)**
