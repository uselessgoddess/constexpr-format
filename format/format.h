#ifndef CONSTEXPR_FORMAT
#define CONSTEXPR_FORMAT

#include <array>
#include <sstream>
#include <utility>
#include <iostream>
#include <tuple>

namespace constexpr_format
{
    namespace details
    {
        namespace sta
        {
            // TODO maybe use concept constraint
            template <typename char_type, char_type... chars>
            struct string {
                static constexpr char value[] = { chars... };
            };
        }

        inline namespace constexpr_for
        {
            enum class control_flow
            {
                // Mmm... payed control flows
                $continue,
                $break,
            };

            template<std::size_t I, std::size_t N>
            constexpr auto for_each(auto f, std::pair<std::size_t, std::size_t> result)
            {
                if constexpr (I != N) {
                    using result_type = decltype(f(std::integral_constant<std::size_t, I>{}));
                    if constexpr (std::same_as<result_type, control_flow>) {
                        auto condition = f(std::integral_constant<std::size_t, I>{});
                        switch (condition) {
                            case control_flow::$break:
                                return for_each<N, N>(f, {I, N});
                            case control_flow::$continue:
                                return for_each<I + 1, N>(f, {I + 1, N});
                        }
                    } else {
                        f(std::integral_constant<std::size_t, I>{});
                        return for_each<I + 1, N>(f, {I + 1, N});
                    }
                } else {
                    return result;
                }
            }

            template<std::size_t N>
            constexpr auto for_constexpr(auto&& func)
            // FIXME: requires requires { func(std::integral_constant...}); }
            {
                return for_each<0, N>(std::forward<decltype(func)>(func), {0, N});
            }
        }

        namespace numerics
        {
            constexpr auto pow(std::integral auto base, std::integral auto power)
            {
                if (power == 0)
                    return decltype(base){1};

                auto default_base = base;
                power--;

                while (power--) {
                    base *= default_base;
                }

                return base;
            }

            constexpr auto isdigit(std::integral auto digit)
            {
                return digit >= '0' && digit <= '9';
            }

            constexpr auto issign(std::integral auto sign)
            {
                return sign == '-' || sign == '+';
            }

            constexpr auto todigit(std::integral auto digit)
            {
                return digit - '0';
            }

            constexpr auto size(std::integral auto number)
            {
                return (std::size_t)log10(number) + 1;
            }
        }

        constexpr auto parse_index(std::string_view slice)
        {
            std::size_t size = 0;
            std::size_t result = 0;
            std::size_t end = slice.find('}') - 1;

            if (end == std::string_view::npos) {
                return std::size_t(-1);
            }

            while (numerics::isdigit(slice[size])) {
                result += numerics::todigit(slice[size]) * numerics::pow(10, end - size);
                size++;
            }

            if (slice[size] != '}') {
                // TODO: maybe use Rust-style Result
                return std::size_t(-1);
            }

            return result;
        }

        constexpr auto only_indexes_or_only_without_indexes(std::string_view fmt)
        {
            bool indexes = false;
            bool without_indexes = false;

            for (std::size_t i = 0; i < fmt.size() - 1; i++) {
                if (fmt[i] == '{') {
                    auto parsed = parse_index(fmt.substr(i + 1));
                    if (parsed == std::size_t(-1)) {
                        without_indexes = true;
                    } else {
                        indexes = true;
                    }
                }
            }

            return indexes xor without_indexes;
        }

        constexpr auto match_aggregate_size(std::string_view slice)
        {
            auto result = slice.find('}') + 1;
            // TODO: maybe create constant std::size_t(-1)
            return (result != std::string_view::npos) ? result : std::size_t(-1);
        }

        template<std::size_t start, std::size_t count, typename string>
        constexpr auto format_array_positions()
        {
            constexpr std::string_view fmt = { string::value, sizeof(string::value) };
            constexpr auto slice = fmt.substr(start);

            using array_type = std::pair<std::size_t, std::size_t>;

            if constexpr (fmt.empty()) {
                return std::array<array_type, 0>{};
            }

            //static_assert(slice.find('{') == slice.find('}') - 1 /* ==> ignore */ || slice.find('{') == std::string_view::npos,
            //    "Format violation {}, syntax error");

            if constexpr (slice.find('{') != std::string_view::npos && start < fmt.size() - 1) {
                return format_array_positions<start + slice.find('{') + 2, count + 1, string>();
            } else {
                std::array<array_type, count> array{};

                static_assert(only_indexes_or_only_without_indexes(fmt),
                              "You either use indices or you don't use");

                std::size_t last = 0;
                for (std::size_t i = 0; i < fmt.size() - 1; i++) {
                    if (fmt[i] == '{') {
                        auto parsed = parse_index(fmt.substr(i + 1));
                        if (parsed == std::size_t(-1)) {
                            array[last] = {i, last};
                        } else {
                            array[last] = {i, parsed};
                        }
                        last++;
                    }
                }

                return array;
            }
        }

        template<std::size_t I, typename T, std::size_t N>
        constexpr auto get_array(const std::array<T, N>& array)
        {
            using get_type = std::decay_t<decltype(std::get<I>(array[0]))>;
            std::array<get_type, N> result;

            for (std::size_t i = 0; i < array.size(); i++) {
                result[i] = std::get<I>(array[i]);
            }

            return result;
        }

        template<typename string>
        constexpr auto format_array()
        {
            constexpr std::string_view fmt = { string::value, sizeof(string::value) };
            constexpr auto positions = format_array_positions<0, 0, string>();

            std::array<std::string_view, positions.size() + 1> array;

            if constexpr (positions.size() == 0) {
                return std::pair{std::array{fmt}, get_array<1>(positions)};
            }

            array[0] = fmt.substr(0, fmt.find('{'));
            for (std::size_t i = 0; i < positions.size() - 1; i++) {
                auto position = std::get<0>(positions[i]);
                auto next_position = std::get<0>(positions[i + 1]);
                auto distance = next_position - position;

                auto aggregate_size = match_aggregate_size(fmt.substr(position));
                array[i + 1] = fmt.substr(position + aggregate_size, distance - aggregate_size);
            }
            auto aggregate_size = match_aggregate_size(fmt.substr(fmt.rfind('{')));
            array[positions.size()] = fmt.substr(std::get<0>(positions.back()) + aggregate_size);

            return std::pair{array, get_array<1>(positions)};
        }

        template<typename T, std::size_t N>
        constexpr auto max_find(const std::array<T, N>& array)
        {
            T max{};

            for (auto&& element : array) {
                if (element > max)
                    max = element;
            }

            return max;
        }

        template<std::size_t size, std::size_t N>
        constexpr auto indexes_check(const std::array<std::size_t, N>& array)
        {
            std::array<std::size_t, size> map_parody{};

            for (auto&& item : array) {
                map_parody[item]++;
            }

            for (auto&& item : map_parody) {
                if (item == 0) {
                    return std::size_t(-1);
                }
            }

            return (size == 0) ? std::size_t(-1) : size;
        }
    }

    namespace literals
    {
        template <typename char_type, char_type... chars>
        constexpr auto operator""_fmt()
        {
            return details::sta::string<char_type, chars...>{};
        }
    }

    inline namespace string_conversion
    {
        template<typename Self>
        struct string_converter;

        template<typename Self>
        concept formatable = requires(Self self)
        {
            { string_converter<Self>{}(self) } -> std::same_as<std::string>;
        };

        template<typename Self>
        inline constexpr string_converter<Self> converter{};
    }

    namespace details
    {
        template<typename... Args>
        struct format_args
        {
            std::tuple<const Args&...> args;

            constexpr format_args(const Args&... args) : args(args...) {}

            // TODO: After create noexcept constexpr arithmetic conversion to string
            constexpr bool is_noexcept()
            {
                // TODO: maybe create #define
                auto [i, n] =
                constexpr_for::for_constexpr<sizeof...(Args)>([this] <std::size_t i> (std::integral_constant<std::size_t, i>) {
                    using arg_type = decltype(std::get<i>(args));
                    if constexpr (not noexcept(converter<arg_type>(std::declval<arg_type>()))) {
                        return control_flow::$break;
                    }
                });

                return i == n;
            }
        };

        template<typename string>
        auto combine(auto&&... args)
            noexcept(false /* format_args::is_noexcept */)
        {
            constexpr std::string_view fmt = { string::value, sizeof(string::value) };

            constexpr auto pair_array = format_array<string>();

            constexpr auto array = std::get<0>(pair_array);
            constexpr auto indexes = std::get<1>(pair_array);

            constexpr auto supposed_size = max_find(indexes) + 1;
            constexpr auto check_result = indexes_check<supposed_size>(indexes);

            if constexpr (indexes.size() == 0) {
                return (std::string)fmt;
            }

            static_assert(check_result != std::size_t(-1) || indexes.size() == 0,
                          "interaction error between position parameters and the number of arguments");

            static_assert(check_result == sizeof...(args) || indexes.size() == 0,
                          "interaction error between position parameters and the number of arguments");

            std::string result;
            for_constexpr<array.size() - 1>([&] <std::size_t i> (std::integral_constant<std::size_t, i>) {
                result += array[i];

                constexpr auto index = indexes[i];
                using raw_arg_type = decltype(std::get<index>(std::tie(args...)));
                using arg_type = std::decay_t<raw_arg_type>;
                result += constexpr_format::converter<arg_type>(std::get<index>(std::tie(args...)));
            });
            result += array.back();

            return result;
        }
    }

    template<typename char_type, char_type... chars>
    auto format(details::sta::string<char_type, chars...> fmt, formatable auto&&... args)
        noexcept(noexcept(details::combine<decltype(fmt)>(std::forward<decltype(args)>(args)...)))
    {
        return details::combine<decltype(fmt)>(std::forward<decltype(args)>(args)...);
    }

    template<typename char_type, char_type... chars>
    void print(std::basic_ostream<char_type>& stream, details::sta::string<char_type, chars...> fmt, formatable auto&&... args)
        noexcept(noexcept(format(fmt, std::forward<decltype(args)>(args)...)))
    {
        stream << format(fmt, std::forward<decltype(args)>(args)...);
    }

    template<typename char_type, char_type... chars>
    void print(details::sta::string<char_type, chars...> fmt, formatable auto&&... args)
        noexcept(noexcept(format(fmt, std::forward<decltype(args)>(args)...)))
    {
        std::cout << format(fmt, std::forward<decltype(args)>(args)...);
    }

    template<typename char_type, char_type... chars>
    void println(std::basic_ostream<char_type>& stream, details::sta::string<char_type, chars...> fmt, formatable auto&&... args)
        noexcept(noexcept(format(fmt, std::forward<decltype(args)>(args)...)))
    {
        stream << format(fmt, std::forward<decltype(args)>(args)...) << "\n";
    }

    template<typename char_type, char_type... chars>
    void println(details::sta::string<char_type, chars...> fmt, formatable auto&&... args)
        noexcept(noexcept(format(fmt, std::forward<decltype(args)>(args)...)))
    {
        std::cout << format(fmt, std::forward<decltype(args)>(args)...) << "\n";
    }
}




// CONVERTION OVERRIDE REGION

// ##################################
// ##################################
namespace constexpr_format::string_conversion
{
    template<typename Self>
    requires
        requires(Self self) { std::to_string(self); }
    struct string_converter<Self>
    {
        std::string operator()(Self self) const
        // TODO: create constexpr noexcept integral conversion
            noexcept(false)
        {
            return std::to_string(self);
        }
    };

    template<typename Self>
    requires
        std::convertible_to<Self, std::string>
    struct string_converter<Self>
    {
        std::string operator()(const Self& self) const
            noexcept(noexcept(static_cast<std::string>(self)))
        {
            return static_cast<std::string>(self);
        }
    };

    template<typename Self>
    requires
        requires(Self self, std::stringstream& stream) { stream << self; }
         and
        (not requires(Self self) { std::to_string(self); })
    struct string_converter<Self>
    {
        std::string operator()(const Self& self) const
            // TODO: create fully noexcept check
            noexcept(noexcept(std::declval<std::stringstream>() << self))
        {
            std::stringstream stream;
            stream << self;
            return stream.str();
        }
    };
}
// ##################################
// ##################################


#endif //CONSTEXPR_FORMAT
