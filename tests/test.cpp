#include "InputModule.hxx"

#include <thread>

template <typename... ModulesType>
void SendTestCommand(ModulesType... Modules)
{
    using namespace framework;

    (Modules->WriteToDevice(CommandType::Sleep, false), ...);
    (Modules->WriteToDevice(CommandType::Animate, false), ...);
    (Modules->WriteToDevice(CommandType::Brightness, 30), ...);

    Commands::Payload_Pattern PatternCommand(PatternType::Percentage);
    for (uint8_t i = 0; i <= 100; i++) {
        PatternCommand.Extra = i;
        (Modules->WriteToDevice(PatternCommand), ...);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    (Modules->WriteToDevice(CommandType::Pattern, static_cast<uint8_t>(PatternType::ZigZag)), ...);
    (Modules->WriteToDevice(CommandType::Animate, true), ...);
}

int main(int ac, char** av)
{
    framework::InputModuleManager Manager;

    framework::IInputModule* const Module = Manager.GetInputModule(framework::InputModuleType::LEDMatrix, 0);
    framework::IInputModule* const Module2 = Manager.GetInputModule(framework::InputModuleType::LEDMatrix, 1);

    SendTestCommand(Module, Module2);
    return 0;
}
