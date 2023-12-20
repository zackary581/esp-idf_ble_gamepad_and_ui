idf_component_register(
    SRCS
        "main.cpp"
        "../ESP32-BLE-Gamepad/BleConnectionStatus.cpp"
        "../ESP32-BLE-Gamepad/BleGamepad.cpp"
        "../ESP32-BLE-Gamepad/BleGamepadConfiguration.cpp"

    INCLUDE_DIRS
        "."
        "../include"
        "../ESP32-BLE-Gamepad"

    REQUIRES
        arduino
        esp-nimble-cpp  
)
