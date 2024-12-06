#ifndef REGISTER_HPP
#define REGISTER_HPP

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>

#include "metautils.hpp"
#include "atomic.hpp"

/*!
 * \file
 * \brief Файл с классами для работы с регистрами
 *
 * В этом заголовочнике содержатся статические классы для
 * реализации безопасного доступа к регистрам микроконтроллера.
 * А так же служебные типы и концепты.
 */

/// \brief Используется в Register как AccessMode тип для WO регистров
struct WriteOnlyT {};
/// \brief Используется в Register как AccessMode тип для RO регистров
struct ReadOnlyT {};
/// \brief Используется в Register как AccessMode тип для RW регистров
struct ReadWriteT : public WriteOnlyT, public ReadOnlyT {};
/// \brief Используется в Register для проверки возможности чтения
template <typename T>
concept CanRead = std::derived_from<T, ReadOnlyT>;
/// \brief Используется в Register для проверки возможности записи
template <typename T>
concept CanWrite = std::derived_from<T, WriteOnlyT>;

/*!
 * \brief Связывает размер регистра в бит (разряд) с соответсвующим
 * типом из stdint.h
 */
template <uint32_t size>
struct RegisterType {};
///\copydoc RegisterType
template<>
struct RegisterType<8>
{
    using Type = uint8_t;
};
///\copydoc RegisterType
template<>
struct RegisterType<16>
{
    using Type = uint16_t;
};
///\copydoc RegisterType
template<>
struct RegisterType<32>
{
    using Type = uint32_t;
};
///\copydoc RegisterType
template<>
struct RegisterType<64>
{
    using Type = uint64_t;
};

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
 * \tparam AccessMode Тип доступа к регистру
 */
template<uint32_t address, size_t size, typename AccessMode>
class Register
{
public:
    static constexpr auto Address = address;
    /// \brief Тип из stdint.h соотвествествующий разряду регистра
    using Type = typename RegisterType<size>::Type;

    /// \brief Записывает значение в регистр, если регистр позволяет запись
    template<typename T = void>
        requires CanWrite<AccessMode>
    [[gnu::always_inline]] inline static void Set(Type value)
    {
        *reinterpret_cast<Type *>(address) = value;
    }

    /*!
     * \brief Записывает значение в регистр атомарно инструкциями LDREX, STREX.
     * Работает только с RW регистрами
     */
    template<typename T = void>
        requires CanWrite<AccessMode> && CanRead<AccessMode>
    [[gnu::always_inline]] inline static void AtomicSet(Type value)
    {
        atomic_utils<Type, address>::Set(std::numeric_limits<Type>::max(), value, 0);
    }

    /// \brief Возвращает значение регистра, если регистр позволяет чтение
    template<typename T = void>
        requires CanRead<AccessMode>
    [[gnu::always_inline]] inline static Type Get()
    {
        return *reinterpret_cast<Type *>(address);
    }

    /*!
     * \brief Записывает значения заданных полей в регистр, если регистр позволяет запись
     *
     * Метод принимает список значений полей регистра в виде пакета параметров шаблона,
     * считывает текущее значение из регистра, рассчитывает итоговое значение с учетом всех полей
     * и записывает его в регистр. В списке могут быть только значения полей относящихся к данному
     * регистру, в противном случае код не будет скомпилирован.
     * \tparam Values Значения полей для записи
     */
    template<typename... Values>
        requires CanWrite<AccessMode> && (CompatibleField<Values, Register<address, size, AccessMode>> && ...)
    [[gnu::always_inline]] inline static void SetFields()
    {
        if constexpr (CanRead<AccessMode>)
        {
            Type newRegValue = *reinterpret_cast<Type *>(address);

            newRegValue &= ~getMask<Values...>();
            newRegValue |= getValue<Values...>();
            *reinterpret_cast<Type *>(address) = newRegValue;
        }
        else
            *reinterpret_cast<Type *>(address) = getValue<Values...>();
    }

    /*!
     * \brief То же, что и SetFields, но запись и чтение выполняется инструкциями LDREX, STREX
     * Работает только с RW регистрами
     * \tparam Values Значения полей для записи
     */
    template<typename... Values>
        requires CanWrite<AccessMode> && CanRead<AccessMode>
    [[gnu::always_inline]] inline static void AtomicSetFields()
    {
        atomic_utils<Type, address>::Set(getMask<Values...>(), getValue<Values...>(), 0);
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
        requires CanRead<AccessMode> && (CompatibleField<Values, Register<address, size, AccessMode>> && ...)
    [[gnu::always_inline]] inline static bool IsSetFields()
    {
        Type newRegValue = *reinterpret_cast<Type *>(address);
        return ((newRegValue & getMask<Values...>()) == getValue<Values...>());
    }

private:
    /// Возвращает маску для конктретного битового поля на этапе компиляции.
    template<typename T>
    static consteval auto getIndividualMask()
    {
        Type result = T::Mask << T::Offset;
        return result;
    }

    /// Расчитывает общую маску для всего набора битовых полей на этапе компиляции.
    template<typename... F>
    static consteval auto getMask()
    {
        const auto values = {getIndividualMask<F>()...};
        Type result = 0;
        for (auto const v: values)
        {
            result |= v;
        }
        return result;
    }

    /// Возвращает значение для конктретного битового поля на этапе компиляции.
    template<typename T>
    static consteval auto getIndividualValue()
    {
        Type result = T::Value << T::Offset;
        return result;
    }

    /// Расчитывает значение которое нужно установить в регистре для всего набора битовых полей
    template<typename... F>
    static consteval auto getValue()
    {
        const auto values = {getIndividualValue<F>()...};
        Type result = 0;
        for (const auto v: values)
        {
            result |= v;
        }
        return result;
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
 * \tparam Reg Регистр поля
 * \tparam offset Смещение поля в бит
 * \tparam size Размер поля в бит
 * \tparam AccessMode Тип доступа (WriteMode, ReadMode или ReadWriteMode)
 */
template<typename Reg, size_t offset, size_t size, typename AccessMode>
struct RegisterField
{
    /// \brief Register::Type
    using RegType = typename Reg::Type;
    /// \brief Связанный регистр
    using Register = Reg;
    /// \brief Смещение битового поля в бит от 0
    static constexpr RegType Offset = offset;
    /// \brief Размер битового поля в бит
    static constexpr RegType Size = size;
    /// \brief Тип доступа (WriteMode, ReadMode или ReadWriteMode)
    using Access = AccessMode;

    /// \brief Записывает значение в битовое поле регистра, если регистр позволяет запись
    template<typename T = void>
        requires CanWrite<AccessMode>
    [[gnu::always_inline]] inline static void Set(RegType value)
    {
        if constexpr (CanRead<AccessMode>)
        {
            RegType newRegValue = *reinterpret_cast<RegType *>(Reg::Address);

            newRegValue &= ~(((1 << size) - 1) << offset);
            newRegValue |= (value << offset);

            *reinterpret_cast<RegType *>(Reg::Address) = newRegValue;
        }
        else
            *reinterpret_cast<RegType *>(Reg::Address) = (value << offset);
    }

    /// \brief Записывает значение в битовое поле регистра c использованием LDREX, STREX
    template<typename T = void>
        requires CanWrite<AccessMode> && CanRead<AccessMode>
    [[gnu::always_inline]] inline static void AtomicSet(RegType value)
    {
        atomic_utils<RegType, Reg::Address>::Set((1 << size) - 1, value, offset);
    }

    /*!
     * \brief Инвертирует значение битового поля регистра,
     * если регистр позволяет и чтение, и запись
     */
    template<typename T = void>
        requires CanWrite<AccessMode> && CanRead<AccessMode>
    [[gnu::always_inline]] inline static void Toggle()
    {
        RegType newRegValue = *reinterpret_cast<RegType *>(Reg::Address);

        newRegValue ^= (((1 << size) - 1) << offset);

        *reinterpret_cast<RegType *>(Reg::Address) = newRegValue;
    }

    /// \brief Инвертирует значение битового поля регистра c использованием LDREX, STREX
    template<typename T = void>
        requires CanWrite<AccessMode> && CanRead<AccessMode>
    [[gnu::always_inline]] inline static void AtomicToggle()
    {
        atomic_utils<RegType, Reg::Address>::Toggle((1 << size) - 1, offset);
    }

    /// \brief Возвращает значение битового поля регистра
    template<typename T = void>
        requires CanRead<AccessMode>
    [[gnu::always_inline]] inline static RegType Get()
    {
        return ((*reinterpret_cast<RegType *>(Reg::Address)) & (((1 << size) - 1) << offset)) >> offset;
    }
};

#endif // REGISTER_HPP
