#ifndef REGISTER_HPP
#define REGISTER_HPP

#include <cstddef>
#include <cstdint>
#include <initializer_list>

#include "metautils.hpp"
#include "atomic.hpp"

//Режим доступа к регистрам
struct WriteMode {};
struct ReadMode {};
struct ReadWriteMode: public WriteMode, public ReadMode {};

template <typename T>
concept CanRead = std::derived_from<T, ReadMode>;

template <typename T>
concept CanWrite = std::derived_from<T, WriteMode>;

//Тип регистров в зависимости от размера
template <uint32_t size>
struct RegisterType {};

template<>
struct RegisterType<8>
{
    using Type = uint8_t;
};

template<>
struct RegisterType<16>
{
    using Type = uint16_t;
};

template<>
struct RegisterType<32>
{
    using Type = uint32_t;
};

template<>
struct RegisterType<64>
{
    using Type = uint64_t;
};

//Класс для работы с регистром, можно передавать список битовых полей для установки и проверки
template<uint32_t address, size_t size, typename AccessMode>
class Register
{
public:
    static constexpr auto Address = address;
    using Type = typename RegisterType<size>::Type;

    //Метод будет работать только для регистров, в которые можно записать значение
    template<typename T = void>
        requires CanWrite<AccessMode>
    [[gnu::always_inline]] inline static void Set(Type value)
    {
        *reinterpret_cast<Type *>(address) = value;
    }
    //Метод возвращает целое значение регистра, будет работать только для регистров, которые можно считать
    template<typename T = void>
        requires CanRead<AccessMode>
    [[gnu::always_inline]] inline static Type Get()
    {
        return *reinterpret_cast<Type *>(address);
    }

    //Метод устанавливает битовые поля используя LDREX, STREX, только если регистр может использоваться для записи
    template<typename... F>
        requires CanWrite<AccessMode> && CanRead<AccessMode>
    [[gnu::always_inline]] inline static void AtomicSetFields()
    {
        atomic_utils<Type, address>::Set(getMask<F...>(), getValue<F...>(), 0);
    }

    //Метод устанавливает битовые поля переданые через пачку, только если регистр может использоваться для записи
    template<typename... F>
        requires CanWrite<AccessMode> && (CompatibleField<F, Register<address, size, AccessMode>> && ...)
    [[gnu::always_inline]] inline static void SetFields()
    {
        if constexpr (CanRead<AccessMode>)
        {
            Type newRegValue = *reinterpret_cast<Type *>(address); //Сохраняем текущее значение регистра

            newRegValue &= ~getMask<F...>(); //Сбрасываем битовые поля, которые нужно будет установить
            newRegValue |= getValue<F...>(); //Устанавливаем новые значения битовых полей
            *reinterpret_cast<Type *>(address) = newRegValue; //Записываем в регистр новое значение
        }
        else
            *reinterpret_cast<Type *>(address) = getValue<F...>();
    }

    //Метод IsSet проверяет что все битовые поля из переданной пачки установлены
    template<typename... F>
        requires CanRead<AccessMode> && (CompatibleField<F, Register<address, size, AccessMode>> && ...)
    [[gnu::always_inline]] inline static bool IsSetFields()
    {
        Type newRegValue = *reinterpret_cast<Type *>(address);
        return ((newRegValue & getMask<F...>()) == getValue<F...>());
    }

private:
    //Вспомогательный метод, возвращает маску для конктретного битового поля на этапе компиляции.
    //Метод определен только в случае, если тип битового поля и базовый тип битового поля для регистра совпадают.
    //Т.е. нельзя устанвоить набор битов не соотвествующих набору для для данного регистра.
    template<typename T>
    static consteval auto getIndividualMask()
    {
        Type result = T::Mask << T::Offset;
        return result;
    }

    //Вспомогательный метод, расчитывает общую маску для всего набора битовых полей на этапе компиляции.
    template<typename... F>
    static consteval auto getMask()
    {
        const auto values = {getIndividualMask<F>()...};  //распаковываем набор битовых полей через список инициализации
        Type result = 0;
        for (auto const v: values)
        {
            result |= v;  //для каждого битового поля устанавливаем битовую маску
        }
        return result;
    }

    //Вспомогательный метод, возвращает значение для конктретного битового поля на этапе компиляции.
    //Метод определен только в случае, если тип битового поля и базовый тип битового поля для регистра совпадают.
    //Т.е. нельзя устанвоить набор битов не соотвествующих набору для для данного регистра.
    template<typename T>
    static consteval auto getIndividualValue()
    {
        Type result = T::Value << T::Offset;
        return result;
    }

    //Вспомогательный метод, расчитывает значение которое нужно установить в регистре для всего набора битовых полей
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

//Базовый класс для работы с битовыми полями регистров
template<typename Reg, size_t offset, size_t size, typename AccessMode>
struct RegisterField
{
    using RegType = typename Reg::Type;
    using Register = Reg;
    static constexpr RegType Offset = offset;
    static constexpr RegType Size = size;
    using Access = AccessMode;

    //Метод устанавливает значение битового поля, только в случае, если оно достпуно для записи
    template<typename T = void>
        requires CanWrite<AccessMode>
    [[gnu::always_inline]] inline static void Set(RegType value)
    {
        if constexpr (CanRead<AccessMode>)
        {
            RegType newRegValue = *reinterpret_cast<RegType *>(Reg::Address); //Сохраняем текущее значение регистра

            newRegValue &= ~(((1 << size) - 1) << offset); //Вначале нужно очистить старое значение битового поля
            newRegValue |= (value << offset); // Затем установить новое

            *reinterpret_cast<RegType *>(Reg::Address) = newRegValue;
        }
        else
            *reinterpret_cast<RegType *>(Reg::Address) = (value << offset);
    }

    //Метод устанавливает значение битового поля используя LDREX, STREX, только если оно доступно для записи
    template<typename T = void>
        requires CanWrite<AccessMode> && CanRead<AccessMode>
    [[gnu::always_inline]] inline static void AtomicSet(RegType value)
    {
        atomic_utils<RegType, Reg::Address>::Set((1 << size) - 1, value, offset);
    }

    template<typename T = void>
        requires CanWrite<AccessMode> && CanRead<AccessMode>
    [[gnu::always_inline]] inline static void Toggle()
    {
        RegType newRegValue = *reinterpret_cast<RegType *>(Reg::Address);

        newRegValue ^= (((1 << size) - 1) << offset);

        *reinterpret_cast<RegType *>(Reg::Address) = newRegValue;
    }

    template<typename T = void>
        requires CanWrite<AccessMode> && CanRead<AccessMode>
    [[gnu::always_inline]] inline static void AtomicToggle()
    {
        atomic_utils<RegType, Reg::Address>::Toggle((1 << size) - 1, offset);
    }

    //Метод устанавливает проверяет установлено ли значение битового поля
    template<typename T = void>
        requires CanRead<AccessMode>
    [[gnu::always_inline]] inline static RegType Get()
    {
        return ((*reinterpret_cast<RegType *>(Reg::Address)) & (((1 << size) - 1) << offset)) >> offset;
    }
};

#endif // REGISTER_HPP
