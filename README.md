# Alunswara DAC

A USB Digital-to-Analog Converter (DDC) based on Raspberry Pi Pico (RP2040) with I2S output, master clock (MCLK), and OLED display.

Supports USB Audio Class 1 (Full-Speed) and Class 2 (High-Speed) with up to **96kHz / 24-bit stereo** Hi-Res audio playback. Real-time audio information is shown on a 128x32 SSD1306 OLED display.

Based on [pico_usb_i2s_speaker](https://github.com/BambooMaster/pico_usb_i2s_speaker) by BambooMaster.

## Features

- **USB Audio**: UAC1 (Full-Speed) + UAC2 (High-Speed) dual-mode
- **Hi-Res Audio**: 44.1 / 48 / 88.2 / 96 kHz, 16-bit and 24-bit stereo
- **I2S Output**: Low-jitter MCLK via PIO (pico-i2s-pio library)
- **OLED Display**: SSD1306 128x32 with boot animation and real-time audio info
- **Asynchronous Feedback**: Adaptive clock sync for glitch-free playback
- **Dual-Core**: Core 0 handles USB + display, Core 1 dedicated to I2S DMA

## OLED Display

The 128x32 pixel SSD1306 OLED shows:

- **Boot animation**: 9-frame "Alunswara" animation at 10fps with fade transition
- **Streaming info**:
  - Line 1: "Hi-Res" or "Standard"
  - Line 2: Sample rate and bit depth (e.g., "96.0kHz / 24-bit")
  - Line 3: "USB PCM"
- **Idle states**: "Ready" (connected) / "No Signal" (disconnected)

## Pin Assignment

### I2S Output (PIO0)

| Signal | GPIO | Board Pin |
|--------|------|-----------|
| DATA | GP18 | Pin 24 |
| LRCLK | GP20 | Pin 26 |
| BCLK | GP21 | Pin 27 |
| MCLK | GP22 | Pin 29 |

### OLED SSD1306 (I2C0, 400kHz, address 0x3C)

| Signal | GPIO | Board Pin |
|--------|------|-----------|
| SDA | GP0 | Pin 1 |
| SCL | GP1 | Pin 2 |
| VCC | 3V3 | Pin 36 |
| GND | GND | Pin 3 |

## Build

### Prerequisites
- [Pico SDK 2.2.0](https://github.com/raspberrypi/pico-sdk) (auto-detected via VS Code extension or environment variable)
- ARM GCC toolchain (arm-none-eabi-gcc)
- CMake 3.13+ and Ninja

### Steps

```bash
git clone <your-fork-url>
cd pico_usb_i2s_speaker
git submodule update --init
cmake -B build -G Ninja
cmake --build build -j4
```

Output: `build/pico_usb_i2s_speaker.uf2`

### Flash to Pico

1. Hold **BOOTSEL** button on the Pico
2. Connect USB while holding BOOTSEL
3. Release BOOTSEL — a drive named `RPI-RP2` appears
4. Copy `pico_usb_i2s_speaker.uf2` to the drive
5. Pico reboots automatically and appears as "Alunswara DAC"

## Interpolation (RP2350 branch)

An oversampling/interpolation feature using RP2350 DSP is available on the [interpolation](https://github.com/BambooMaster/pico_usb_i2s_speaker/tree/interpolation) branch.

- **44.1/48kHz**: 8x oversampling
- **88.2/96kHz**: 4x oversampling
- Filter: 20.5kHz passband, -140dB stopband attenuation

## I2S Modes

The [pico-i2s-pio](https://github.com/BambooMaster/pico-i2s-pio) library supports:
- Standard I2S
- PT8211 (16-bit right-justified)
- AK449X (EXDF)
- Slave mode
- Dual-mono operation

Default: Low-jitter mode, standard I2S.

## Tested On

- Windows 11
- Ubuntu 24.04
- Pixel 6a (Android 16)
- iPad Air Gen4 (iPadOS 26.3.1)

## License

MIT License — see [LICENSE](LICENSE) for details.

Original work by [BambooMaster](https://github.com/BambooMaster). OLED display feature and modifications by Alunswara.
