#ifndef FIELD_HPP
#define FIELD_HPP

//Базовый класс для работы с битовыми полями регистров

template<typename Field, typename Field::Register::Type value>
struct FieldValueBase
{
    using Register = typename Field::Register;
    using RegType = typename Field::Register::Type;
    //Метод устанавливает значение битового поля, только в случае, если оно достпуно для записи
    [[gnu::always_inline]] static void Set()
    {
        Field::Set(value);
    }

    //Метод устанавливает значение битового поля используя LDREX, STREX, только если оно доступно для записи
    [[gnu::always_inline]] inline static void AtomicSet()
    {
        Field::AtomicSet(value);
    }

    //Метод устанавливает проверяет установлено ли значение битового поля
    [[gnu::always_inline]] inline static bool IsSet()
    {
        return Field::Get() == (value << Field::Offset);
    }
};

//Класс для работы с битовыми полями. Добавился тип Base, который необходим для того, чтобы проверить, что
//В регистре устанавливаются те битовые поля, которые допустимы для данного регистра
template<typename Field, typename Field::Register::Type value>
struct FieldValue : public FieldValueBase<Field, value>
{
    constexpr static auto Mask = static_cast<Field::Register::Type>(1U << Field::Size) - 1U;
    constexpr static auto Value = value;
    constexpr static auto Offset = Field::Offset;
    using Access = typename Field::Access;
};

#endif // FIELD_HPP
