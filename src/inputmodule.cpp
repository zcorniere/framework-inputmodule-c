#include <cstdint>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>

class InputModule
{
public:
    enum class CommandType : uint8_t {
        Brightness = 0x00,
        Patern = 0x01,
        Bootloader = 0x02,
        Sleep = 0x03,
        GetSleep = 0x03,
        Animate = 0x04,

        PwnFrequency = 0x1E,
    };

    enum class PaternType : uint8_t {
        Percentage = 0x00,
        Gradient = 0x01,
        DoubleGradient = 0x02,
        DisplayLotusHorizontal = 0x03,
        ZigZag = 0x04,
        FullBrightness = 0x05,
    };

    struct [[gnu::packed]] Payload {
        const uint8_t Magic1 = 0x32;
        const uint8_t Magic2 = 0xAC;
        CommandType Type;
        uint8_t ExtraParameter;
        uint8_t ExtraParameter2;
    };

public:
    InputModule(const std::string& FileDevicePath): DeviceFD(open(FileDevicePath.c_str(), O_RDWR | O_NOCTTY))
    {
        if (DeviceFD < 0) {
            throw std::runtime_error("Failed to open device file");
        }
    }

    ~InputModule()
    {
        if (DeviceFD > 0) {
            close(DeviceFD);
        }
    }

    void SetPercentageLevel(int PercentageLevel)
    {
        WriteToDevice(CommandType::Patern, static_cast<uint8_t>(PaternType::Percentage), PercentageLevel);
    }

public:
    void WriteToDevice(CommandType Type, uint8_t ExtraParameter = 0, uint8_t ExtraParameter2 = 0)
    {
        const Payload Payload{
            .Type = Type,
            .ExtraParameter = ExtraParameter,
            .ExtraParameter2 = ExtraParameter2,
        };
        int size = write(DeviceFD, &Payload, sizeof(Payload));
        if (size != sizeof(Payload)) {
            throw std::runtime_error("Failed to write to device");
        }
    }

private:
    int DeviceFD = 0;
};

int main(int ac, char** av)
{
    InputModule InputModule("/dev/ttyACM0");
    InputModule.WriteToDevice(InputModule::CommandType::Sleep, false);
    InputModule.WriteToDevice(InputModule::CommandType::Animate, false);
    InputModule.WriteToDevice(InputModule::CommandType::Brightness, 30);

    for (int i = 0; i <= 100; i++) {
        InputModule.SetPercentageLevel(i);
        usleep(100000);
    }
    InputModule.WriteToDevice(InputModule::CommandType::Patern, static_cast<uint8_t>(InputModule::PaternType::ZigZag));
    InputModule.WriteToDevice(InputModule::CommandType::Animate, true);
    return 0;
}
