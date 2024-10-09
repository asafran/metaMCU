#ifndef PIN_HPP
#define PIN_HPP

#include "metautils.hpp"
#include "configutils.hpp"

template<typename Interface>
concept HasClearSet = IsFieldValue<typename Interface::SetOutputValue> && IsFieldValue<typename Interface::ClearOutputValue>;

/*
template<typename Interface>
concept CompatibleInterface = IsFieldValue<typename Interface::HighOutputValue> && IsFieldValue<typename Interface::LowOutputValue> &&
                              IsFieldValue<typename Interface::FloatingModeValue> && IsFieldValue<typename Interface::PushPullModeValue> &&
                              IsFieldValue<typename Interface::ReadValue>

    requires
{
    Interface::OutputField;
};
*/

template<typename Pin>
    //requires CompatibleInterface<Interface<GPIO, pinNum>>
class Pins
{
public:
    template<typename T = void>
        requires CanOutput<Configuration>
    [[gnu::always_inline]] inline static void SetHigh()
    {
        if constexpr (HasClearSet<Interface>)
            Interface::SetOutputValue::Set();
        else
            Interface::HighOutputValue::AtomicSet();
    }

    template<typename T = void>
        requires CanOutput<Configuration>
    [[gnu::always_inline]] inline static void SetLow()
    {
        if constexpr (HasClearSet<Interface>)
            Interface::ClearOutputValue::Set();
        else
            Interface::LowOutputValue::AtomicSet();
    }

    template<typename T = void>
        requires CanOutput<Configuration>
    [[gnu::always_inline]] inline static bool IsSet()
    {
        return Interface::HighOutputValue::IsSet();
    }

    template<typename T = void>
        requires CanOutput<Configuration> && requires { Interface::OutputField; }
    [[gnu::always_inline]] inline static void Toggle()
    {
        Interface::OutputField::AtomicToggle();
    }

    template<typename T = void>
        requires CanInput<Configuration>
    [[gnu::always_inline]] inline static bool GetInput()
    {
        return Interface::ReadValue::IsSet();
    }

    [[gnu::always_inline]] inline static void Reset()
    {
        SetFloatingInput();
    }

    template<typename T = void>
        requires CanAnalog<Configuration>
    [[gnu::always_inline]] inline static void SetAnalog()
    {
        Reset();
        Interface::AnalogModeValue::AtomicSet();
    }

    template<typename T = void>
        requires CanInput<Configuration>
    [[gnu::always_inline]] inline static void SetFloatingInput()
    {
        Interface::LowOutputValue::AtomicSet();
        Interface::FloatingModeValue::AtomicSet();
        Interface::InputModeValue::AtomicSet();
    }

    template<typename T = void>
        requires CanInput<Configuration>
    [[gnu::always_inline]] inline static void SetPullUpInput()
    {
        Reset();
        Interface::PullUpDownModeValue::AtomicSet();
        Interface::PullUpValue::AtomicSet();
    }

    template<typename T = void>
        requires CanInput<Configuration>
    [[gnu::always_inline]] inline static void SetPullDownInput()
    {
        Reset();
        Interface::PullUpDownModeValue::AtomicSet();
        Interface::PullDownValue::AtomicSet();
    }

    template<PinStrenght strenght = NORMAL_STR>
        requires CanOutput<Configuration>
    [[gnu::always_inline]] inline static void SetStrenght()
    {
        switch (strenght) {
        case NORMAL_STR:
            Interface::NormalOutputModeValue::AtomicSet();
            Interface::NormalStrValue::AtomicSet();
            break;
        case LARGE_STR:
            Interface::LargeOutputModeValue::AtomicSet();
            Interface::NormalStrValue::AtomicSet();
            break;
        case MAX_STR:
            Interface::LargeOutputModeValue::AtomicSet();
            Interface::MaximumStrValue::AtomicSet();
            break;
        }
    }

    template<PinStrenght strenght = NORMAL_STR>
            requires CanOutput<Configuration>
    [[gnu::always_inline]] inline static void SetOutput()
    {
        Interface::LowOutputValue::AtomicSet();
        Interface::PushPullModeValue::AtomicSet();
        SetStrenght<strenght>();
    }

    template<PinStrenght strenght = NORMAL_STR>
        requires CanOutput<Configuration> && IsFieldValue<typename Interface::OpenDrainModeValue>
    [[gnu::always_inline]] inline static void SetOpenDrainOutput()
    {
        Interface::LowOutputValue::AtomicSet();
        Interface::OpenDrainModeValue::AtomicSet();
        SetStrenght<strenght>();
    }

    template<PinStrenght strenght = NORMAL_STR>
        requires CanOutput<Configuration> && IsFieldValue<typename Interface::AltPushPullModeValue>
    [[gnu::always_inline]] inline static void SetAltPushPull()
    {
        Interface::LowOutputValue::AtomicSet();
        Interface::AltPushPullModeValue::AtomicSet();
        SetStrenght<strenght>();
    }

    template<PinStrenght strenght = NORMAL_STR>
        requires CanOutput<Configuration>
    [[gnu::always_inline]] inline static void SetAltOpenDrain()
    {
        Interface::LowOutputValue::AtomicSet();
        Interface::AltOpenDrainModeValue::AtomicSet();
        SetStrenght<strenght>();
    }

    template<PinConfiguration mode = Configuration::mode, PinStrenght strenght = Configuration::strenght>
    [[gnu::always_inline]] static inline void Configure()
    {
        switch (mode) {
        case ANALOG_INPUT:
            SetAnalog();
            break;
        case FLOAT_INPUT:
            SetFloatingInput();
            break;
        case PULLUP_INPUT:
            SetPullUpInput();
            break;
        case PULLDOWN_INPUT:
            SetPullDownInput();
            break;
        case PUSHPULL_OUTPUT:
            SetOutput<strenght>();
            break;
        case OPENDRAIN_OUTPUT:
            SetOpenDrainOutput<strenght>();
            break;
        case AUX_PUSHPULL_OUTPUT:
            SetAltPushPull<strenght>();
            break;
        case AUX_OPENDRAIN_OUTPUT:
            SetAltOpenDrain<strenght>();
            break;
        }
    }
};

#endif // PIN_HPP
