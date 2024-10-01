#ifndef ATOMIC_HPP
#define ATOMIC_HPP

#include <cstdint>

template <typename T, uint32_t address>
class atomic_utils
{
public:
    [[gnu::always_inline]] inline static bool CompareExchange(volatile T* ptr, T oldValue, T newValue)
    {
        //    // эксклюзивно читаем значение переменной и сравниваем со старым значением
        if(LDREX(ptr) == static_cast<T>(oldValue))
        {
            //    //  // пытаемся эксклюзивно записать в переменную новое значение
            return (STREX(static_cast<T>(newValue), static_cast<volatile T*>(ptr)) == 0);
            //
        }
        CLREX();
        return false;
    }

    [[gnu::always_inline]] inline static void Set(T mask, T value, T offset)
    {
        T newRegValue;

        do
        {
            newRegValue = LDREX(reinterpret_cast<volatile T *>(address));
            newRegValue &= ~(mask << offset);
            newRegValue |= (value << offset);
        }
        while(STREX(newRegValue, reinterpret_cast<volatile T *>(address)));
    }

    [[gnu::always_inline]] inline static void Toggle(T mask, T offset)
    {
        T newRegValue;

        do
        {
            newRegValue = LDREX(reinterpret_cast<volatile T *>(address));
            newRegValue ^= (mask << offset);
        }
        while(STREX(newRegValue, reinterpret_cast<volatile T *>(address)));
    }
private:
    [[gnu::always_inline]] inline static uint32_t LDREX(volatile uint32_t *addr)
    {
        uint32_t result;

        __asm volatile ("ldrex %0, %1" : "=r" (result) : "Q" (*addr) );
        return(result);
    }

    [[gnu::always_inline]] inline static uint32_t STREX(uint32_t value, volatile uint32_t *addr)
    {
        uint32_t result;

        __asm volatile ("strex %0, %2, %1" : "=&r" (result), "=Q" (*addr) : "r" (value) );
        return(result);
    }

    [[gnu::always_inline]] inline static uint16_t LDREX(volatile uint16_t *addr)
    {
        uint16_t result;

        __asm volatile ("ldrex %0, %1" : "=r" (result) : "Q" (*addr) );
        return(result);
    }

    [[gnu::always_inline]] inline static uint16_t STREX(uint16_t value, volatile uint16_t *addr)
    {
        uint16_t result;

        __asm volatile ("strex %0, %2, %1" : "=&r" (result), "=Q" (*addr) : "r" (value) );
        return(result);
    }

    [[gnu::always_inline]] inline static uint8_t LDREX(volatile uint8_t *addr)
    {
        uint8_t result;

        __asm volatile ("ldrex %0, %1" : "=r" (result) : "Q" (*addr) );
        return(result);
    }

    [[gnu::always_inline]] inline static uint8_t STREX(uint8_t value, volatile uint8_t *addr)
    {
        uint8_t result;

        __asm volatile ("strex %0, %2, %1" : "=&r" (result), "=Q" (*addr) : "r" (value) );
        return(result);
    }

    [[gnu::always_inline]] inline static void CLREX()
    {
        __asm volatile ("clrex" ::: "memory");
    }
};

#endif // ATOMIC_HPP
