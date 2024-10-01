#ifndef PORT_HPP
#define PORT_HPP

#include "register.hpp"
#include "atomic.hpp"

template <typename T>
class Port
{
    using SCRType = typename T::SCR::Type;

    enum Configuration : uint32_t
    {
        ANALOG_INPUT = 0b00,
        FLOATING_INPUT = 0b01,
        PULL_INPUT = 0b10,
        PUSHPULL_OUTPUT = 0b00,

    };

    static constexpr std::uint8_t PinsCount = 16U;
private:
    __forceinline static void Set(std::uint32_t value)
    {
        T::SCR::Set(static_cast<SCRType>(value));
    }

    __forceinline static void Reset(std::uint32_t value)
    {
        T::SCR::Set(static_cast<SCRType>(value) << PinsCount);
    }

    __forceinline static void TogglePin(std::uint32_t pinNum)
    {
        T::SCR::Set(T::ODT::Get() ^ (1U << pinNum));
    }

    __forceinline static auto GetInput()
    {
        return T::IDT::Get();
    }

    template<typename... Ps>
    __forceinline static void SetAnalog(Ps... pins)
    {
        using RegType = typename T::MODER::Type;
        AtomicUtils<RegType>::Set(
            T::MODER::Address,
            T::MODER::FieldValues::Analog::Mask,
            T::MODER::FieldValues::Analog::Value,
            static_cast<ModerType>(pinNum * uint8_t{2U})
            );
    }

    __forceinline static void SetInput(std::uint32_t pinNum)
    {
        AtomicUtils<ModerType>::Set(
            T::MODER::Address,
            0b11,
            T::MODER::FieldValues::Input::Value,
            static_cast<ModerType>(pinNum * uint8_t{2U})
            );
        volatile auto value = T::MODER::Get() ;
        value &= ~(3 << (pinNum * 2U)) ;
        value |= 	(T::MODER::FieldValues::Input::Value << (pinNum * 2U)) ;
        T::MODER::Write(value);
    }

    __forceinline static void SetOutput(std::uint32_t pinNum)
    {
        AtomicUtils<ModerType>::Set(
            T::MODER::Address,
            T::MODER::FieldValues::Output::Mask,
            T::MODER::FieldValues::Output::Value,
            static_cast<ModerType>(pinNum * uint8_t{2U})
            );
    }

    __forceinline static void SetAlternate(std::uint32_t pinNum)
    {
        AtomicUtils<ModerType>::Set(
            T::MODER::Address,
            T::MODER::FieldValues::Alternate::Mask,
            T::MODER::FieldValues::Alternate::Value,
            static_cast<ModerType>(pinNum * uint8_t{2U})
            );
    }

} ;



template<typename ...T>
struct Pins{

    __forceinline inline static void Toggle()
    {
        Pass((T::Toggle(), true)...) ;
    }

    __forceinline inline static void Set()
    {
        Pass((T::Set(), true)...) ;
    }

    __forceinline inline static void Reset()
    {
        Pass((T::Reset(), true)...) ;
    }

    __forceinline inline static void SetOutput()
    {
        Pass((T::SetOutput(), true)...) ;
    }

    __forceinline inline static void SetInput()
    {
        Pass((T::SetInput(), true)...) ;
    }

    __forceinline inline static void SetAnalog()
    {
        Pass((T::SetInput(), true)...) ;
    }

    __forceinline inline static void SetAlternate()
    {
        Pass((T::SetInput(), true)...) ;
    }

private:
    __forceinline template<typename... Args>
    static void inline Pass(Args... ) //Вспомогательный метод для распаковки вариативного шаблона
    {
    }

} ;

#endif // PORT_HPP
