#include "InputModule.hxx"

struct InvalidInputStruct {
    int somehting;
};

int main(int ac, char** av)
{
    using namespace framework;

    InputModule Module(InputModuleType::LEDMatrix, "/dev/ttyACM0");
    Module.WriteToDevice(CommandType::Sleep, false);
    Module.WriteToDevice(CommandType::Animate, false);
    Module.WriteToDevice(CommandType::Brightness, 30);

    Commands::Payload_Pattern PatternCommand(PatternType::Percentage);
    for (uint8_t i = 0; i <= 100; i++) {
        PatternCommand.Extra = i;
        Module.WriteToDevice(PatternCommand);
        usleep(100000);
    }
    Module.WriteToDevice(CommandType::Pattern, static_cast<uint8_t>(PatternType::ZigZag));
    Module.WriteToDevice(CommandType::Animate, true);
    return 0;
}
