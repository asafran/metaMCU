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
    concept Register_compatible_values = (Values::template IsRegisterCompatible<Register>() && ...);
    /// \brief Проверить возможность записи в данное поле регистра
    template<typename Value>
    concept Can_write_value = requires
    {
        Value::Set();
    };
    /// \brief Проверить возможность записи в данные поля регистра
    template<typename... Values>
    concept Can_write_values = (Can_write_value<Values> && ...);

    /*!
     * \brief Обеспечивает безопасный доступ к регистрам
     * микроконтроллера
     * \warning Данный шаблонный класс разработан для работы вместе
     * со скриптом RegistersGenerator, подставляющим в шаблонные
     * параметры информацию из svd файла и автоматически
     * генерирующим соответствующие заголовочные файлы. Инстанцирование этого
     * класса "вручную" должно производится только при крайней необходимости.
     * \tparam address Адрес регистра
     * \tparam size Размер регистра в бит
     * \tparam AccessT Тип доступа к регистру
     */
    template<size_t address, size_t size, typename Access>
    struct Register
    {
    private:
        template <uint32_t bits>
        struct Match_Value_t {};
        template<>
        struct Match_Value_t<8>
        {
            using Type = uint8_t;
        };
        template<>
        struct Match_Value_t<16>
        {
            using Type = uint16_t;
        };
        template<>
        struct Match_Value_t<32>
        {
            using Type = uint32_t;
        };
        template<>
        struct Match_Value_t<64>
        {
            using Type = uint64_t;
        };
    public:
        /// \brief Тип из stdint.h соотвествествующий разряду регистра
        using Value_t = typename Match_Value_t<size>::Type;

        /// \brief Адрес регистра
        static consteval auto Address()
        {
            return address;
        }

        /// \brief Записывает значение в регистр, если регистр позволяет запись
        template<typename T = void>
            requires Can_write<Access>
        [[gnu::always_inline]] inline static void Set(Value_t value)
        {
            *reinterpret_cast<volatile Value_t *>(address) = value;
        }

        /// \brief Возвращает значение регистра, если регистр позволяет чтение
        template<typename T = void>
            requires Can_read<Access>
        [[gnu::always_inline]] inline static Value_t Get()
        {
            return *reinterpret_cast<volatile Value_t *>(address);
        }

        /// \brief Устанавливает и сбрасывает данные биты, если регистр позволяет и чтение, и запись
        template<typename T = void>
            requires Can_write<Access> && Can_read<Access>
        [[gnu::always_inline]] inline static void SetResetBits(Value_t set, Value_t reset)
        {
            auto new_value = Get();
            new_value &= ~reset;
            new_value |= set;
            Set(new_value);
        }

        /// \brief Инвертирует значения бит по маске, если регистр позволяет и чтение, и запись
        template<typename T = void>
            requires Can_write<Access> && Can_read<Access>
        [[gnu::always_inline]] inline static void ToggleBits(Value_t mask = std::numeric_limits<Value_t>::max())
        {
            auto new_value = Get();
            new_value ^= mask;
            Set(new_value);
        }

        /*!
         * \brief Записывает значения данных битовых полей в регистр сохраняя значения других полей.
         * Регистр должен быть доступен для чтения и записи
         *
         * Метод принимает список значений полей регистра в виде пакета параметров шаблона,
         * считывает текущее значение из регистра, устанавливает или сбрасывает биты по маске данных битовых полей
         * и записывает итоговое значение в регистр. В списке могут быть только значения полей относящихся к данному
         * регистру, в противном случае код не будет скомпилирован.
         * \tparam Values Значения полей для записи
         */
        template<typename... Values>
            requires Register_compatible_values<Register<address, size, Access>, Values...>
        [[gnu::always_inline]] inline static void SetValues()
        {
            constexpr auto mask = getMask<Values...>();
            constexpr auto value = getValue<Values...>();
            SetResetBits(value, mask);
        }

        /*!
         * \brief Устанавливает значения данных битовых полей в регистр, сбрасывает остальные биты.
         * Регистр должен быть доступен для записи
         *
         * Метод принимает список значений полей регистра в виде пакета параметров шаблона, и устанавливает
         * данные поля. В списке могут быть только значения полей относящихся к данному
         * регистру, в противном случае код не будет скомпилирован.
         * \tparam Values Значения полей для записи
         */
        template<typename... Values>
            requires Register_compatible_values<Register<address, size, Access>, Values...>
        [[gnu::always_inline]] inline static void SetValuesDirectly()
        {
            Set(getValue<Values...>());
        }

        /*!
         * \brief Проверяет заданны или нет значения перечисленных полей регистра,
         * если регистр позволяет чтение
         *
         * Метод принимает список значений полей регистра в виде пакета параметров шаблона,
         * считывает текущее значение из регистра, накладывает маску переданных значений полей
         * и сравнивает значение в регистре с переданными значениями.
         * При полном соотвествии выводит истину. В списке могут быть только значения полей
         * относящихся к данному регистру, в противном случае код не будет скомпилирован.
         * \tparam Values Значения полей для записи
         */
        template<typename... Values>
            requires Register_compatible_values<Register<address, size, Access>, Values...>
        [[gnu::always_inline]] inline static bool IsSetFields()
        {
            auto register_value = Get();
            constexpr auto values_mask = calculateMask<Values...>();
            constexpr auto values_sum = accumulateValues<Values...>();
            return ((register_value & values_mask) == values_sum);
        }

    private:
        /// Возвращает маску для конктретного битового поля на этапе компиляции.
        template<typename Field>
        static consteval auto getIndividualMask()
        {
            Value_t result = Field::Mask << Field::Offset;
            return result;
        }

        /// Расчитывает общую маску для всего набора битовых полей на этапе компиляции.
        template<typename... Values>
        static consteval auto calculateMask()
        {
            const auto values = {getIndividualMask<Values>()...};
            Value_t result = 0;
            for (auto const v : values)
            {
                result |= v;
            }
            return result;
        }

        /// Возвращает значение для конктретного битового поля на этапе компиляции.
        template<typename T>
        static consteval auto getIndividualValue()
        {
            Value_t result = T::Value << T::Offset;
            return result;
        }

        /// Расчитывает значение которое нужно установить в регистре для всего набора битовых полей
        template<typename... Values>
        static consteval auto accumulateValues()
        {
            const auto values = {getIndividualValue<Values>()...};
            Value_t result = 0;
            for (const auto v : values)
            {
                result |= v;
            }
            return result;
        }
    };
}



#endif // REGISTER_HPP
