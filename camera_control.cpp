#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <camera/camera.h>
#include <camera/device_discovery.h>
#include <camera/photography_settings.h>

#ifdef _WIN32
#include <io.h>
#define ACCESS_FUNC _access
#define F_OK 0
#else
#include <unistd.h>
#include <time.h>
#define ACCESS_FUNC access
#endif

std::string getCurrentTime() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef WIN32
    if (localtime_s(&tm, &t))
#else
    if (localtime_r(&t, &tm))
#endif
    {
    }
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &tm);
    return std::string(buffer);
}

bool fileExists(const std::string& file_path) {
    return ACCESS_FUNC(file_path.c_str(), F_OK) == 0;
}

std::string getFileName(const std::string& path) {
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash == std::string::npos) {
        return path;
    }
    return path.substr(lastSlash + 1);
}

class CameraController {
private:
    std::shared_ptr<ins_camera::Camera> camera_;
    bool is_connected_;

public:
    CameraController() : is_connected_(false) {}

    ~CameraController() {
        disconnect();
    }

    bool discoverAndConnect() {
        std::cout << "Discovering Insta360 cameras..." << std::endl;
        
        ins_camera::SetLogLevel(ins_camera::LogLevel::ERR);
        ins_camera::DeviceDiscovery discovery;
        auto device_list = discovery.GetAvailableDevices();
        
        if (device_list.empty()) {
            std::cerr << "Error: No Insta360 camera found." << std::endl;
            std::cerr << "Please ensure:" << std::endl;
            std::cerr << "  1. Camera is powered on" << std::endl;
            std::cerr << "  2. Camera is connected via USB or WiFi" << std::endl;
            return false;
        }

        std::cout << "Found " << device_list.size() << " camera(s):" << std::endl;
        for (size_t i = 0; i < device_list.size(); i++) {
            const auto& device = device_list[i];
            std::cout << "  [" << i << "] " << device.camera_name 
                      << " (SN: " << device.serial_number 
                      << ", FW: " << device.fw_version << ")" << std::endl;
        }

        // use the first available camera
        const auto& selected_device = device_list[0];
        std::cout << "\nConnecting to: " << selected_device.camera_name 
                  << " (SN: " << selected_device.serial_number << ")..." << std::endl;

        camera_ = std::make_shared<ins_camera::Camera>(selected_device.info);
        
        if (!camera_->Open()) {
            std::cerr << "Error: Failed to open camera connection." << std::endl;
            discovery.FreeDeviceDescriptors(device_list);
            return false;
        }

        // sync time to camera
        time_t now = time(nullptr);
        std::tm tm{};
#ifdef WIN32
        localtime_s(&tm, &now);
        time_t time_seconds = _mkgmtime(&tm);
#else
        localtime_r(&now, &tm);
        time_t time_seconds = timegm(&tm);
#endif
        camera_->SyncLocalTimeToCamera(time_seconds);

        is_connected_ = true;
        std::cout << "Successfully connected to camera!" << std::endl;
        
        discovery.FreeDeviceDescriptors(device_list);
        return true;
    }

    void disconnect() {
        if (camera_ && is_connected_) {
            camera_->Close();
            is_connected_ = false;
            std::cout << "Disconnected from camera." << std::endl;
        }
    }

    bool takePhoto(const std::string& save_directory = "./") {
        if (!is_connected_ || !camera_) {
            std::cerr << "Error: Camera not connected." << std::endl;
            return false;
        }

        // check if camera is still connected
        if (!camera_->IsConnected()) {
            std::cerr << "Error: Camera connection lost." << std::endl;
            is_connected_ = false;
            return false;
        }

        std::cout << "Setting photo mode..." << std::endl;
        bool ret = camera_->SetPhotoSubMode(ins_camera::SubPhotoMode::PHOTO_SINGLE);
        if (!ret) {
            std::cerr << "Warning: Failed to set photo mode, continuing anyway..." << std::endl;
        }

        std::cout << "Taking photo..." << std::endl;
        const auto url = camera_->TakePhoto();
        
        if (url.Empty() || !url.IsSingleOrigin()) {
            std::cerr << "Error: Failed to take photo." << std::endl;
            return false;
        }

        const std::string photo_url = url.GetSingleOrigin();
        std::cout << "Photo captured! URL: " << photo_url << std::endl;

        // download the photo if save directory is provided
        // if (!save_directory.empty()) {
        //     std::string save_path = save_directory;
        //     if (save_path.back() != '/' && save_path.back() != '\\') {
        //         save_path += "/";
        //     }
            
        //     // check if directory exists
        //     if (!fileExists(save_path)) {
        //         std::cerr << "Warning: Save directory does not exist: " << save_path << std::endl;
        //         std::cerr << "Photo URL saved on camera: " << photo_url << std::endl;
        //         return true;
        //     }

        //     std::string file_name = getFileName(photo_url);
        //     if (file_name.empty()) {
        //         file_name = "photo_" + getCurrentTime() + ".jpg";
        //     }
            
        //     std::string full_path = save_path + file_name;
        //     std::cout << "Downloading photo to: " << full_path << std::endl;

        //     int64_t last_progress = -1;
        //     bool download_success = camera_->DownloadCameraFile(photo_url, full_path,
        //         [&](int64_t current, int64_t total_size) {
        //             int64_t progress = total_size > 0 ? (current * 100 / total_size) : 0;
        //             if (progress != last_progress) {
        //                 std::cout << "\rDownload progress: " << progress << "%" << std::flush;
        //                 last_progress = progress;
        //             }
        //         });
            
        //     std::cout << std::endl;
            
        //     if (download_success) {
        //         std::cout << "Photo successfully downloaded to: " << full_path << std::endl;
        //         return true;
        //     } else {
        //         std::cerr << "Error: Failed to download photo." << std::endl;
        //         std::cerr << "Photo URL on camera: " << photo_url << std::endl;
        //         return false;
        //     }
        // }

        return true;
    }

    bool shutdownCamera() {
        if (!is_connected_ || !camera_) {
            std::cerr << "Error: Camera not connected." << std::endl;
            return false;
        }

        std::cout << "Shutting down camera..." << std::endl;
        bool ret = camera_->ShutdownCamera();
        
        if (ret) {
            std::cout << "Camera shutdown command sent successfully." << std::endl;
            is_connected_ = false;
            return true;
        } else {
            std::cerr << "Error: Failed to shutdown camera." << std::endl;
            return false;
        }
    }

    bool getBatteryStatus() {
        if (!is_connected_ || !camera_) {
            std::cerr << "Error: Camera not connected." << std::endl;
            return false;
        }

        ins_camera::BatteryStatus status{};
        bool ret = camera_->GetBatteryStatus(status);
        
        if (!ret) {
            std::cerr << "Error: Failed to get battery status." << std::endl;
            return false;
        }

        std::cout << "Battery Status:" << std::endl;
        std::cout << "  Power Type: " << (status.power_type == ins_camera::PowerType::BATTERY ? "Battery" : "Adapter") << std::endl;
        std::cout << "  Battery Level: " << status.battery_level << "%" << std::endl;
        std::cout << "  Battery Scale: " << status.battery_scale << std::endl;
        
        return true;
    }

    bool getStorageStatus() {
        if (!is_connected_ || !camera_) {
            std::cerr << "Error: Camera not connected." << std::endl;
            return false;
        }

        ins_camera::StorageStatus status{};
        bool ret = camera_->GetStorageState(status);
        
        if (!ret) {
            std::cerr << "Error: Failed to get storage status." << std::endl;
            return false;
        }

        // convert bytes to GB or MB
        auto formatBytes = [](uint64_t bytes) -> std::string {
            const uint64_t GB = 1024ULL * 1024ULL * 1024ULL;
            const uint64_t MB = 1024ULL * 1024ULL;
            
            if (bytes >= GB) {
                double gb = static_cast<double>(bytes) / GB;
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "%.2f GB", gb);
                return std::string(buffer);
            } else if (bytes >= MB) {
                double mb = static_cast<double>(bytes) / MB;
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "%.2f MB", mb);
                return std::string(buffer);
            } else {
                return std::to_string(bytes) + " bytes";
            }
        };

        // convert CardState enum to readable text
        std::string state_text;
        switch (status.state) {
            case ins_camera::STOR_CS_PASS:
                state_text = "OK";
                break;
            case ins_camera::STOR_CS_NOCARD:
                state_text = "No Card";
                break;
            case ins_camera::STOR_CS_NOSPACE:
                state_text = "No Space";
                break;
            case ins_camera::STOR_CS_INVALID_FORMAT:
                state_text = "Invalid Format";
                break;
            case ins_camera::STOR_CS_WPCARD:
                state_text = "Write Protected";
                break;
            case ins_camera::STOR_CS_OTHER_ERROR:
                state_text = "Other Error";
                break;
            default:
                state_text = "Unknown";
                break;
        }

        uint64_t used_space = status.total_space - status.free_space;
        double used_percentage = status.total_space > 0 
            ? (static_cast<double>(used_space) / status.total_space) * 100.0 
            : 0.0;

        std::cout << "Storage Status:" << std::endl;
        std::cout << "  State: " << state_text << std::endl;
        std::cout << "  Total Space: " << formatBytes(status.total_space) << std::endl;
        std::cout << "  Free Space: " << formatBytes(status.free_space) << std::endl;
        std::cout << "  Used Space: " << formatBytes(used_space) << " (" 
                  << std::fixed << std::setprecision(1) << used_percentage << "%)" << std::endl;
        
        return true;
    }

    bool setVideoMode() {
        if (!is_connected_ || !camera_) {
            std::cerr << "Error: Camera not connected." << std::endl;
            return false;
        }

        std::cout << "Setting video mode..." << std::endl;
        bool ret = camera_->SetVideoSubMode(ins_camera::SubVideoMode::VIDEO_NORMAL);
        
        if (!ret) {
            std::cerr << "Error: Failed to set video mode." << std::endl;
            return false;
        }

        std::cout << "Video mode set successfully." << std::endl;
        return true;
    }

    bool startRecording() {
        if (!is_connected_ || !camera_) {
            std::cerr << "Error: Camera not connected." << std::endl;
            return false;
        }

        // check if camera is still connected
        if (!camera_->IsConnected()) {
            std::cerr << "Error: Camera connection lost." << std::endl;
            is_connected_ = false;
            return false;
        }

        // set video mode first
        std::cout << "Setting video mode..." << std::endl;
        bool ret = camera_->SetVideoSubMode(ins_camera::SubVideoMode::VIDEO_NORMAL);
        if (!ret) {
            std::cerr << "Warning: Failed to set video mode, continuing anyway..." << std::endl;
        }

        // set video capture parameters (not sure yet exactly what to set the parameters to)
        ins_camera::RecordParams record_params;
        record_params.resolution = ins_camera::VideoResolution::RES_3840_3840P30;
        record_params.bitrate = 1024 * 1024 * 10; // 10 Mbps default
        ret = camera_->SetVideoCaptureParams(record_params, ins_camera::CameraFunctionMode::FUNCTION_MODE_NORMAL_VIDEO);
        if (!ret) {
            std::cerr << "Warning: Failed to set video capture params, continuing anyway..." << std::endl;
        }

        std::cout << "Starting recording..." << std::endl;
        ret = camera_->StartRecording();
        
        if (!ret) {
            std::cerr << "Error: Failed to start recording." << std::endl;
            return false;
        }

        std::cout << "Recording started successfully!" << std::endl;
        return true;
    }

    bool stopRecording() {
        if (!is_connected_ || !camera_) {
            std::cerr << "Error: Camera not connected." << std::endl;
            return false;
        }

        std::cout << "Stopping recording..." << std::endl;
        const auto url = camera_->StopRecording();
        
        if (url.Empty()) {
            std::cerr << "Error: Failed to stop recording or no video was recorded." << std::endl;
            return false;
        }

        std::cout << "Recording stopped successfully!" << std::endl;
        
        // display video URL(s)
        if (url.IsSingleOrigin()) {
            std::cout << "Video URL: " << url.GetSingleOrigin() << std::endl;
        } else {
            const auto& origins = url.OriginUrls();
            std::cout << "Video URLs (" << origins.size() << "):" << std::endl;
            for (size_t i = 0; i < origins.size(); i++) {
                std::cout << "  [" << i << "] " << origins[i] << std::endl;
            }
        }
        
        return true;
    }

    bool isConnected() const {
        return is_connected_ && camera_ && camera_->IsConnected();
    }
};

void printUsage(const char* program_name) {
    std::cout << "Insta360 Camera Control for Raspberry Pi" << std::endl;
    std::cout << "Usage: " << program_name << " <command> [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  connect              - Connect to camera" << std::endl;
    std::cout << "  photo [save_dir]    - Take a photo (optionally save to directory)" << std::endl;
    std::cout << "  shutdown             - Power off the camera" << std::endl;
    std::cout << "  battery              - Get battery status" << std::endl;
    std::cout << "  storage              - Get storage capacity status" << std::endl;
    std::cout << "  video-mode           - Switch camera to video mode" << std::endl;
    std::cout << "  record-start         - Start recording video (keeps connection open)" << std::endl;
    std::cout << "  record-stop          - Stop recording video and display URL(s)" << std::endl;
    std::cout << "  interactive          - Interactive mode" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << program_name << " photo                    # Take photo" << std::endl;
    std::cout << "  " << program_name << " photo ./photos          # Take photo and save to ./photos" << std::endl;
    std::cout << "  " << program_name << " shutdown                # Power off camera" << std::endl;
    std::cout << "  " << program_name << " interactive             # Interactive mode" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string command = argv[1];
    
    CameraController controller;

    if (command == "connect") {
        if (!controller.discoverAndConnect()) {
            return 1;
        }
        std::cout << "Camera connected. Use 'photo', 'shutdown', 'battery', 'storage', or video commands." << std::endl;
        return 0;
    }

    // for other commands, we need to connect first
    if (!controller.discoverAndConnect()) {
        return 1;
    }

    if (command == "photo") {
        std::string save_dir = (argc > 2) ? argv[2] : "./";
        bool success = controller.takePhoto(save_dir);
        controller.disconnect();
        return success ? 0 : 1;
    }
    else if (command == "shutdown") {
        bool success = controller.shutdownCamera();
        controller.disconnect();
        return success ? 0 : 1;
    }
    else if (command == "battery") {
        bool success = controller.getBatteryStatus();
        controller.disconnect();
        return success ? 0 : 1;
    }
    else if (command == "storage") {
        bool success = controller.getStorageStatus();
        controller.disconnect();
        return success ? 0 : 1;
    }
    else if (command == "video-mode") {
        bool success = controller.setVideoMode();
        controller.disconnect();
        return success ? 0 : 1;
    }
    else if (command == "record-start") {
        bool success = controller.startRecording();
        if (success) {
            std::cout << "\nRecording in progress. Keep this process running." << std::endl;
            std::cout << "Use 'record-stop' command in another terminal or Ctrl+C to stop." << std::endl;
            std::cout << "Press Enter to stop recording and exit..." << std::endl;
            std::cin.get();
            controller.stopRecording();
        }
        controller.disconnect();
        return success ? 0 : 1;
    }
    else if (command == "record-stop") {
        bool success = controller.stopRecording();
        controller.disconnect();
        return success ? 0 : 1;
    }
    else if (command == "interactive") {
        std::cout << "\n=== Interactive Mode ===" << std::endl;
        std::cout << "Commands: photo [dir], shutdown, battery, storage, video-mode, record-start, record-stop, quit" << std::endl;
        
        std::string line;
        while (true) {
            std::cout << "\n> ";
            std::getline(std::cin, line);
            
            if (line == "quit" || line == "exit") {
                break;
            }
            else if (line == "photo") {
                controller.takePhoto("./");
            }
            else if (line.substr(0, 5) == "photo") {
                std::string dir = line.length() > 6 ? line.substr(6) : "./";
                controller.takePhoto(dir);
            }
            else if (line == "shutdown") {
                if (controller.shutdownCamera()) {
                    break;
                }
            }
            else if (line == "battery") {
                controller.getBatteryStatus();
            }
            else if (line == "storage") {
                controller.getStorageStatus();
            }
            else if (line == "video-mode") {
                controller.setVideoMode();
            }
            else if (line == "record-start") {
                controller.startRecording();
            }
            else if (line == "record-stop") {
                controller.stopRecording();
            }
            else if (line.empty()) {
                continue;
            }
            else {
                std::cout << "Unknown command. Try: photo, shutdown, battery, storage, video-mode, record-start, record-stop, quit" << std::endl;
            }
            
            // check if still connected
            if (!controller.isConnected()) {
                std::cout << "Camera disconnected. Exiting..." << std::endl;
                break;
            }
        }
        
        controller.disconnect();
        return 0;
    }
    else {
        std::cerr << "Unknown command: " << command << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    return 0;
}

