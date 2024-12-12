#ifndef FIELD_HPP
#define FIELD_HPP

#include <concepts>
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
    template<typename Register, size_t Offset, size_t Size, typename Access>
    class Field : private Register
    {
    public:
        using Value_t = typename Register::Value_t;

        /// \brief Смещение поля в бит от 0
        static consteval auto bit_offset()
        {
            return Offset;
        }

        /// \brief Размер поля в бит
        static consteval auto size()
        {
            return Size;
        }

        /// \brief Маска битового поля
        static consteval auto mask()
        {
            return ((1 << size()) - 1) << bit_offset();
        }

    protected:
        /// \brief Записывает значение в битовое поле регистра, если регистр позволяет запись
        template<typename Value>
            requires Can_read<Access> && Can_write<Access>
        [[gnu::always_inline]] inline static void set()
        {
            Register::template values_set<Value>();
        }

        /// \brief Записывает значение в битовое поле регистра, если регистр позволяет запись
        template<typename Value>
            requires Can_write<Access>
        [[gnu::always_inline]] inline static void write()
        {
            Register::template values_write<Value>();
        }

        /// \brief Возвращает значение битового поля регистра
        template<typename Value>
            requires Can_read<Access>
        [[gnu::always_inline]] inline static Register::Value_t is_set()
        {
            Register::template values_is_set<Value>();
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

        [[gnu::always_inline]] inline static bool is_set()
        {
            return Field::template is_set<Field_value>();
        }
    };
}

#endif // FIELD_HPP
