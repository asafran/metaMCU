#ifndef FIELD_HPP
#define FIELD_HPP

#include <concepts>
#include <cstddef>

#include "register.hpp"

namespace metaMCU {

    /*!
     * \brief Обеспечивает безопасный доступ к битовым полям регистров
     * микроконтроллера
     * \warning Данный шаблонный класс разработан для работы вместе
     * со скриптом RegistersGenerator, подставляющим в шаблонные
     * параметры по информацию из svd файла и автоматически
     * генерирующим соответствующие заголовочные файлы. Инстанцирование этого
     * класса "вручную" должно производится только при крайней необходимости.
     * \tparam Reg Регистр поля
     * \tparam offset Смещение поля в бит
     * \tparam size Размер поля в бит
     * \tparam AccessMode Тип доступа (WriteMode, ReadMode или ReadWriteMode)
     */
    template<typename Register, size_t offset, size_t size, typename Access>
    struct RegisterField
    {
        /// \brief Тип из stdint.h соотвествествующий разряду регистра
        using Value_t = typename Register::Value_t;

        /// \brief Смещение битового поля в бит от 0
        static consteval auto BitOffset()
        {
            return offset;
        }

        /// \brief Размер битового поля в бит
        static consteval auto Size()
        {
            return size;
        }

        /// \brief Маска битового поля
        static consteval auto Mask()
        {
            return ((1 << size) - 1) << offset;
        }

        /// \brief Проверяет принадлежность поля данному регистру
        template<typename R>
        static consteval auto IsRegisterCompatible()
        {
            return std::derived_from<Register, R>;
        }

        /// \brief Записывает значение в битовое поле регистра, если регистр позволяет запись
        template<typename T = void>
            requires Can_read<Access> && Can_write<Access>
        [[gnu::always_inline]] inline static void Set(Register::Value_t value)
        {
            Register::SetResetBits((value << BitOffset()), Mask());
        }

        /// \brief Записывает значение в битовое поле регистра, если регистр позволяет запись
        template<typename T = void>
            requires Can_write<Access>
        [[gnu::always_inline]] inline static void SetDirectly(Register::Value_t value)
        {
            Register::Set(value << BitOffset());
        }

        /// \brief Записывает значение в битовое поле регистра c использованием LDREX, STREX
        template<typename T = void>
            requires Can_write<Access> && Can_read<Access>
        [[gnu::always_inline]] inline static void AtomicSet(RegType value)
        {
            atomic_utils<RegType, Reg::Address>::Set(Mask, value, offset);
        }

        /*!
             * \brief Инвертирует значение битового поля регистра,
             * если регистр позволяет и чтение, и запись
             */
        template<typename T = void>
            requires Can_write<Access> && Can_read<Access>
        [[gnu::always_inline]] inline static void Toggle()
        {
            Register::Toggle(Mask());
        }

        /// \brief Инвертирует значение битового поля регистра c использованием LDREX, STREX
        template<typename T = void>
            requires Can_write<Access> && Can_read<Access>
        [[gnu::always_inline]] inline static void AtomicToggle()
        {
            atomic_utils<RegType, Reg::Address>::Toggle((1 << size) - 1, offset);
        }

        /// \brief Возвращает значение битового поля регистра
        template<typename T = void>
            requires Can_read<Access>
        [[gnu::always_inline]] inline static Register::Value_t Get()
        {
            return (Register::Get() & Mask()) >> offset;
        }
    };

    struct FieldValueBase {};

    template<typename Field, typename Field::Size_t value>
    struct FieldValue : FieldValueBase
    {
        /// \brief Проверяет принадлежность поля данному регистру
        template<typename R>
        static consteval bool IsRegisterCompatible()
        {
            return Field::IsRegisterCompatible();
        }

        /// \brief Смещение битового поля в бит от 0
        static consteval auto BitOffset()
        {
            return Field::BitOffset();
        }

        /// \brief Размер битового поля в бит
        static consteval auto Size()
        {
            return Field::Size();
        }

        /// \brief Маска битового поля
        static consteval auto Mask()
        {
            return Field::Mask();
        }

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
