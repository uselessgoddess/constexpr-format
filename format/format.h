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
        template<typename F, std::size_t... Is>
        constexpr void for_each(F f, std::integer_sequence<std::size_t, Is...>)
        {
            if constexpr (sizeof...(Is) != 0) {
                auto _ = { f(std::integral_constant<std::size_t, Is>{})... };
            }
        }

        template<typename Callable>
        struct lambda
        {
            Callable func;

            template<std::size_t N>
            constexpr std::size_t operator()(std::integral_constant<std::size_t, N>) const
            {
                func(std::integral_constant<std::size_t, N>{});
                return 0;
            }
        };

        template<std::size_t N>
        constexpr void for_constexpr(auto&& func)
        // FIXME: requires requires { func(std::size_t{}); }
        {
            for_each(
                lambda<decltype(func)>{std::forward<decltype(func)>(func)},
                std::make_integer_sequence<std::size_t, N>{}
            );
        }

        std::string to_string(auto self)
        requires requires { std::to_string(self); }
        {
            return std::to_string(self);
        }

        std::string to_string(auto&& self)
            requires std::convertible_to<decltype(self), std::string>
        {
            return self;
        }

        std::string to_string(auto&& self)
            requires
                requires(std::stringstream& stream) { stream << self; }
                 and
                (not std::is_arithmetic_v<std::remove_reference_t<decltype(self)>>)
                 and
                (not std::convertible_to<decltype(self), std::string>)
        {
            std::stringstream stream;
            stream << self;
            return stream.str();
        }

        namespace sta
        {
            // TODO maybe use concept constraint
            template <typename char_type, char_type... chars>
            struct string {
                static constexpr char value[] = { chars... };
            };
        }

        template<std::size_t start, std::size_t count, typename string>
        constexpr auto format_array_positions()
        {
            constexpr std::string_view fmt = { string::value, sizeof(string::value) };
            constexpr auto slice = fmt.substr(start);

            if constexpr (fmt.empty()) {
                return std::array<std::size_t, 0>{};
            }

            if constexpr (slice.find('{') != std::string_view::npos && start < fmt.size() - 1) {
                return format_array_positions<slice.find('{') + 1 + start, count + 1, string>();
            } else {
                std::array<std::size_t, count> array{};

                std::size_t last = 0;
                for (std::size_t i = 0; i < fmt.size() - 1; i++) {
                    if (fmt[i] == '{') {
                        array[last] = i;
                        last++;
                    }
                }

                return array;
            }
        }

        template<typename string>
        constexpr auto format_array()
        {
            constexpr std::string_view fmt = { string::value, sizeof(string::value) };
            constexpr auto positions = format_array_positions<0, 0, string>();
            std::array<std::string_view, positions.size() + 1> array;

            if constexpr (positions.size() == 0) {
                return std::array{fmt};
            }

            array[0] = fmt.substr(0, fmt.find('{'));
            for (std::size_t i = 0; i < positions.size() - 1; i++) {
                auto distance = positions[i + 1] - positions[i];
                array[i + 1] = fmt.substr(positions[i] + 2, distance - 2);
            }
            array[positions.size()] = fmt.substr(positions.back() + 2);

            return array;
        }

        template<typename string>
        auto combine(auto&&... args)
        {
            constexpr auto array = format_array<string>();
            static_assert(array.size() - 1 == sizeof...(args),
                "The number of arguments does not match the template of format-string");

            std::string result;

            for_constexpr<array.size() - 1>([&] <std::size_t i> (std::integral_constant<std::size_t, i>) {
                result += array[i];
                result += details::to_string(std::get<i>(std::tie(args...)));
            });
            result += array.back();

            return result;
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

    template<typename Self>
    concept formatable = requires(Self self) { details::to_string(self); };

    template<typename char_type, char_type... chars>
    auto format(details::sta::string<char_type, chars...> fmt, formatable auto&&... args)
    {
        return constexpr_format::details::combine<decltype(fmt)>(std::forward<decltype(args)>(args)...);
    }

    template<typename char_type, char_type... chars>
    void print(std::basic_ostream<char_type>& out, details::sta::string<char_type, chars...> fmt, formatable auto&&... args)
    {
        out << format(fmt, std::forward<decltype(args)>(args)...);
    }

    template<typename char_type, char_type... chars>
    void print(std::ostream& stream, details::sta::string<char_type, chars...> fmt, formatable auto&&... args)
    {
        stream << format(fmt, std::forward<decltype(args)>(args)...);
    }

    template<typename char_type, char_type... chars>
    void print(details::sta::string<char_type, chars...> fmt, formatable auto&&... args)
    {
        std::cout << format(fmt, std::forward<decltype(args)>(args)...);
    }

    template<typename char_type, char_type... chars>
    void println(std::ostream& stream, details::sta::string<char_type, chars...> fmt, formatable auto&&... args)
    {
        stream << format(fmt, std::forward<decltype(args)>(args)...) << "\n";
    }

    template<typename char_type, char_type... chars>
    void println(details::sta::string<char_type, chars...> fmt, formatable auto&&... args)
    {
        std::cout << format(fmt, std::forward<decltype(args)>(args)...) << "\n";
    }
}

#endif //CONSTEXPR_FORMAT
