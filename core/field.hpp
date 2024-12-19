#ifndef FIELD_HPP
#define FIELD_HPP

#include <concepts>
#include <bit>
#include <cstddef>

#include "register.hpp"

namespace metaMCU::core {

    /// \brief Проверка возможности атомарной записи в регистр
    template<typename Register>
    concept Can_atomic_write = requires
    {
        Register::bits_set_clear_atomic(0x0, 0x0);
        Register::bits_toggle_atomic();
    };

    /// \brief Проверка возможности bit band записи в регистр
    template<typename Register>
    concept Can_bit_band = requires
    {
        Register::bit_band_set(0x0);
        Register::bit_band_clear(0x0);
    };

    template<typename Field, Field::Value_t Value>
    class Field_value
    {
    public:
        [[gnu::always_inline]] static void set()
        {
            Field::set(Value);
        }

        [[gnu::always_inline]] inline static bool is_set()
        {
            return Field::get() == Value;
        }

        template<typename FV1, typename FV2>
        consteval auto operator|(this FV1 lhs, FV2 rhs)
        {
            constexpr auto offset = 0;
            constexpr auto mask = lhs.mask_with_offset() | rhs.mask_with_offset();
            //constexpr bool canRead = lhs.a;
            //constexpr Field<F1::Register_t, offset, mask, F1::Access_t>
            return rhs;
        }
    };

    /*!
     * \brief Обеспечивает безопасный доступ к битовым полям регистров
     * микроконтроллера
     * \warning Данный шаблонный класс разработан для работы вместе
     * со скриптом RegistersGenerator, подставляющим в шаблонные
     * параметры по информацию из svd файла и автоматически
     * генерирующим соответствующие заголовочные файлы. Инстанцирование этого
     * класса "вручную" должно производится только при крайней необходимости.
     * \tparam Register Регистр поля
     * \tparam offset Смещение поля в бит
     * \tparam size Размер поля в бит
     * \tparam Access Тип доступа (WriteMode, ReadMode или ReadWriteMode)
     */
    template<typename Register, Register::Value_t Offset, Register::Value_t Mask, typename Access>
    class Field : protected Register
    {
    public:
        using Value_t = typename Register::Value_t;

        /// \brief Записывает значение в битовое поле регистра, если регистр позволяет запись
        template<typename T = void>
            requires Can_read<Access> && Can_write<Access>
        [[gnu::always_inline]] inline static void set(Value_t value)
        {
            auto register_value = Register::read();

            register_value &= ~mask_with_offset();
            register_value |= (value << Offset);

            Register::write(register_value);
        }

        /// \brief Записывает значение в битовое поле регистра, если регистр позволяет запись
        template<typename T = void>
            requires Can_write<Access>
        [[gnu::always_inline]] inline static void write(Value_t value)
        {
            static_assert(value_in_range(value));
            dynamic_write(value);
        }

        /// \brief Возвращает значение битового поля регистра
        template<typename Value>
            requires Can_read<Access>
        [[gnu::always_inline]] inline static Register::Value_t get()
        {
            return (Register::read() & mask_with_offset) >> Offset;
        }

        template<typename F1, typename R1, R1::Value_t O1, R1::Value_t M1, typename A1, typename F2>
        consteval auto operator|(this F1<R1, O1> lhs, F2 rhs)
        {
            constexpr auto offset = 0;
            constexpr auto mask = lhs.mask_with_offset() | rhs.mask_with_offset();
            //constexpr bool canRead = lhs.a;
            //constexpr Field<F1::Register_t, offset, mask, F1::Access_t>
            return rhs;
        }

    protected:
/*        using Register_t = Register;
        using Access_t = Access;*/

        static consteval bool value_in_range(Value_t value)
        {
            return (value & ~mask_with_offset()) == 0;
        }

        static consteval auto bit_offset()
        {
            return Offset;
        }

        static consteval auto size()
        {
            return std::popcount(Mask);
        }

        static consteval auto mask_with_offset()
        {
            return Mask << Offset;
        }

        template<typename F, F::Value_t V>
        friend class Field_value;
    };
}

#endif // FIELD_HPP
