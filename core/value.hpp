#ifndef VALUE_HPP
#define VALUE_HPP

namespace metaMCU::core {

    template<typename Field, Field::Value_t Value>
    class Field_value
    {
    public:
        [[gnu::always_inline]] static void set()
        {
            Field::set(Value);
        }

        [[gnu::always_inline]] inline static bool is_set()
        {
            return Field::get() == Value;
        }

    protected:
        static consteval auto value()
        {
            return Value;
        }

    public:
        template<typename F1, F1::Value_t V1, typename F2, F2::Value_t V2>
        consteval auto operator|(this Field_value<F1, V1> lhs, Field_value<F2, V2> rhs)
        {
            constexpr auto value = lhs.value() | rhs.value();
            constexpr auto field = F1() | F2();
            return Field_value<decltype(field), value>();
        }

        template<typename F, F::Value_t V>
        friend class Field_value;
    };
}

#endif // VALUE_HPP
