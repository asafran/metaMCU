#ifndef FIELDS_HPP
#define FIELDS_HPP

#include "metautils.hpp"

template<typename... Vs>
    requires (sizeof...(Vs) != 0) && NoDuplicates<Vs...>
class Values
{
public:
    [[gnu::always_inline]] inline static void Set()
    {
        recursiveSet(deduplicatedRegisters, values);
    }

    [[gnu::always_inline]] inline static bool IsSet()
    {
        return recursiveIsSet(deduplicatedRegisters, values);
    }

private:

    template<typename... Xs>
        requires (IsFieldValue<Xs> && ...) && NoDuplicates<Xs...>
    static consteval auto deduplicateRegisters(meta_utils::TypeContainer<Xs...>)
    {
        return (... + meta_utils::TypeContainer<typename Xs::Register>());
    }

    static constexpr auto values = (meta_utils::TypeContainer() | ... | meta_utils::TypeContainer<Vs>());
    static constexpr auto deduplicatedRegisters = deduplicateRegisters(values);

    template<typename R, typename... Xs>
    [[gnu::always_inline]] inline static void applyToRegister(meta_utils::TypeContainer<R, Xs...>)
    {
        R::template SetFields<Xs...>();
    }

    template<typename R, typename... Xs>
    [[gnu::always_inline]] inline static bool checkRegister(meta_utils::TypeContainer<R, Xs...>)
    {
        return R::template IsSetFields<Xs...>();
    }

    template<typename R, typename... Rs, typename... Xs>
    [[gnu::always_inline]] inline static void recursiveSet(meta_utils::TypeContainer<R, Rs...>, meta_utils::TypeContainer<Xs...> values)
    {
        constexpr auto matched = (meta_utils::TypeContainer<R>() & ... & meta_utils::TypeContainer<Xs>());
        applyToRegister(matched);

        if constexpr (sizeof ...(Rs) != 0)
        {
            constexpr meta_utils::TypeContainer<Rs...> registers;
            recursiveSet(registers, values);
        }
    }

    template<typename R, typename ... Rs, typename... Xs>
    [[gnu::always_inline]] inline static bool recursiveIsSet(meta_utils::TypeContainer<R, Rs...>, meta_utils::TypeContainer<Xs...> values)
    {
        constexpr auto matched = (meta_utils::TypeContainer<R>() & ... & meta_utils::TypeContainer<Xs>());
        if(!checkRegister(matched))
            return false;
        else
        {
            if constexpr (sizeof ...(Rs) != 0)
            {
                constexpr meta_utils::TypeContainer<Rs...> registers;
                return recursiveIsSet(registers, values);
            }
            return true;
        }
    }
};

template<typename... Xs, typename V>
consteval auto operator|(meta_utils::TypeContainer<Xs...> lhs, meta_utils::TypeContainer<V> rhs)
{
    return meta_utils::TypeContainer<Xs..., V>();
}

template<typename... Xs, typename... Vs>
consteval auto operator|(meta_utils::TypeContainer<Xs...> lhs, meta_utils::TypeContainer<Values<Vs...>> rhs)
{
    return meta_utils::TypeContainer<Xs..., Vs...>();
}

#endif // FIELDS_HPP
