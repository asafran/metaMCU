#ifndef FIELD_HPP
#define FIELD_HPP

#include <concepts>
#include <cstddef>

#include "register.hpp"

namespace metaMCU {

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
    template<typename Register, size_t offset, size_t size, typename Access>
    class Field
    {
    public:
        using Value_t = typename Register::Value_t;

        /// \brief Смещение поля в бит от 0
        static consteval auto bit_offset()
        {
            return offset;
        }

        /// \brief Размер поля в бит
        static consteval auto size()
        {
            return size;
        }

        /// \brief Маска битового поля
        static consteval auto mask()
        {
            return ((1 << size()) - 1) << bit_offset();
        }

        /// \brief Проверяет принадлежность поля данному регистру
        template<typename R>
        static consteval bool register_is_compatible()
        {
            return std::derived_from<Register, R>;
        }

    protected:
        /// \brief Записывает значение в битовое поле регистра, если регистр позволяет запись
        template<Value_t value>
            requires Can_read<Access> && Can_write<Access>
        [[gnu::always_inline]] inline static void set()
        {
            if constexpr(Can_bit_band<Register> && size() == 1)
                value ? Register::bit_band_set(offset()) : Register::bit_band_reset(offset());
            else
                Register::bits_set_clear((value << bit_offset()), mask());
        }

        /// \brief Записывает значение в битовое поле регистра, если регистр позволяет запись
        template<typename T = void>
            requires Can_write<Access>
        [[gnu::always_inline]] inline static void write(Value_t value)
        {
            Register::write(value << bit_offset());
        }

        /// \brief Записывает значение в битовое поле регистра c использованием LDREX, STREX
        template<typename T = void>
            requires Can_write<Access> && Can_read<Access>
        [[gnu::always_inline]] inline static void set_atomic(Value_t value)
        {
            Register::bits_set_clear_atomic((value << bit_offset()), mask())
        }

        /// \brief Возвращает значение битового поля регистра
        template<typename T = void>
            requires Can_read<Access>
        [[gnu::always_inline]] inline static Register::Value_t Get()
        {
            return (Register::Get() & Mask()) >> offset;
        }
    };

    template<typename Field, typename Field::Size_t value>
    class Field_value : public Field
    {
        /// \brief Значение битового поля без смещения
        static consteval auto Value()
        {
            return value;
        }

        [[gnu::always_inline]] static void Set()
        {
            Field::Set(value);
        }

        [[gnu::always_inline]] inline static bool IsSet()
        {
            return Field::Get() == (value << Field::Offset);
        }
    };

    template<typename Field, typename Field::Size_t value>
    class Field_value : public Field
    {
        /// \brief Значение битового поля без смещения
        static consteval auto Value()
        {
            return value;
        }

        [[gnu::always_inline]] static void Set()
        {
            Field::Set(value);
        }

        [[gnu::always_inline]] inline static bool IsSet()
        {
            return Field::Get() == (value << Field::Offset);
        }
    };
}

#endif // FIELD_HPP
