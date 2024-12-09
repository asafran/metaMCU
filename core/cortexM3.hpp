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

    template <size_t address, size_t size, typename Access>
    struct CortexM3Register : Register<address, size, Access>
    {
    public:
        static constexpr size_t Peripheral_base_addr = 0x4;

        static constexpr size_t Bit_band_base_addr = 0x4;

        using Value_t = typename Register<address, size, Access>::Value_t;

        template<typename T = void>
            requires Can_write<Access> && Can_read<Access>
        [[gnu::always_inline]] inline static void CompareExchangeSetReset(Value_t set, Value_t reset)
        {
            Value_t new_value;

            do
            {
                new_value = LDREX(reinterpret_cast<volatile Value_t *>(address));
                new_value &= ~reset;
                new_value |= set;
            }
            while(STREX(new_value, reinterpret_cast<volatile Value_t *>(address)));
        }

        template<typename T = void>
            requires Can_write<Access> && Can_read<Access>
        [[gnu::always_inline]] inline static void CompareExchangeToggle(Value_t mask)
        {
            Value_t new_value;

            do
            {
                new_value = LDREX(reinterpret_cast<volatile Value_t *>(address));
                new_value ^= mask;
            }
            while(STREX(new_value, reinterpret_cast<volatile Value_t *>(address)));
        }

        template<typename T = void>
            requires Can_write<Access>
        [[gnu::always_inline]] inline static void BitBand(Value_t value, Value_t offset)
        {
            constexpr auto bit_word_addr = Bit_band_base_addr + 32 * (address - Peripheral_base_addr) + 4 * offset;
            *reinterpret_cast<volatile Value_t *>(bit_word_addr) = value;
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
}



#endif // CORTEXM3_HPP
