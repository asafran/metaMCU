#ifndef METAUTILS_HPP
#define METAUTILS_HPP

#include <concepts>
#include <type_traits>

#include "field.hpp"

namespace meta_utils {
    template<typename... Xs> struct TypeContainer {};

    template<typename... Xs, typename V>
    consteval int countSameType(TypeContainer<Xs...> lhs, TypeContainer<V> rhs)
    {
        return (... + (std::is_same_v<Xs, V> ? 1 : 0));
    }

    template<typename... Xs, typename V>
    consteval auto operator+(TypeContainer<Xs...> lhs, TypeContainer<V> rhs)
    {
        if constexpr (countSameType(lhs, rhs))
            return lhs;
        else
            return TypeContainer<Xs..., V>();
    }

    template<typename R, typename... Xs, typename V>
    consteval auto operator&(TypeContainer<R, Xs...> lhs, TypeContainer<V> rhs)
    {
        if constexpr (std::is_same_v<R, typename V::Register>)
            return TypeContainer<R, Xs..., V>();
        else
            return lhs;
    }

    template<typename F, typename F::Register::Type value>
    consteval void isFieldValue(FieldValue<F, value>) {}
}

template<typename Value>
concept IsFieldValue = requires (Value v)
{
    meta_utils::isFieldValue(v);
};

template<typename Value, typename Register>
concept CompatibleField = IsFieldValue<Value> && std::derived_from<typename Value::Register, Register>;

template<typename... Xs>
concept NoDuplicates = (... && (meta_utils::countSameType(meta_utils::TypeContainer<Xs...>(), meta_utils::TypeContainer<Xs>()) == 1));

#endif // METAUTILS_HPP
