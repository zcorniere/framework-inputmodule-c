# Framework Inputmodule

## THIS IS NOT AN OFFICIAL FRAMEWORK LAPTOP PROJECT
## [This is the official project ](https://github.com/FrameworkComputer/inputmodule-rs)

This project aims to implement the Framework 16 input module protocol into a cpp library.
This my first project of this nature, so contributions and criticism are welcome

## Usage

**The only supported platform is Linux for the moment, but Windows support is possible / planned.**

This example code will display a progress bar from 0 to 100 and then put the module to animated a zigzig pattern
```cpp
    using namespace framework;

    InputModuleManager Manager;

    IInputModule* const Module = Manager.GetInputModule(InputModuleType::LEDMatrix, 0);
    Module->WriteToDevice(CommandType::Sleep, false);
    Module->WriteToDevice(CommandType::Animate, false);
    Module->WriteToDevice(CommandType::Brightness, 30);

    Commands::Pattern PatternCommand(PatternType::Percentage);
    for (uint8_t i = 0; i <= 100; i++) {
        PatternCommand.Extra = i;
        Module->WriteToDevice(PatternCommand);
        usleep(100000);
    }
    Module->WriteToDevice(CommandType::Pattern, static_cast<uint8_t>(PatternType::ZigZag));
    Module->WriteToDevice(CommandType::Animate, true);
```

