#include "InputModule.hxx"

#include <iostream>
#include <syncstream>
#include <thread>

void SendTestCommand(framework::IInputModule* const Module)
{
    using namespace framework;

    if (!Module) {
        return;
    }
    {
        std::osyncstream bout(std::cout);

        framework::Commands::Version VersionCommand;
        framework::Commands::Version::Reply VersionReply = Module->WriteToDevice(VersionCommand);
        bout << "Version: " << VersionReply.ToString() << std::endl;
    }

    Module->WriteToDevice(CommandType::Sleep, false);
    Module->WriteToDevice(CommandType::Animate, false);
    Module->WriteToDevice(CommandType::Brightness, 30);

    Commands::Pattern PatternCommand{
        .Type = PatternType::Percentage,
    };
    for (int8_t i = 0; i <= 100; i++) {
        PatternCommand.Extra = i;
        Module->WriteToDevice(PatternCommand);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    Commands::GetBrightness::Reply BrightnessReply = Module->WriteToDevice<Commands::GetBrightness>();
    for (int i = 0; i < 3; i++) {
        Commands::Brightness BrightnessCommand{
            .BrightnessValue = 0,
        };
        Module->WriteToDevice(BrightnessCommand);
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
        BrightnessCommand.BrightnessValue = BrightnessReply.Brightness;
        Module->WriteToDevice(BrightnessCommand);
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
    }

    Module->WriteToDevice(CommandType::Pattern, static_cast<uint8_t>(PatternType::ZigZag));
    const Commands::Brightness BrightnessPatternCommand{
        .BrightnessValue = 50,
    };

    Module->WriteToDevice(BrightnessPatternCommand);

    Commands::Animate AnimateCommand{
        .bShouldAnimate = true,
    };
    Module->WriteToDevice(AnimateCommand);

    sleep(10);
    AnimateCommand.bShouldAnimate = false;
    Module->WriteToDevice(AnimateCommand);

    for (int8_t i = 100; i > 0; i--) {
        PatternCommand.Extra = i;
        Module->WriteToDevice(PatternCommand);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main(int ac, char** av)
{
    framework::InputModuleManager Manager;

    framework::IInputModule* const Module = Manager.GetInputModule(framework::InputModuleType::LEDMatrix, 0);
    framework::IInputModule* const Module2 = Manager.GetInputModule(framework::InputModuleType::LEDMatrix, 1);

    std::jthread Thread1 = std::jthread(SendTestCommand, Module);
    std::jthread Thread2 = std::jthread(SendTestCommand, Module2);

    return 0;
}
