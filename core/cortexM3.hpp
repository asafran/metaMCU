#ifndef CORTEXM3_HPP
#define CORTEXM3_HPP

#include "register.hpp"

/*!
 * \file
 * \brief Файл с классами для работы с регистрами
 *
 * В этом заголовочнике содержатся статические классы для
 * реализации безопасного доступа к регистрам микроконтроллера.
 * А так же служебные типы и концепты.
 */

namespace metaMCU {

    template <size_t address, typename Value, typename Access>
    class Register_CortexM3 : Register<address, Value, Access>
    {
    private:
        static constexpr size_t peripheral_base_addr = 0x4;
        static constexpr size_t bit_band_base_addr = 0x4;

    public:
        using Value_t = typename Register<address, Value, Access>::Value_t;

        template<typename T = void>
            requires Can_write<Access> && Can_read<Access>
        [[gnu::always_inline]] inline static void bits_set_clear_atomic(Value_t set, Value_t clear)
        {
            Value_t new_value;

            do
            {
                new_value = LDREX(reinterpret_cast<volatile Value_t*>(address));
                new_value &= ~reset;
                new_value |= set;
            }
            while(STREX(new_value, reinterpret_cast<volatile Value_t*>(address)));
        }

        template<typename T = void>
            requires Can_write<Access> && Can_read<Access>
        [[gnu::always_inline]] inline static void bits_toggle_atomic(Value_t mask)
        {
            Value_t new_value;

            do
            {
                new_value = LDREX(reinterpret_cast<volatile Value_t*>(address));
                new_value ^= mask;
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
        static consteval auto bit_band_word_addr(size_t offset)
        {
            return bit_band_base_addr + 32 * (address - peripheral_base_addr) + 4 * offset;
        }

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

    template<typename Register, size_t offset, size_t size, typename Access>
    class Field
    {

    };
}



#endif // CORTEXM3_HPP
