#ifndef CORTEXM3_HPP
#define CORTEXM3_HPP

#include "field.hpp"
#include "register.hpp"

/*!
 * \file
 * \brief Файл с классами для работы с регистрами
 *
 * В этом заголовочнике содержатся статические классы для
 * реализации безопасного доступа к регистрам микроконтроллера.
 * А так же служебные типы и концепты.
 */

namespace metaMCU::CortexM3 {

    template <size_t address, typename Value, typename Access>
    class Register : public core::Register<address, Value, Access>
    {
    public:
        using Value_t = typename core::Register<address, Value, Access>::Value_t;

        template<typename... Values>
            requires Can_write<Access> && Can_read<Access>
        [[gnu::always_inline]] inline static void values_set_atomic()
        {
            Value_t new_value;

            do
            {
                new_value = LDREX(reinterpret_cast<volatile Value_t*>(address));
                new_value &= ~calculateMask<Values...>();
                new_value |= accumulateValues<Values...>();
            }
            while(STREX(new_value, reinterpret_cast<volatile Value_t*>(address)));
        }

        template<typename T = void>
            requires Can_write<Access>
        [[gnu::always_inline]] inline static void bit_band_set(size_t bit_offset)
        {
            *reinterpret_cast<volatile Value_t*>(bit_band_word_addr(offset)) = 0x01;
        }

        template<typename T = void>
            requires Can_write<Access>
        [[gnu::always_inline]] inline static void bit_band_clear(size_t bit_offset)
        {
            *reinterpret_cast<volatile Value_t*>(bit_band_word_addr(offset)) = 0x00;
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
    };

    template<typename Register, size_t Offset, size_t Size, typename Access>
    class Field : public core::Field<Register, Offset, Size, Access>
    {
    protected:
        /// \brief Записывает значение в битовое поле регистра c использованием LDREX, STREX
        template<typename Value>
            requires Can_write<Access> && Can_read<Access>
        [[gnu::always_inline]] inline static void set_atomic()
        {
            Register::template values_set_atomic<Value>();
        }
    };

    template<typename Register, size_t Offset, typename Access>
        requires Can_write<Access>
    class Field<Register, Offset, 1, Access> : public core::Field<Register, Offset, 1, Access>
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
        template<typename Val>
        [[gnu::always_inline]] inline static void set()
        {
            *reinterpret_cast<volatile Value_t*>(bit_band_word_addr()) = Val::value();
        }

        template<typename Value>
        [[gnu::always_inline]] inline static void write()
        {
            set();
        }

        template<typename Value>
        [[gnu::always_inline]] inline static void set_atomic()
        {
            set();
        }
    };

    template<typename Field, typename Field::Size_t Value>
    class Field_value : public Field
    {
        /// \brief Значение битового поля без смещения
        static consteval auto value()
        {
            return Value;
        }

        [[gnu::always_inline]] static void set()
        {
            Field::template set<Field_value>();
        }

        [[gnu::always_inline]] inline static void set_atomic()
        {
            Field::template set_atomic<Field_value>();
        }

        [[gnu::always_inline]] inline static bool is_set()
        {
            return Field::template is_set<Field_value>();
        }
    };
}



#endif // CORTEXM3_HPP
