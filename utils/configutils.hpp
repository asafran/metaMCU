#ifndef CONFIGUTILS_HPP
#define CONFIGUTILS_HPP

#include <concepts>

template <typename Pin, typename SpecializationKey = Pin::SpecializationKey>
struct PinsConfiguration {};

enum PinMode
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

template<typename Configuration>
concept PinConfiguredAsInput = requires {
    requires Configuration::mode == FLOAT_INPUT ||
             Configuration::mode == PULLUP_INPUT ||
             Configuration::mode == PULLDOWN_INPUT;
};

template<typename Configuration>
concept PinConfiguredAsAnalog = requires {
    requires Configuration::mode == ANALOG_INPUT;
};

template<typename Configuration>
concept PinConfiguredAsOutput = requires {
    requires Configuration::mode == PUSHPULL_OUTPUT ||
             Configuration::mode == OPENDRAIN_OUTPUT ||
             Configuration::mode == AUX_PUSHPULL_OUTPUT ||
             Configuration::mode == AUX_OPENDRAIN_OUTPUT;
};

template<typename Configuration>
concept CanConfigure = requires {
    requires Configuration::Policy == PIN_CONFIGURABLE;
};

template<typename Pin>
concept CanInput = CanConfigure<PinsConfiguration<Pin>> || PinConfiguredAsInput<PinsConfiguration<Pin>>;

template<typename Pin>
concept CanAnalog = CanConfigure<PinsConfiguration<Pin>> || PinConfiguredAsAnalog<PinsConfiguration<Pin>>;

template<typename Pin>
concept CanOutput = CanConfigure<PinsConfiguration<Pin>> || PinConfiguredAsOutput<PinsConfiguration<Pin>>;

template<PinMode mode, PinStrenght strenght = NORMAL_STR, PinPolicy policy = PIN_NON_CONFIGURABLE>
struct StartupConfiguration
{
    constexpr static PinMode Mode = mode;
    constexpr static PinStrenght Strenght = strenght;
    constexpr static PinPolicy Policy = policy;
};

#endif // CONFIGUTILS_HPP
