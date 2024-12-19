#ifndef FIELD_HPP
#define FIELD_HPP

#include <concepts>
#include <bit>
#include <cstddef>

#include "register.hpp"

namespace metaMCU::core {

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
    class Field
    {
    public:
        using Value_t = typename Register::Value_t;

    public:
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
            Register::write(value << Offset);
        }

        /// \brief Возвращает значение битового поля регистра
        template<typename Value>
            requires Can_read<Access>
        [[gnu::always_inline]] inline static Register::Value_t get()
        {
            return (Register::read() & mask_with_offset) >> Offset;
        }

    protected:
        static consteval bool value_in_range(Value_t value)
        {
            return (value & ~mask_with_offset()) == 0;
        }

        static consteval auto mask_with_offset()
        {
            return Mask << Offset;
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
}

#endif // FIELD_HPP
