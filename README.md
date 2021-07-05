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
#### [small benchmark (it is true)](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAM1QDsCBlZAQwBtMQBGAFlJvoCqAZ0wAFAB4gA5AAYppAFZdSrZrVDIApACYAQjt2kR7ZATx1KmWugDCqVgFcAtrRABON6SvoAMnlqYAHLOAEaYxCAAzJykAA6oQoTmtHaOLu6e8YlmdH4BwU5hEdFGmCY5tAwEzMQEqc6uHqXlyVU1BHlBoeFRMULVtfXpTf3tnQVFvQCURqgOxMgcUjqR/siOWADUmpE2YbTICE41ANYA9PuHx8QnAHQIO9iaMgCCz2/aqwcbmNva2ggCARYkIQGczsRmAB3W7AQgIBwhBwiBZ0AhWAi3NBOM7IsqYIRCYAYLCEs5oWj9TDiWLEAC0NGIxwIZzYsQQzDOjOZXJIzPuOm072FrxWWCo/l%2BADEALIAFQA%2BgAJbAvAAi2AASgqAPKBHwATW2os%2Bax%2B212VCcLLQxEwAsiTxNX3WDi2OxsVpZ3OYmIejpFL1GZmQmwAbqg8OhNlLrRBLkdTiAQG10ToAGybYOYKbbADs%2Blem2Lm0ZmwgzAcRE2Cs2ICz1XRuc0BfeJfbDfQyf6xH8wAbvfUFrVpetyZ9BAggqnBZkLbVUxnuk486mLd0c7zC6XK63a9nq53q/Xm%2B3693C5Ph/Px4Pe6Pe7X/1Im0h1ggM1fanQH7XkULLwdiWCbXCcyZqqggSoAQOqxGYTh4AAXpgEA9n2f4Ae287Clu7y6NggQ2EqMovJqADSECxgQGGBsGeChhGUYxtadiUuiNLEDYCCYMgJzxlYVxJimjaYBmDa%2Bjm%2BaYSWZYVlWqA1nW4lNlJbZAZ23YEIO/ZoUOOwjl6458r6lHygqDBypqACSgQAOJTv8S6nouN57le96uZed5ngWF77hu16%2BbeAUeUFbneS5YULk%2B2ifm%2BP5xd%2Bv47NJQEgUJEFQTBcF4AhyGoVp6EpWpxbYaKuGvPhhHEaRFFUaxVIcVxPEnDRoqvHRDGRtGDXsbSUrGZO6U3MmqaidombZs2rZFiWtDME4BKxMwiyjgQw6bBSjW0gqE7FbNxbIn2mzzYtQjLathkgKwhDhGwQj7W8B2liQ5aVtWtb1lNqnPe2/RdiAuk6YVemRAZY4gBODnaE5gXLsFzkPl5IU%2BfD4Uo5FaPI4jnn%2BTjUVTDFu3Wi%2B8UfqTSWE496mbMNYEgJl0GwfBSEoUDbWASWZVvBVLxVURJHkRAvXUv1g0c3hBEC7VCokTZH4pVIMysNIACs8iuLI8ioNINgGAYWZzAsvwrJw8gENIciE6QYGqzIKjSNwGuW9r0jyKC9sW1rMxwLAMCICgqBOLEeDsGQFAQNiIdhyAwCcDI2h8KH6LEKC8Yu6QIT%2BDUACe0hm6Q2KLfQOq0Kweda6QWDHOo7AZ/gdqmHgYYEhn1I8VWSxyOQ9BlBnN0hJCxA53YWD5%2BbvZOOPMz8IwLB1zwfBosIYiSJXSgxKo6goPregqHgISgrAp0cHTpAtxEWkOLQrWkBAJ%2BQyTF/SHmMhWzMqA5XQoJSHSOqRJsOk/19JaD0AYbQMhixAKgQAdTYKwQBapAgal0AIWyUDWB00AawOCCA7TMGjO7fETcLAQG8EMVwMRvDjG6BEGIWQkh0AocoBhFQaGFB6H0YhFQ2iDHsA0ZQxgeI8IGB0SUExOFGFEcwvooj2GTE4DMIQRtFhcGVmrZ2lcdZSHEAADnTHSdM3BNjAGQKGeOtxtDllwIQV6psXx2GDqHcIfxoi5j1mAvQ5sXbW1tvbFWUgnakCnnbUgmtu7aPdiAT2Pi77%2BwgEgOYwIqzkEoFHZxdCvD4CIJk2eTA2AcEXrPFeEgM5QkhLEaeDspDqzCRnbROoqyxCrJsVAVBNh6IMUYkxZjNgWKscLIO0cXGmymN472MxuIEJ6OTAJQSQn23Ca7KQUSYkTJtiAUJATIiaIiW7UgXt37VO0Ls5Z4yjkX0SBYbgQA%3D%3D)
