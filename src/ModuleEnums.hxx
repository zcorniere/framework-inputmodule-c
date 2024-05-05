#pragma once

#include <cstdint>

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
enum class PaternType : uint8_t {
    Percentage = 0x00,                /// Percentage pattern (needs a extra parameter)
    Gradient = 0x01,                  /// Gradient pattern (Brightness from top to bottom)
    DoubleGradient = 0x02,            /// Double gradient pattern (Brightness from the middle to top and bottom)
    DisplayLotusHorizontal = 0x03,    /// Display "LOTUS" 90 degrees rotated
    ZigZag = 0x04,                    /// Display a zigzag pattern
    FullBrightness = 0x05,            /// Turn every LED on and display full brightness
    DisplayPanic = 0x06,              /// Display the string "PANIC"
    DisplayLotusVertical = 0x07,      /// Display the string "LOTUS"
};

}    // namespace framework
