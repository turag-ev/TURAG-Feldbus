#ifndef STELLANTRIEBEDEVICE_H
#define STELLANTRIEBEDEVICE_H
#include <tina++/feldbus/host/device.h>
#include <tina++/feldbus/host/legacystellantriebedevice.h>
#include <type_traits>
#include <cstring>

namespace TURAG {
namespace Feldbus {

class StellantriebeDevice: public Feldbus::Device {
public:
    StellantriebeDevice(const char* name, unsigned address, FeldbusAbstraction& feldbus,
                        ChecksumType type = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_CHECKSUM_TYPE,
                        const AddressLength addressLength = TURAG_FELDBUS_DEVICE_CONFIG_STANDARD_ADDRESS_LENGTH) :
                  Device(name, address, feldbus, type, addressLength),
                  first_command_(nullptr)
    { }

    bool init();
    using WriteAccess = Feldbus::LegacyStellantriebeDevice::Command_t::WriteAccess;
    using CommandLength = Feldbus::LegacyStellantriebeDevice::Command_t::CommandLength;
    enum class CommandType {
        real, control
    };
protected:
    //cache for writeable values
    template<typename T, WriteAccess>
    struct CommandCache { };
    template<typename T>
    struct CommandCache<T, WriteAccess::write> {
        T value_;
    };
    //apply conversion factor to real-valued commands
    template<typename T, bool real>
    struct CommandConversion { };

    template<typename T>
    struct CommandConversion<T, false> {
        void setFactor(float) { }
        T fromDevice(T value) { return value; }
        T toDevice(T value) { return value; }
    };
    template<typename T>
    struct CommandConversion<T, true> {
        void setFactor(float factor) { factor_ = factor; }
        float factor_;
        float fromDevice(T value) { return value * factor_; }
        T toDevice(float value) { return value / factor_; }
    };

    //translate types to CommandLength
    //dummy parameter avoids explicit template
    //specialization at non-namespace scope :/
    template<typename T, typename dummy=int>
    struct TypeCommandLength {};
    template<typename dummy>
    struct TypeCommandLength<int8_t,dummy>  { static constexpr auto value = CommandLength::length_char; };
    template<typename dummy>
    struct TypeCommandLength<int16_t,dummy> { static constexpr auto value = CommandLength::length_short; };
    template<typename dummy>
    struct TypeCommandLength<int32_t,dummy> { static constexpr auto value = CommandLength::length_long; };
    template<typename dummy>
    struct TypeCommandLength<float,dummy>   { static constexpr auto value = CommandLength::length_float; };

    class CommandBase {
    public:
        CommandBase(const char* name, StellantriebeDevice* dev):
            name_(name), dev_(dev), key_(0), next_(nullptr)
        {
            //append to command list of device
            if(!dev_->first_command_) {
                dev_->first_command_ = this;
            } else {
                CommandBase* list_end = dev_->first_command_;
                while(list_end->next_)
                    list_end = list_end->next_;
                list_end->next_ = this;
            }
        }
        //name used to identify command in the command set
        const char* name() const { return name_; }
        //properties of command (have to match the information returned by the device)
        virtual WriteAccess access() const = 0;
        virtual CommandType type() const = 0;
        virtual CommandLength length() const = 0;

        //let StellantriebeDevice set properties obtained from device (TODO: clean design)
        virtual void setConversionFactor(float) = 0;
        void setKey(uint8_t key) { key_ = key; }
        bool assert_initialized() const;

        CommandBase* next() const { return next_; }
    protected:
        const char* name_;
        StellantriebeDevice* dev_;
        uint8_t key_;
    private:
        CommandBase* next_;
    };

    template<typename T, WriteAccess writeaccess, CommandType cmdtype,
             typename InterfaceType = typename std::conditional<cmdtype == CommandType::real, float, T>::type>
    class Command:
            public CommandBase,
            private CommandCache<InterfaceType, writeaccess>,
            private CommandConversion<T,cmdtype == CommandType::real> {
    public:
        Command(const char* name, StellantriebeDevice* dev):
            CommandBase(name, dev)
        {}
        WriteAccess access() const override { return writeaccess; }
        CommandType type() const override { return cmdtype; }
        CommandLength length() const override { return TypeCommandLength<T>::value; }

        void setConversionFactor(float factor) override { this->setFactor(factor); }

        template<bool condition = (writeaccess == WriteAccess::write)>
        typename std::enable_if<condition, bool>::type
        getValue(InterfaceType* value) {
            //return value from cache
            if (value)
                *value = this->value_;
            return true;
        }
        template<bool condition = (writeaccess == WriteAccess::write)>
        typename std::enable_if<!condition, bool>::type
        getValue(InterfaceType* value) {
            if (!assert_initialized())
                return false;
            //read value from device
            T v;
            if(!dev_->getValue<T>(key_, &v)) {
                return false;
            } else {
                if (value)
                    *value = this->fromDevice(v);
                return true;
            }
        }
        bool setValue(InterfaceType value) {
            static_assert(writeaccess == WriteAccess::write, "Command does not have write access!");
            if (!assert_initialized())
                return false;
            this->value_ = value;
            return dev_->setValue<T>(key_, this->toDevice(value));
        }
    };
private:
    CommandBase* first_command_;

    template<typename T>
    bool getValue(uint8_t key, T* value) {
        Request<uint8_t> req;
        req.data = key;
        Response<T> resp;
        if(!transceive(req,&resp))
            return false;
        if (value)
            *value = resp.data; //assumes host is little-endian!
        return true;
    }
    template<typename T>
    bool setValue(uint8_t key, T value) {
        struct Data {
            uint8_t key;
            T value;
        } _packed;
        Request<Data> req;
        req.data.key = key;
        req.data.value = value; //assumes host is little-endian
        Response<> resp;
        return transceive(req,&resp);
    }
};

} // namespace Feldbus
} // namespace TURAG

#endif // STELLANTRIEBEDEVICE_H
