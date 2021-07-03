# constexpr-format
You are annoyed by the long compilation time when you use fmt lib. Now your code will take even longer to compile, and programs will run even faster.

#### [release branch](https://github.com/uselessgoddess/constexpr-format/tree/main) -> [Features after the final review - you can work]
#### [beta branch](https://github.com/uselessgoddess/constexpr-format/tree/beta) -> [Features after small review - you can work with preview features]
#### [alpha branch](https://github.com/uselessgoddess/constexpr-format/tree/alpha) -> [Only for the developing - non-stable features]

## Getting started
### Hello world
```cpp
#include "format.h" // include full library 

int main()
{
    namespace fmt = constexpr_format; // short namespace alias 
    using namespace fmt::literals; // for use ""_fmt literal

    std::cout << fmt::format("Hello {}!"_fmt, "world") << std::endl;
    fmt::print("Hello {}!\n"_fmt, "world");
    fmt::println("Hello {}!"_fmt, "world");
    
    // can use
    // fmt::print("Hello world!\n"_fmt);
}
```

### Is this true at compile time?
Yeah 
```cpp
#include "format.h" // include full library 

int main()
{
    namespace fmt = constexpr_format; // short namespace alias 
    using namespace fmt::literals; // for use ""_fmt literal

    fmt::print("Hello {}!\n"_fmt, "world", "lol"); // static assertation failed
    fmt::print("Hello {}!\n"_fmt); // static assertation failed
    
    // emm... in future ))
    fmt::print("Hello { }!\n"_fmt, "world"); // static assertation failed
    fmt::print("Hello {!\n"_fmt); // static assertation failed
    // and others 
}
```

### Convert custom types to string

#### std::string operator
```cpp
#include "format.h"

struct string_wrapper
{
    std::string str;

    explicit operator std::string() const
    {
        return str;
    }
};

int main()
{
    namespace fmt = constexpr_format;
    using namespace fmt::literals;

    // it's javascript?
    fmt::print(R"("{}" + "{}" == "{}")"_fmt, string_wrapper{"1"}, string_wrapper{"2"}, string_wrapper{"12"});
}
```

#### operator<< for ostream
```cpp
#include "format.h"

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

    fmt::print("{} + {} == {}"_fmt, pair{1, 2}, pair{2, 1}, pair{3, 3});
}
```

#### future converting 
```cpp
#include "format.h"

namespace fmt = constexpr_format;
using namespace fmt::literals;

template<typename T1, typename T2>
struct fmt::to_string<std::pair<T1, T2>>
{
    std::string operator()(const std::pair<T1, T2> pair) const
    {
        return fmt::format("({}, {})", pair.first, pair.second);        
    }
};
```
