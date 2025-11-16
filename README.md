# Insta360 Camera Control for Raspberry Pi

This application provides basic control of Insta360 cameras from a Raspberry Pi (any model with a USB port should work), including:
- Taking photos
- Powering the camera on/off (WIP!)
- Checking battery status
- Interactive mode for multiple commands

## 📖 Getting Started

### Hardware
- Raspberry Pi (I've personally used a Pi Zero 2 W and a Pi 5)
- Insta360 camera (I've used an X4, as far as I understand X3 and X5 should work. Unsure of other models)
- USB cable or WiFi connection to camera (WiFi WIP!)

### Software
- Raspberry Pi OS (or compatible Linux distribution)
- C++ compiler (g++)
- Make

Install dependencies:
```bash
sudo apt-get update
sudo apt-get install build-essential g++ make
```

## Building

1. Extract the SDK if not already done:
```bash
tar -xzf ../CameraSDK-2.0.2-build1-20250418-gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar.gz
```

2. Build the application:
```bash
make
```

3. Set up library path:
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/CameraSDK-20250418_161512-2.0.2-gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/lib
```

## Usage

### Command Line Interface

#### Take a photo
```bash
./camera_control photo
```

#### Take a photo and save to directory
```bash
./camera_control photo ./photos
```

#### Power off the camera
```bash
./camera_control shutdown
```

#### Check battery status
```bash
./camera_control battery
```

#### Interactive mode
```bash
./camera_control interactive
```

In interactive mode, you can run multiple commands:
- `photo [directory]` - Take a photo
- `shutdown` - Power off camera
- `battery` - Check battery status
- `quit` or `exit` - Exit interactive mode

## Camera Connection

### USB Connection
1. Connect camera to Raspberry Pi via USB cable
2. Ensure camera is powered on
3. Run the application

### WiFi Connection
1. Put camera in WiFi mode
2. Connect Raspberry Pi to camera's WiFi network
3. Run the application

The application will automatically discover cameras connected via USB or WiFi.

## Troubleshooting

### "No camera found"
- Ensure camera is powered on
- Check USB/WiFi connectio
- Try running with `sudo`

### "Failed to open camera"
- Camera may be in use by another application
- Try disconnecting and reconnecting
- Check camera battery level

### Library not found
- Ensure `LD_LIBRARY_PATH` is set correctly
- Check that `libCameraSDK.so` exists in the lib directory
- Try installing the library system-wide (see Installation)

## Installation (Optional)

To install system-wide:
```bash
sudo make install
```

This will:
- Copy `camera_control` to `/usr/local/bin`
- Copy `libCameraSDK.so` to `/usr/local/lib`
- Run `ldconfig` to update library cache

After installation, you can run from anywhere:
```bash
camera_control photo
```

## Files

- `camera_control.cpp` - Main application source code
- `Makefile` - Build configuration
- `CameraSDK-*/` - Insta360 Camera SDK (headers, library, examples)

## Notes

- **Power On**: The SDK does not provide a software "power on" function. The camera must be manually powered on or connected via USB/WiFi.
- **Power Off**: The `shutdown` command will (should :/) power off the camera.
- **Photo Download**: Photos are automatically downloaded after capture if a save directory is specified.
- **Connection**: The camera must be powered on and connected (USB or WiFi) before running commands.

## License

This application uses the Insta360 Camera SDK. Please refer to Insta360's SDK license terms.