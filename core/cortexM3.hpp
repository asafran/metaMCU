#ifndef CORTEXM3_HPP
#define CORTEXM3_HPP

#include "register.hpp"
#include "field.hpp"
#include "value.hpp"

/*!
 * \file
 * \brief Файл с классами для работы с регистрами
 *
 * В этом заголовочнике содержатся статические классы для
 * реализации безопасного доступа к регистрам микроконтроллера.
 * А так же служебные типы и концепты.
 */

namespace metaMCU::CortexM3 {

    template<typename T>
    consteval bool is_single_bit (T mask)
    {
        return std::popcount(mask) == 1;
    }

    template<size_t Address, typename Value, typename Access>
    using Register = core::Register<Address, Value, Access>;

    template<typename Register, Register::Value_t Offset, Register::Value_t Mask, typename Access>
    class Field : public core::Field<Register, Offset, Mask, Access>
    {
    public:
        using Value_t = core::Field<Register, Offset, Mask, Access>::Value_t;

        /// \brief Записывает значение в битовое поле регистра c использованием LDREX, STREX
        template<typename T = void>
            requires Can_read<Access> && Can_write<Access>
        [[gnu::always_inline]] inline static void set_atomic(Value_t value)
        {
            Value_t register_value;

            do
            {
                register_value = LDREX(reinterpret_cast<volatile Value_t*>(Register::address()));
                register_value &= ~Field::mask_with_offset();
                register_value |= (value << Offset);
            }
            while(STREX(register_value, reinterpret_cast<volatile Value_t*>(Register::address())));
        }

    private:
        [[gnu::always_inline]] inline static Value_t LDREX(volatile Value_t *addr)
        {
            Value_t result;

            __asm volatile ("ldrex %0, %1" : "=r" (result) : "Q" (*addr) );
            return(result);
        }

        [[gnu::always_inline]] inline static Value_t STREX(Value_t value, volatile Value_t *addr)
        {
            Value_t result;

            __asm volatile ("strex %0, %2, %1" : "=&r" (result), "=Q" (*addr) : "r" (value) );
            return(result);
        }

        [[gnu::always_inline]] inline static void CLREX()
        {
            __asm volatile ("clrex" ::: "memory");
        }

    public:
        template<typename F1, typename F2>
        consteval auto operator|(this F1 lhs, F2 rhs)
        {
            constexpr auto offset = 0;
            constexpr auto mask = lhs.mask_with_offset() | rhs.mask_with_offset();
            return Field<Register, offset, mask, Access>();
        }

        template<typename R, R::Value_t O, R::Value_t M, typename A>
        friend class Field;
    };

    template<typename Register, Register::Value_t Offset, Register::Value_t Mask, typename Access>
        requires Can_write<Access> && (is_single_bit(Mask))
    class Field<Register, Offset, Mask, Access> : public core::Field<Register, Offset, Mask, Access>
    {
    public:
        using Value_t = typename Register::Value_t;

    private:
        static constexpr size_t peripheral_base_addr = 0x4;
        static constexpr size_t bit_band_base_addr = 0x4;

        static consteval auto bit_band_word_addr()
        {
            return bit_band_base_addr + 32 * (Register::address() - peripheral_base_addr) + 4 * Offset;
        }

    protected:
        [[gnu::always_inline]] inline static void set(Value_t value)
        {
            *reinterpret_cast<volatile Value_t*>(bit_band_word_addr()) = value;
        }

        [[gnu::always_inline]] inline static void write(Value_t value)
        {
            set(value);
        }

        [[gnu::always_inline]] inline static void set_atomic(Value_t value)
        {
            set(value);
        }


    };

    template<typename Field, typename Field::Value_t Value>
    class Field_value : public core::Field_value<Field, Value>
    {
    public:
        [[gnu::always_inline]] inline static void set_atomic()
        {
            Field::set_atomic(Value);
        }

        template<typename F1, F1::Value_t V1, typename F2, F2::Value_t V2>
        consteval auto operator|(this Field_value<F1, V1> lhs, Field_value<F2, V2> rhs)
        {
            constexpr auto value = lhs.value() | rhs.value();
            constexpr auto field = F1() | F2();
            return Field_value<decltype(field), value>();
        }

        template<typename F, F::Value_t V>
        friend class Field_value;
    };
}



#endif // CORTEXM3_HPP
