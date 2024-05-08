#pragma once

#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
    #define INPUTMODULE_PLATFORM_WINDOWS
#elif defined(__unix__) || defined(__unix)
    #define INPUTMODULE_PLATFORM_LINUX
#endif

#ifndef INPUTMODULE_ASSERT
    #include <assert.h>
    #define INPUTMODULE_ASSERT(x) assert(x)
#endif    // INPUTMODULE_ASSERT

namespace framework
{

enum class InputModuleType {
    LEDMatrix,          /// LED Matrix
    B1Display,          /// B1 Display
    C1MinimalModule,    /// C1 Minimal Module
};

/// @brief Command types that can be sent to the input module
///
/// Comes for the this file on the rust repo
/// Please update the permalink if the file changes
/// https://github.com/FrameworkComputer/inputmodule-rs/blob/13efc56a0bc0b93495195197c64e6d7bf22cd119/commands.md
/// (the file seams outdated)
///
/// - L = LED Matrix
/// - D = B1 Display
/// - M = C1 Minimal Module
enum class CommandType : uint8_t {
    Brightness = 0x00,    /// Control the brightness of the screen (L | M)
    Pattern = 0x01,       /// Control the pattern of the screen (L)
    Bootloader = 0x02,    /// Jump to the bootloader (L | D | M)
    Sleep = 0x03,         /// Put the device to sleep (L | D | M)
    GetSleep = 0x03,      /// Get the sleep status (L | D | M)
    Animate = 0x04,       /// Scroll the pattern on the screen (L)
    GetAnimate = 0x04,    /// Check whether animating (L)
    Panic = 0x05,         /// Panic the device (L | D | M)
    DrawBW = 0x06,        /// Draw a black and white image (L)
    StageCol = 0x07,      /// Send a grayscale column (L)
    FlushCol = 0x08,      /// Flush/draw all column buffer (L)
    // SetText = 0x09,         /// (MARKED AS DEPRECATED IN THE RUST CODE)
    StartGame = 0x10,       /// Start a embedded game (L)
    GameControl = 0x11,     /// Send a game command (L)
    GameStatus = 0x12,      /// Check the game status (WIP) (L)
    SetColor = 0x13,        /// Set the color of the screen (M)
    DisplayOn = 0x14,       /// Turn on the display (D)
    InvertScreen = 0x15,    /// Invert the screen (D)
    SetPxColor = 0x16,      /// Set the color of a pixel (D)
    FlushFB = 0x17,         /// Flush the framebuffer (D)
    Version = 0x20,         /// Get the version of the device (L | D)

    // The following commands were found in the firmware but are not documented
    GetBrightness = Brightness,    /// Get the brightness of the screen (L | D | M)
};

/// @brief Pattern types that can be sent to the input module
///
/// This is the value that must be send as a second parameter to the Pattern command
enum class PatternType : uint8_t {
    Percentage = 0x00,                /// Percentage pattern (needs a extra parameter)
    Gradient = 0x01,                  /// Gradient pattern (Brightness from top to bottom)
    DoubleGradient = 0x02,            /// Double gradient pattern (Brightness from the middle to top and bottom)
    DisplayLotusHorizontal = 0x03,    /// Display "LOTUS" 90 degrees rotated
    ZigZag = 0x04,                    /// Display a zigzag pattern
    FullBrightness = 0x05,            /// Turn every LED on and display full brightness
    DisplayPanic = 0x06,              /// Display the string "PANIC"
    DisplayLotusVertical = 0x07,      /// Display the string "LOTUS"
};

namespace Commands
{

    /// @brief Header of a command to an input module
    struct [[gnu::packed]] PayloadHeader {
        const uint8_t Magic1 = 0x32;
        const uint8_t Magic2 = 0xAC;
        CommandType Type;
        /// Extra parameter for the command need to be sent separately
    };

    /// @brief Payload for the Brightness command
    ///
    /// Change the brightness of the input module
    struct [[gnu::packed]] Brightness {
        const PayloadHeader Header = {
            .Type = CommandType::Brightness,
        };
        uint8_t Brightness = 0;
    };

    /// @brief Payload for the Pattern command
    ///
    /// Display a pattern on the input module, the pattern is defined by the PatternType enum, and the extra parameter
    /// when needed
    struct [[gnu::packed]] Pattern {
        const PayloadHeader Header = {
            .Type = CommandType::Pattern,
        };
        PatternType Pattern = PatternType::Percentage;
        uint8_t Extra = 0;
    };

    /// @brief Payload for the Bootloader command
    ///
    /// Jump to the bootloader of the input module
    struct [[gnu::packed]] Bootloader {
        const PayloadHeader Header = {
            .Type = CommandType::Bootloader,
        };
    };

    /// @brief Payload for the Sleep command
    ///
    /// Put the input module to sleep or wake it up
    struct [[gnu::packed]] Sleep {
        /// @brief Reply for the GetSleep command
        struct [[gnu::packed]] Reply {
            bool Sleep = false;
        };

        const PayloadHeader Header = {
            .Type = CommandType::Sleep,
        };
        bool Sleep = false;
    };

    /// @brief Payload for the GetSleep command
    ///
    /// Get the sleep status of the input module
    struct [[gnu::packed]] GetSleep {
        /// @brief Reply for the GetSleep command
        struct [[gnu::packed]] Reply {
            bool Sleep = false;
        };

        const PayloadHeader Header = {
            .Type = CommandType::GetSleep,
        };
    };

    /// @brief Payload for the Animate command
    ///
    /// Start or stop the animation of the input module
    struct [[gnu::packed]] Animate {
        const PayloadHeader Header = {
            .Type = CommandType::Animate,
        };
        bool Animate = false;
    };

    /// @brief Payload for the GetAnimate command
    ///
    /// Get the animation status of the input module
    struct [[gnu::packed]] GetAnimate {
        /// @brief Reply for the GetAnimate command
        struct [[gnu::packed]] Reply {
            bool Animate = false;
        };

        const PayloadHeader Header = {
            .Type = CommandType::GetAnimate,
        };
    };

    /// @brief Payload for the Panic command
    ///
    /// Panic the input module
    struct [[gnu::packed]] Panic {
        const PayloadHeader Header = {
            .Type = CommandType::Panic,
        };
    };

    /// @brief Payload for the DrawBW command
    ///
    /// Draw a black and white image on the input module
    struct [[gnu::packed]] DrawBW {
        const PayloadHeader Header = {
            .Type = CommandType::DrawBW,
        };

        // 34x9 display packed 1 bit per pixel
        uint8_t Data[39] = {0};
    };
    static_assert(sizeof(DrawBW) == 39 + sizeof(PayloadHeader));

    /// @brief Payload for the StageCol command
    ///
    /// Send a grayscale column to the input module
    struct [[gnu::packed]] StageCol {
        const PayloadHeader Header = {
            .Type = CommandType::StageCol,
        };

        uint8_t Col = 0;
        uint8_t Data[34] = {0};
    };

    /// @brief Payload for the FlushCol command
    ///
    /// Flush the column buffer of the input module
    struct [[gnu::packed]] FlushCol {
        const PayloadHeader Header = {
            .Type = CommandType::FlushCol,
        };
    };

    /// @brief Payload for the StartGame command
    ///
    /// Start a embedded game on the input module
    struct [[gnu::packed]] StartGame {
        const PayloadHeader Header = {
            .Type = CommandType::StartGame,
        };
        uint8_t GameID = 0;
    };

    /// @brief Payload for the GameControl command
    ///
    /// Send a game command to the input module
    struct [[gnu::packed]] GameControl {
        const PayloadHeader Header = {
            .Type = CommandType::GameControl,
        };
        uint8_t Control = 0;
    };

    /// @brief Payload for the GameStatus command
    ///
    /// Get the game status of the input module
    struct [[gnu::packed]] GameStatus {
        /// @brief Reply for the GameStatus command
        // The firmware currently reply nothing for now
        // struct [[gnu::packed]] Reply {
        // };

        const PayloadHeader Header = {
            .Type = CommandType::GameStatus,
        };
    };

    /// @brief Payload for the SetColor command
    ///
    /// Set the color of the input module
    struct [[gnu::packed]] SetColor {
        const PayloadHeader Header = {
            .Type = CommandType::SetColor,
        };
        uint8_t R = 0;
        uint8_t G = 0;
        uint8_t B = 0;
    };

    /// @brief Payload for the DisplayOn command
    ///
    /// Turn on the display of the input module
    struct [[gnu::packed]] DisplayOn {
        const PayloadHeader Header = {
            .Type = CommandType::DisplayOn,
        };
        bool bDisplay = true;
    };

    /// @brief Payload for the InvertScreen command
    ///
    /// Invert the screen of the input module
    struct [[gnu::packed]] InvertScreen {
        const PayloadHeader Header = {
            .Type = CommandType::InvertScreen,
        };
        bool bInvert = true;
    };

    /// @brief Payload for the SetPxColor command
    ///
    /// Set a column of pixel
    struct [[gnu::packed]] SetPxColor {
        const PayloadHeader Header = {
            .Type = CommandType::SetPxColor,
        };
        uint8_t Col = 0;
        uint8_t ColorData[49] = {0};
    };

    /// @brief Payload for the FlushFB command
    ///
    /// Flush the framebuffer of the input module
    struct [[gnu::packed]] FlushFB {
        const PayloadHeader Header = {
            .Type = CommandType::FlushFB,
        };
    };

    /// @brief Payload for the Version command
    ///
    /// Get the version of the input module
    struct [[gnu::packed]] Version {
        /// @brief Reply for the Version command
        struct [[gnu::packed]] Reply {
            int GetMajor() const
            {
                return Major;
            }
            int GetMinor() const
            {
                return (MinorPatch & 0xF0) >> 4;
            }
            int GetPatch() const
            {
                return MinorPatch & 0x0F;
            }
            bool IsPreRelease() const
            {
                return PreRelease;
            }
            std::string ToString() const
            {
                return std::to_string(GetMajor()) + "." + std::to_string(GetMinor()) + "." +
                       std::to_string(GetPatch()) + (PreRelease ? "-pre" : "");
            }

            uint8_t Major = 0;
            uint8_t MinorPatch = 0;
            bool PreRelease = 0;
        };

        const PayloadHeader Header = {
            .Type = CommandType::Version,
        };
    };

    /// @brief Payload for the GetBrightness command
    ///
    /// Get the brightness of the input module
    struct [[gnu::packed]] GetBrightness {
        /// @brief Reply for the GetBrightness command
        struct [[gnu::packed]] Reply {
            uint8_t Brightness = 0;
        };

        const PayloadHeader Header = {
            .Type = CommandType::GetBrightness,
        };
    };

}    // namespace Commands

template <typename T>
concept TInputModulePayloadType = requires {
    offsetof(T, Header) == 0 && requires(T t) {
        {
            t.Header
        } -> std::same_as<Commands::PayloadHeader>;
    };
};

template <typename T>
concept TInputModulePayloadTypeWithReply = TInputModulePayloadType<T> && requires { typename T::Reply; };

/// Main class representing a framework input module
class IInputModule
{
private:
public:
    explicit IInputModule(InputModuleType Type): Type(Type)
    {
    }

    virtual ~IInputModule() = default;

    /// Is the device valid
    virtual bool IsValid() const = 0;

    template <typename T>
    requires TInputModulePayloadTypeWithReply<T>
    T::Reply WriteToDevice(const T& Data = {})
    {
        const int WrittenData = WriteToDevice_Internal(reinterpret_cast<const uint8_t*>(&Data), sizeof(T));
        if (WrittenData < 0) {
            return typename T::Reply{};
        }

        // The firmware always return 32 bytes (ledmatrix/src/main.rs:481 response
        // will always be a 32 bytes slice)
        std::vector<std::uint8_t> Reply = ReadFromDevice_Internal(32);
        if (Reply.empty()) {
            return typename T::Reply{};
        }

        typename T::Reply ReplyData;
        std::memcpy(&ReplyData, Reply.data(), sizeof(typename T::Reply));
        return ReplyData;
    }

    template <typename T>
    requires TInputModulePayloadType<T>
    int WriteToDevice(const T& Data = {})
    {
        return WriteToDevice_Internal(reinterpret_cast<const uint8_t*>(&Data), sizeof(T));
    }

    template <typename... ArgsType>
    requires(std::is_convertible_v<ArgsType, uint8_t> && ...)
    /// Send a command to the device
    ///
    /// @return >= 0 on success, -1 on error
    /// The return value contain the number of bytes written to the device
    int WriteToDevice(CommandType Type, ArgsType... Args)
    {
        std::vector<uint8_t> PayloadData(sizeof...(Args));

        std::size_t Offset = 0;
        ((PayloadData[Offset++] = uint8_t(Args)), ...);

        return WriteToDevice(Type, PayloadData);
    }

    /// Send a command to the device
    int WriteToDevice(CommandType Type, const std::vector<uint8_t>& Data)
    {
        INPUTMODULE_ASSERT(IsValid());
        const Commands::PayloadHeader Payload{
            .Type = Type,
        };
        // Build the buffer to send to the device
        // The buffer is initialized to the be correct size
        std::vector<uint8_t> PayloadData(sizeof(Payload) + Data.size());
        std::memcpy(PayloadData.data(), &Payload, sizeof(Payload));
        /// And data are copied into it
        std::memcpy(PayloadData.data() + sizeof(Payload), Data.data(), Data.size());

        return WriteToDevice_Internal(PayloadData.data(), PayloadData.size());
    }

protected:
    virtual int WriteToDevice_Internal(const uint8_t* Data, const int Size) = 0;
    virtual std::vector<uint8_t> ReadFromDevice_Internal(const int ExpectedSize) = 0;

private:
    const InputModuleType Type;
};

class IInputModuleManager
{
public:
    /// Get an input module of the requested type
    ///
    /// @param Type The type of the input module
    /// @param Index The index of the input module of the requested type, like if there are multiple LED Matrix
    /// @return A pointer to the input module, or nullptr if the input module is not available. The pointer is owned
    /// by the Manager and should not be deleted
    virtual IInputModule* GetInputModule(InputModuleType Type, int Index = 0) = 0;

    /// Check if an input module of the requested type is available
    ///
    /// @param Type The type of the input module
    /// @return The number of input module is available 0 if none, -1 if there were an error type is not supported
    virtual int IsTypeOfInputModuleAvailable(InputModuleType Type) const = 0;
};
}    // namespace framework

////////////////////////////////////////
// Platform Implementation
////////////////////////////////////////
#if defined(INPUTMODULE_PLATFORM_LINUX)

    #include <libudev.h>
    #include <sys/ioctl.h>
    #include <termios.h>

namespace framework
{

class InputModuleLinux final : public IInputModule
{
public:
    InputModuleLinux(InputModuleType Type, const std::string& FileDevicePath)
        : IInputModule(Type), DeviceFD(open(FileDevicePath.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK))
    {
        if (DeviceFD < 0) {
            perror("open");
            return;
        }

        // Exclusive access to the device
        if (ioctl(DeviceFD, TIOCEXCL)) {
            perror("ioctl(TIOCEXCL)");
            return;
        }

        // Remove the Non-Blocking flag
        if (fcntl(DeviceFD, F_SETFL, 0)) {
            perror("fcntl");
            return;
        }

        struct termios TTYSettings;
        if (tcgetattr(DeviceFD, &TTYSettings)) {
            perror("tcgetattr");
            return;
        }

        // Set the terminal settings to be binary
        TTYSettings.c_cflag |= CREAD | CLOCAL;
        cfmakeraw(&TTYSettings);

        if (tcsetattr(DeviceFD, TCSANOW, &TTYSettings)) {
            perror("tcsetattr");
            return;
        }
        if (tcflush(DeviceFD, TCIOFLUSH)) {
            perror("tcflush");
            return;
        }

        // Parity settings : None
        TTYSettings.c_cflag &= !(PARENB | PARODD);
        TTYSettings.c_iflag &= !INPCK;
        TTYSettings.c_iflag |= IGNPAR;

        // Flow control settings : None
        TTYSettings.c_iflag &= !(IXON | IXOFF);
        TTYSettings.c_cflag &= !CRTSCTS;

        // Set data bit: 8
        TTYSettings.c_cflag &= !CSIZE;
        TTYSettings.c_cflag |= CS8;

        // Set stop bit: 1
        TTYSettings.c_cflag &= !CSTOPB;
        cfsetspeed(&TTYSettings, B115200);
        tcsetattr(DeviceFD, TCSANOW, &TTYSettings);
    }
    virtual ~InputModuleLinux()
    {
        if (DeviceFD > 0) {
            if (ioctl(DeviceFD, TIOCNXCL)) {
                perror("ioctl(TIOCNXCL)");
            }
            close(DeviceFD);
        }
    }

    virtual bool IsValid() const override
    {
        return DeviceFD > 0;
    }

protected:
    virtual int WriteToDevice_Internal(const uint8_t* Data, const int Size) override
    {
        return write(DeviceFD, Data, Size);
    }

    virtual std::vector<uint8_t> ReadFromDevice_Internal(const int ExpectedSize) override
    {
        std::vector<uint8_t> Buffer(ExpectedSize);
        read(DeviceFD, Buffer.data(), ExpectedSize);
        return Buffer;
    }

private:
    const int DeviceFD = 0;
};

class InputModuleManagerLinux final : public IInputModuleManager
{
public:
    InputModuleManagerLinux()
    {
        // #TODO smart pointer with custom deleter
        struct udev* UdevContext = udev_new();
        INPUTMODULE_ASSERT(UdevContext != nullptr);

        // We list every tty udev device
        struct udev_enumerate* enumerate = udev_enumerate_new(UdevContext);
        udev_enumerate_add_match_subsystem(enumerate, "tty");
        udev_enumerate_scan_devices(enumerate);

        /// Iterate over the list of devices
        struct udev_list_entry* list = nullptr;
        struct udev_list_entry* node = nullptr;
        list = udev_enumerate_get_list_entry(enumerate);
        udev_list_entry_foreach(node, list)
        {
            const std::string_view SysPath = udev_list_entry_get_name(node);
            struct udev_device* const dev = udev_device_new_from_syspath(UdevContext, SysPath.data());

            const char* const SerialShortPtr = udev_device_get_property_value(dev, "ID_SERIAL_SHORT");
            if (SerialShortPtr == nullptr) {
                udev_device_unref(dev);
                continue;
            }
            const std::string_view SerialShort(SerialShortPtr);
            if (!SerialShort.starts_with("FRA")) {
                udev_device_unref(dev);
                continue;    // Not a Framework device
            }

            // #HACK
            // I don't have another device to test with and the firmware seem to have the wrong serial number
            // for the B1 display
            // Framework device serial number format
            //                            FRA                - Framwork
            //                               KDE             - C1 LED Matrix
            //                                  BZ           - BizLink
            //                                    01         - SKU, Default Configuration
            //                                      00000000 - Device Identifier
            if (!SerialShort.starts_with("FRAKDEBZ")) {
                udev_device_unref(dev);
                continue;    // Not a Framework device
            }

            const char* const DevName = udev_device_get_property_value(dev, "DEVNAME");
            if (DevName == nullptr) {
                udev_device_unref(dev);
                continue;
            }
            const std::string_view DevPath(DevName);
            if (DevPath.empty()) {
                udev_device_unref(dev);
                continue;
            }

            std::unique_ptr<InputModuleLinux> InputModule =
                std::make_unique<InputModuleLinux>(InputModuleType::LEDMatrix, std::string(DevPath));
            InputModulesMap[InputModuleType::LEDMatrix].emplace_back(std::move(InputModule));
            udev_device_unref(dev);
        }
        udev_enumerate_unref(enumerate);
        udev_unref(UdevContext);
    }

    virtual IInputModule* GetInputModule(InputModuleType Type, int Index = 0) override
    {
        const auto Iter = InputModulesMap.find(Type);
        if (Iter == InputModulesMap.end()) {
            return nullptr;
        }

        if (Index >= Iter->second.size()) {
            return nullptr;
        }
        return Iter->second[Index].get();
    }

    virtual int IsTypeOfInputModuleAvailable(InputModuleType Type) const override
    {
        const auto Iter = InputModulesMap.find(Type);
        if (Iter == InputModulesMap.end()) {
            return 0;
        }
        return Iter->second.size();
    }

private:
    using InputModuleList = std::vector<std::unique_ptr<IInputModule>>;
    std::unordered_map<InputModuleType, InputModuleList> InputModulesMap;
};

using InputModuleManager = InputModuleManagerLinux;

}    // namespace framework

#elif defined(INPUTMODULE_PLATFORM_WINDOWS)

namespace framework
{

class InputModuleManagerWindows final : public IInputModuleManager
{
public:
    InputModuleManagerWindows()
    {
    }

    virtual IInputModule* GetInputModule(InputModuleType Type, int Index = 0) override
    {
        assert(false && "Not implemented");
        return nullptr;
    }

    virtual int IsTypeOfInputModuleAvailable(InputModuleType Type) const override
    {
        assert(false && "Not implemented");
        return 0;
    }
};

using InputModuleManager = InputModuleManagerWindows;

}    // namespace framework

#endif    // INPUTMODULE_PLATFORM_WINDOWS
