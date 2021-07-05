# constexpr-format
You are annoyed by the long compilation time when you use fmt lib. Now your code will take even longer to compile, and programs will run even faster.

## Preview features

- **[format-indexes]**
=======
#### [release branch](https://github.com/uselessgoddess/constexpr-format/tree/main) -> [Features after the final review - you can work]
#### [beta branch](https://github.com/uselessgoddess/constexpr-format/tree/beta) -> [Features after small review - you can work with preview features]
#### [alpha branch](https://github.com/uselessgoddess/constexpr-format/tree/alpha) -> [Only for the developing - non-stable features]

## Getting started
### Hello world
```cpp
#include <format.h> // include full library 

namespace fmt = constexpr_format; // short namespace alias 
using namespace fmt::literals; // for use ""_fmt literal

int main()
{
    std::cout << fmt::format("Hello {}!"_fmt, "world") << std::endl;
    fmt::print("Hello {}!\n"_fmt, "world");
    fmt::println("Hello {}!"_fmt, "world");
    
    // also can use
    // fmt::print("Hello world!\n"_fmt);
}
```

### Is this true at compile time?
**Yeah?!** 
```cpp
#include <format.h> // include full library 

namespace fmt = constexpr_format; // short namespace alias 
using namespace fmt::literals; // for use ""_fmt literal

int main()
{
    fmt::println("Hello {}!"_fmt, "world", "lol"); // static assertion failed
    fmt::println("Hello {}!"_fmt); // static assertion failed
    fmt::println("Hello { }!"_fmt, "world"); // static assertion failed
    fmt::println("Hello {!"_fmt, "world"); // static assertion failed

    fmt::println("Hello {1}!"_fmt, "hello world");
    fmt::println("{0} {0}!"_fmt, "Hello", "world");
    // and others
}
```

### Convert custom types to string

#### std::string operator
```cpp
#include <format.h>

namespace fmt = constexpr_format;
using namespace fmt::literals;

struct string_wrapper
{
    std::string str;
    explicit operator std::string() const noexcept
    {
        return str;
    }
};

int main()
{
    // it's javascript?
    fmt::print(R"("{}" + "{}" == "{}")"_fmt, string_wrapper{"1"}, string_wrapper{"2"}, string_wrapper{"12"});
}
```

#### operator<< for ostream
```cpp
#include <format.h>

template<typename T1, typename T2>
struct pair
{
    T1 first;
    T2 second;

    friend auto& operator<<(std::ostream& stream, pair<T1, T2> pair) {
        stream << "(" << pair.first << ", " << pair.second << ")";
        return stream;
    }
};

template<typename T1, typename T2>
pair(T1, T2) -> pair<T1, T2>;

int main()
{
    namespace fmt = constexpr_format;
    using namespace fmt::literals;

    fmt::print("{0} + {1} == {1} + {0} == {2}"_fmt, 
    /* #0 */   pair{1, 2}, 
    /* #1 */   pair{2, 1}, 
    /* #2 */   pair{3, 3});
}
```

#### temporary built in converting 
```cpp
#include <format.h>

namespace fmt = constexpr_format;
using namespace fmt::literals;

template<fmt::formatable T1, fmt::formatable T2>
struct fmt::to_string<std::pair<T1, T2>>
{
    std::string operator()(const std::pair<T1, T2> pair) const noexcept ( /* after creating noexcept integral conversion in new version */ )
    {
        return fmt::format("({}, {})"_fmt, pair.first, pair.second);        
    }
};
```

### Future conversion 
**Possible syntax**
```cpp
#include <format.h>

namespace fmt = constexpr_format;
using namespace fmt::literals;

template<typename T1, typename T2>
struct formatable_pair
{
    T1 first;
    T2 second;
    
    void fmt(fmt::formatter& fmt) noexcept
    {
        fmt.write_string(std::format("({}, {})"_fmt, first, second));
    }
};

template<fmt::formatable T1, fmt::formatable T2>
struct fmt::to_string<std::pair<T1, T2>>
{
    void fmt(fmt::formatter& fmt, const std::pair<T1, T2>& self) const noexcept 
    {
        fmt.write_string(std::format("({}, {})"_fmt, self.first, self.second));
    }
};
```

### Performance
#### [small benchmark with std::chrono (it is true)](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAM1QDsCBlZAQwBtMQBGAFlJvoCqAZ0wAFAB4gA5AAYppAFZdSrZrVDIApACYAQjt2kR7ZATx1KmWugDCqVgFcAtrRAA2UlfQAZPLUwAcs4ARpjEXACspAAOqEKE5rR2ji7uMXEJdL7%2BQU6h4ZxRxpimiQwEzMQEyc6uHsWldOWVBNmBIWGRRhVVNan1Pa1%2B7XmdhQCURqgOxMgcUjoAzH7IjlgA1JqLNsgIxHSoW9iaMgCCJ%2Bfay7SrDhs62ggEBNFCIAD078TMAO4AdMBCAgHMEHCJZnQCFYCH80E53mDMOwhEJgBgsCj3mhaEIoeJosQALQ0YhOZgEd5saIIZjvElkin08l/BAPC7sq4rNaYTbbKhOCloYiYFlHDnXW73PkCukkBmixbHM4XPwEdZkvwQcbsgDs%2BjO60N61x6BAIHiAC9MAB9NXAVioYJsa2YWK7XkAEXWnDAshkMl9/pkW315wNRs%2B6wAKgB5D0xkDrRHGgimkACzgATkzix14214cNJrNQm%2B1ggxZAZicmAgtAcrFY0QIxHzIfZhc2eouRt761ozBrQmizDm635aq2XuxuMw%2BOI1qZBHbnd7YL8wH7g8ww9HPInZtYhDCbCEK7Dpz7RsrlptatdqF2Qk963tjudD9256vRdTJZbG4psQG7nj2P7MA4RApi0L6Vrs%2By0KgJYAJ6zk41qrI%2BADWZqIT8WqgauRo/AgeDsOsECfggQiEoS4xdqGP59riwHqC%2BB4gEuEBsnqwY6l6mh6pwgkeg8i4CqQ6xlugWqSdJWraosjE/iJYFXhxBKqqwtDcdo2heFJO4NgQiaCfo/FiROkksRuim6OskaafQ6xQYhaqoM2eBOHgVodpe4GQag6wGVOKZpvBBwoWhGEOsgOEgHhBFKWpfYaaxBDabp2jVjyU55YqDEiU4Z56eJBCkClTFVXBeyRSA6AzOSiQYcwuJbDYNUIUh6ZkUeIjYugZ6KpR1jrIS0FVOMsLTPQCnfkaqnKp2ZmVdef7mj5d7BW61Evm%2BTqsC6O3zVeN4AWxNnqIR/lXhBUG4jBoWdXVQioVC6GYXFuGoPhdl%2BUxJFkTylE7TRdGFURp3nZuoUcVxPG6HxAlCapelyWoMmTFJGNzclkNdqJ%2BNpaqWUGcKQjGaZ3YWRENi0A81nQ39S03X2d1BSFixes9iFRe9MXYd9v0nalApmk5BBZTlnr5dghX8cVmi0/TaOrVVvY891DXfGYdAtW12ya2a3mNng/V0INRwjegY0TQQU1oA4s1tnjrOLRevYrfjN6bba22PrtoX7R%2Bx2u0xZ2sZul3ANdTHs3bsHrRFvPmm9mAfbF8WJczHs/oD5EgwHYP0V7rNMZGUakc%2BZvrMwwTTGqCA/esM54gSi5yuSZpLusPwkFhZ74xr63R%2BsCBItEYQi0x49NmEmx6E961ENa0cQPJLvKers%2BT8QC/6FzC/aEf08/jv88GEvaYr2vG85%2BrhqRv9D%2Bj6F5/EKfhru%2BHScN7yNjtSPmTIyrATJH3/oA1%2B2xAFsmVmyV2ata6BWCqNK%2BZpk7dVetFT6WcfpJS3qLEyIAJZSy8rlLmst5YekVnA1WQ91ZG3qo1XWtB9bLkNknWqKcTZ9RKBbIa2Bra2wepNaaTtJabxSu7d2UhJisGkBEeQrhZDyCQlIABi89DGmmLMXKVxODyBMio/MpAcIRBkCoaQ3AlHSDkKQNR8g3gWKMXISYcBYAwEQCgVAThohAzIBQCAcI/HsHCMATgMhtB8DIlCYgbwIDBFsfIYIfhKjIWkAY0gcIaz0BjLQVg6SVGkCwGSdQ7AknFLwMKUoAA3HcFS5wlEgvMOxqokQVKPMEb4xBkJ2CwBkwxwEnADMmPwRgLByk8D4JCYQYhJBFKUJwFQagNCXz0CoPAwQ3iwAHDWTiEk6nhCkDqf0sjJgeRYW8KQhIYxjRNFOLQmjdDaDsQ0FhlhrB9FcEsrwbRcj5GULEeILCvmAoyCwv5HQChGCRCUFhzRej2FqMoN5ZRBiQtGNCkR1QkWpCWdijFALOCTCEDouYXA5EKJsUUtR4gAAcbhCRuG4K%2BZAyBvQyD%2BMfCAuBCAkAXosJZ6w7C%2BP8QK4lwq1m6EMUkkxZiLHyKkNY0gwzzGkGUXYhxRgQDONlaQdxXiG7REguQSgwT/HKEwPgIg0KxlMDYBwKZYzZkSAqT8b40QRmWKkIo9VFS1Exkgsa9yVB1j0sZcy1l7KIlcooiKkJF99HjBlcYyY49mBYHCLJRVyrVUWI1ao6QjidWkBcWc0xIA1WKsWNSzVRbS2yspVIbQtbC1SBTa4yYhz4gWG4EAA%3D%3D)
