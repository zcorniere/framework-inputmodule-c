#pragma once

#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <vector>

#include "ModuleEnums.hxx"

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

/// Main class representing an input module
class IInputModule
{
private:
    struct [[gnu::packed]] Payload {
        const uint8_t Magic1 = 0x32;
        const uint8_t Magic2 = 0xAC;
        CommandType Type;
        /// Extra parameter for the command need to be sent separately
    };

public:
    explicit IInputModule(InputModuleType Type): Type(Type)
    {
    }

    virtual bool IsValid() const = 0;

    template <typename... ArgsType>
    /// Send a command to the device
    ///
    /// @return >= 0 on success, -1 on error
    /// The return value contain the number of bytes the device will send back as a reply
    int WriteToDevice(CommandType Type, ArgsType... Args)
    {
        INPUTMODULE_ASSERT(IsValid());
        const Payload Payload{
            .Type = Type,
        };
        // Build the buffer to send to the device
        // The buffer is initialized to the be correct size
        std::vector<uint8_t> PayloadData(sizeof(Payload) + sizeof...(Args));
        std::memcpy(PayloadData.data(), &Payload, sizeof(Payload));
        /// And data are copied into it
        std::size_t Offset = sizeof(Payload);
        ((PayloadData[Offset++] = uint8_t(Args)), ...);

        WriteToDevice_Internal(PayloadData);
        return 0;
    }

protected:
    virtual int WriteToDevice_Internal(const std::vector<uint8_t>& Data) = 0;

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
    virtual int WriteToDevice_Internal(const std::vector<uint8_t>& Data) override
    {
        return write(DeviceFD, Data.data(), Data.size());
    }

private:
    int DeviceFD = 0;
};

using InputModule = InputModuleLinux;

#elif defined(INPUTMODULE_PLATFORM_WINDOWS)

#endif    // INPUTMODULE_PLATFORM_WINDOWS

}    // namespace framework
