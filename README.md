# constexpr-format
You are annoyed by the long compilation time when you use fmt lib. Now your code will take even longer to compile, and programs will run even faster.

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
struct fmt::string_converter<std::pair<T1, T2>>
{
    std::string operator()(const std::pair<T1, T2> pair) const noexcept ( /* after creating noexcept integral conversion in new version */ )
    {
        return fmt::format("({}, {})"_fmt, pair.first, pair.second);        
    }
};
```

### Performance
#### [small benchmark (it is true)](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAM1QDsCBlZAQwBtMQBGAFlJvoCqAZ0wAFAB4gA5AAYppAFZdSrZrVDIApACYAQjt2kR7ZATx1KmWugDCqVgFcAtrRAAObaSvoAMnlqYAHLOAEaYxCAAzKQADqhChOa0do4u7p5xCWZ0fgHBTmER0caYpkkMBMzEBCnOrh5GmCbZtBVVBLlBoeFRRpXVtWkNQv0d/l0FPZEAlEaoDsTIHFI6kf7IjlgA1JqRNmG0yAhOVQDWAPQHRyfEpwB0CLvYmjIAgi/v2muHm5g72toEAQCDEhCBzudiMwAO53YCEBAOEIOESLOgEKwEO5oJznFFNTBCITADBYInnNC0EaYcQxYgAWhoxBOBHOJ385yZLM5JBZDx02g%2BQreqywVHGWwAYgBZAAqAH0ABLYV4AEWwACV5QB5QI%2BACaOxFX3Wvx2eyoTlZaGImH5kWexu%2BGwc212NktrK5zCxjwdwteIx9eGQWwAbqg8OgpVaIFdjmcQCA2hidAA2LZBjHTHYAdn0by2Ra2TK2EGYDiIW3lWxAmcq2bzBdexdb9fQSZGxH8wHr3fU5tVJatSe9BAgAon%2BZkmlzqumU90nFn89nuhnc4Xa%2BXm7XG/ni53q%2BnK63%2BaP0z3p8Pp6vm5vm8vANIWyh1ggs1fanQH8vkWbbbFvGNynEmqqoIEqAENqMRmE4eAAF6YBAXY9n%2BAHFiuQpzh8ujYIENiKtKrwagA0hAkpWuhAZZiG4aRtGlE1HQ1K0sQNgIKUpxxlY1yJsmDaYOm9Y%2BpgOZrh8raluWlaoNWtYiY2EmFoB7adgQ/a9qhA67EOnqjryPoUXK8oMLKGoAJKBAA4hOAKLvuZ5LreJ73tuLnrte7m7q5B7ecenlueeHmOQ%2B85Pton5vj%2BUXfr%2BuwYYBwH8eBkHQbBeDwUhKEaWhCWSZhOEikVrx4QRREkeRTF2FSGJsRxXHUSKby0aGEZRlsNWsXSkqGeOyW3EmKZCdoGZZmJTYFUWtDME4hIxMwSzDgQg5bJS3XEPKY75SpRYoj2WwzXNQgLUt%2BkgKwhDhGwQg7e8u0liQZYVlWNZ1uN4n5lNbYjB2IDaVpuU6ZEekjiAY52doDlecFPmBX5sMBaF/mXr5TkXneCPOXDyOI9MEVbVaL7RR%2BxNxfjd2qVsA2gSAqVQTBcGIchANNS2hWqthnNvGVhHEWREBdXVPV9WzuH4XzlXysRVkfglUizKw0gAKzyK4sjyKg0g2AYBiZvMix/KsnDyAQ0hyPjpCgcrMgqNI3Bq%2BbmvSPIYK22bGuzHAsAwIgKCoE4MR4OwZAUBAOJByHIDAJwMieOKrAYsQYJxk7pAhP4VQAJ7SCbpA4nN9DarQrA5xrpBYCc6jsGn%2BC2mUYaEmnNKlJWyxyOQ9BNGnl0hFCxBZ3YWC56b3ZOCPsz8IwLA1zwfDosIYiSOXSicCoagaLregqHgIRgrAR0cDTpCNxEGkOLQpxe4f4NE6f0i5jIFuzKgGUsdI9LapEWz0r9ulaHoAw2gZBFl/qA1UgR1S6AENZUBrAaY/1YLBBAtpmDRldgSMoFgIDeEGK4Ne3hOj5EKMoTIiQ6B4NIfEchtAiHdAiGvEoWDWijEoYwzBLQ2jVDoZMBhfR2hsP4dw8YxCeicFmEIA2SwuCKxVo7cuWspDiDcGmekaZuBbGAMgUMsc7jaDLLgQgT1jYvjsIHYO4R/iRHEZ1LeuhTZO0ttbW2SspAO1IOPG2pB1Yd0Ua7EA7tHGkG9n7eYIJKzkEoBHCxfDMD4CIHwqeTA2AcDnlPReEg07QihDECedspCq28WnRR2pKwxErFsVAVAtjKNUeozR2iti6P0YLAOkdLHG2mA4z2sxOJoJ6KTVx7jPG2x8c7KQ/jAk9KtiALxrjIjyN8S7UgHtn75O0Is8Z3S1mnwSBYbgQA%3D%3D%3D)
