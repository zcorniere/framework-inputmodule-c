#pragma once

#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <unistd.h>
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
///
/// - L = LED Matrix
/// - D = B1 Display
/// - M = C1 Minimal Module
enum class CommandType : uint8_t {
    Brightness = 0x00,      /// Control the brightness of the screen (L | M)
    Pattern = 0x01,         /// Control the pattern of the screen (L)
    Bootloader = 0x02,      /// Jump to the bootloader (L | D | M)
    Sleep = 0x03,           /// Put the device to sleep (L | D | M)
    GetSleep = 0x03,        /// Get the sleep status (L | D | M)
    Animate = 0x04,         /// Scroll the pattern on the screen (L)
    GetAnimate = 0x04,      /// Check whether animating (L)
    Panic = 0x05,           /// Panic the device (L | D | M)
    DrawBW = 0x06,          /// Draw a black and white image (L)
    StageCol = 0x07,        /// Send a grayscale column (L)
    FlushCol = 0x08,        /// Flush/draw all column buffer (L)
    SetText = 0x09,         /// (MARKED AS DEPRECATED IN THE RUST CODE)
    StartGame = 0x10,       /// Start a embedded game (L)
    GameControl = 0x11,     /// Send a game command (L)
    GameStatus = 0x12,      /// Check the game status (WIP) (L)
    SetColor = 0x13,        /// Set the color of the screen (M)
    DisplayOn = 0x14,       /// Turn on the display (D)
    InvertScreen = 0x15,    /// Invert the screen (D)
    SetPxColor = 0x16,      /// Set the color of a pixel (D)
    FlushFB = 0x17,         /// Flush the framebuffer (D)
    Version = 0x20,         /// Get the version of the device (D )
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

    /// @brief Payload for the Pattern command
    ///
    /// Display a pattern on the input module, the pattern is defined by the PatternType enum, and the extra parameter
    /// when needed
    struct [[gnu::packed]] Payload_Pattern {
        Payload_Pattern(PatternType Pattern, uint8_t Extra = 0)
            : Header{.Type = CommandType::Pattern}, Pattern(Pattern), Extra(Extra)
        {
        }

        const PayloadHeader Header;
        PatternType Pattern;
        uint8_t Extra;
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

/// Main class representing a framework input module
class IInputModule
{
private:
public:
    explicit IInputModule(InputModuleType Type): Type(Type)
    {
    }

    /// Is the device valid
    virtual bool IsValid() const = 0;

    template <TInputModulePayloadType T>
    int WriteToDevice(const T& Data)
    {
        return WriteToDevice_Internal(reinterpret_cast<const uint8_t*>(&Data), sizeof(T));
    }

    template <typename... ArgsType>
    requires(std::is_convertible_v<ArgsType, uint8_t> && ...)
    /// Send a command to the device
    ///
    /// @return >= 0 on success, -1 on error
    /// The return value contain the number of bytes the device will send back as a reply
    int WriteToDevice(CommandType Type, ArgsType... Args)
    {
        std::vector<uint8_t> PayloadData(sizeof...(Args));

        std::size_t Offset = 0;
        ((PayloadData[Offset++] = uint8_t(Args)), ...);

        WriteToDevice(Type, PayloadData);
        return 0;
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

        WriteToDevice_Internal(PayloadData.data(), PayloadData.size());
        return 0;
    }

protected:
    virtual int WriteToDevice_Internal(const uint8_t* Data, int Size) = 0;

private:
    const InputModuleType Type;
};

#if defined(INPUTMODULE_PLATFORM_LINUX)

class InputModuleLinux final : public IInputModule
{
public:
    InputModuleLinux(InputModuleType Type, const std::string& FileDevicePath)
        : IInputModule(Type), DeviceFD(open(FileDevicePath.c_str(), O_RDWR | O_NOCTTY))
    {
    }
    ~InputModuleLinux()
    {
        if (DeviceFD > 0) {
            close(DeviceFD);
        }
    }

    virtual bool IsValid() const override
    {
        return DeviceFD > 0;
    }

protected:
    virtual int WriteToDevice_Internal(const uint8_t* Data, int Size) override
    {
        return write(DeviceFD, Data, Size);
    }

private:
    const int DeviceFD = 0;
};

using InputModule = InputModuleLinux;

#elif defined(INPUTMODULE_PLATFORM_WINDOWS)

#endif    // INPUTMODULE_PLATFORM_WINDOWS

}    // namespace framework
