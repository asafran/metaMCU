#ifndef CONFIGUTILS_HPP
#define CONFIGUTILS_HPP

#include <concepts>

enum PinConfiguration
{
    ANALOG_INPUT,
    FLOAT_INPUT,
    PULLUP_INPUT,
    PULLDOWN_INPUT,
    PUSHPULL_OUTPUT,
    OPENDRAIN_OUTPUT,
    AUX_PUSHPULL_OUTPUT,
    AUX_OPENDRAIN_OUTPUT
};

enum PinStrenght
{
    NORMAL_STR,
    LARGE_STR,
    MAX_STR
};

enum PinPolicy
{
    PIN_CONFIGURABLE,
    PIN_NON_CONFIGURABLE
};

template<PinConfiguration mode, PinStrenght strenght>
concept PinConfiguredAsInput = requires {
    requires mode == FLOAT_INPUT || mode == PULLUP_INPUT || mode == PULLDOWN_INPUT;
};

template<PinConfiguration mode, PinStrenght strenght>
concept PinConfiguredAsAnalog = requires {
    requires mode == ANALOG_INPUT;
};

template<PinConfiguration mode, PinStrenght strenght>
concept PinConfiguredAsOutput = requires {
    requires mode == PUSHPULL_OUTPUT || mode == OPENDRAIN_OUTPUT || mode == AUX_PUSHPULL_OUTPUT || mode == AUX_OPENDRAIN_OUTPUT;
};

template<typename Pin>
concept CanConfigure = requires {
    requires Pin::Policy == PIN_CONFIGURABLE;
};

template<typename Pin>
concept CanInput = CanConfigure<Pin> || PinConfiguredAsInput<Pin::Mode, Pin::Strenght>;

template<typename Pin>
concept CanAnalog = CanConfigure<Pin> || PinConfiguredAsAnalog<Pin::Mode, Pin::Strenght>;

template<typename Pin>
concept CanOutput = CanConfigure<Pin> || PinConfiguredAsOutput<Pin::Mode, Pin::Strenght>;

template<PinConfiguration mode, PinStrenght strenght = NORMAL_STR, PinPolicy policy = PIN_NON_CONFIGURABLE>
struct StartupConfiguration
{
    constexpr static PinConfiguration Mode = mode;
    constexpr static PinStrenght Strenght = strenght;
    constexpr static PinPolicy Policy = policy;
};

#endif // CONFIGUTILS_HPP
