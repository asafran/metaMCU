#ifndef REGISTER_HPP
#define REGISTER_HPP

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>

/*!
 * \file
 * \brief Файл с классами для работы с регистрами
 *
 * В этом заголовочнике содержатся статические классы для
 * реализации безопасного доступа к регистрам микроконтроллера.
 * А так же служебные типы и концепты.
 */

namespace metaMCU {

    /// \brief Используется в Register как AccessT тип для WO регистров
    struct Write_only_t {};
    /// \brief Используется в Register как AccessT тип для RO регистров
    struct Read_only_t {};
    /// \brief Используется в Register как AccessT тип для RW регистров
    struct Read_write_t : public Write_only_t, public Read_only_t {};
    /// \brief Используется в Register для проверки возможности чтения
    template <typename T>
    concept Can_read = std::derived_from<T, Read_only_t>;
    /// \brief Используется в Register для проверки возможности записи
    template <typename T>
    concept Can_write = std::derived_from<T, Write_only_t>;
    /// \brief Проверка значений полей на принадлежность данному регистру
    template<typename Register, typename... Values>
    concept Register_compatible_values = (std::is_base_of<Register, Values>() && ...);
    /// \brief Проверить возможность записи в данное поле регистра
    template<typename Value>
    concept Can_write_value = requires
    {
        Value::Set();
    };
    /// \brief Проверить возможность записи в данные поля регистра
    template<typename... Values>
    concept Can_write_values = (Can_write_value<Values> && ...);

    namespace core {

        /*!
         * \brief Обеспечивает безопасный доступ к регистрам
         * микроконтроллера
         * \warning Данный шаблонный класс разработан для работы вместе
         * со скриптом RegistersGenerator, подставляющим в шаблонные
         * параметры информацию из svd файла и автоматически
         * генерирующим соответствующие заголовочные файлы. Инстанцирование этого
         * класса "вручную" должно производится только при крайней необходимости.
         * \tparam addr Адрес регистра
         * \tparam Value Тип из stdint.h соотвествествующий разряду регистра
         * \tparam Access Тип доступа к регистру
         */
        template<size_t Address, typename Value, typename Access>
        class Register
        {
        public:
            consteval Register() {}

            using Value_t = Value;

            /// \brief Адрес регистра
            static consteval auto address()
            {
                return Address;
            }

            /// \brief Записывает значение в регистр, если регистр позволяет запись
            template<typename T = void>
                requires Can_write<Access>
            [[gnu::always_inline]] inline static void write(Value_t value)
            {
                *reinterpret_cast<volatile Value_t*>(Address) = value;
            }

            /// \brief Возвращает значение регистра, если регистр позволяет чтение
            template<typename T = void>
                requires Can_read<Access>
            [[gnu::always_inline]] inline static Value_t read()
            {
                return *reinterpret_cast<volatile Value_t*>(Address);
            }
        };
    }
}



#endif // REGISTER_HPP
