![status.badge] [![language.badge]][language.url] [![standard.badge]][standard.url]

# mms: memory-mapped source reader

`mms` is a small C++ library for reading files using memory mapping. It's designed for tools like compilers, preprocessors, and assemblers that need to read source files quickly while keeping track of line and column positions for diagnostics and error messages.

## Building and running tests

The library uses CMake and works with any recent C++20 or later compiler. To build:

```bash
git clone https://github.com/youruser/mms.git
cd mms
mkdir build && cd build
cmake -G "Unix Makefiles" ..
make
```

This builds:

- the static library (libmms.a) in bin/
- the test suite (test-mms) in bin/

To run the tests:

```bash
./bin/test-mms
```

The project uses GoogleTest for testing. If you want to re-run tests after changes, just rebuild with `make`.

If you're integrating mms into another CMake-based project, you can link against `libmms.a` and include headers from `include/`. There are no external dependencies — everything is self-contained.

## Using mms

Here's a basic example that demonstrates how to use mms::source to read a file word-by-word, track line and column numbers, and print out each token with its position. This kind of loop is typical in compilers, interpreters, and preprocessors.

```cpp
#include <mms/mms.h>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: scan <filename>\n";
        return 1;
    }

    try
    {
        mms::source src(argv[1]);

        std::string word;
        while (src >> word)
        {
            std::cout << "Line " << src.line()
                      << ", Column " << src.column()
                      << ": " << word << '\n';
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
```

Here’s what reading from a std::ifstream looks like:

```cpp
std::ifstream in("file.txt");
std::string word;
while (in >> word)
{
    std::cout << word << '\n';
}
```

And here’s the same with mms::source:

```cpp
mms::source src("file.txt");
std::string word;
while (src >> word)
{
    std::cout << src.line() << ":" << src.column() << " -> " << word << '\n';
}
```

## Why standard streams don't work here

Although standard C++ streams (`std::istream` and `std::streambuf`) seem like a natural fit, they cannot be used reliably for this purpose due to limitations in their internal design. The key issue is with how input characters are read.

In the standard library (e.g. libstdc++), most stream operations—including `std::getline`—internally call a function named `sbumpc()`. This function reads the next character and advances the internal cursor. Here is the relevant code from libstdc++:

```cpp
int_type
sbumpc()
{
  int_type __ret;
  if (__builtin_expect(this->gptr() < this->egptr(), true))
  {
    __ret = traits_type::to_int_type(*this->gptr());
    this->gbump(1);
  }
  else
    __ret = this->uflow();
  return __ret;
}
```

The problem is that `sbumpc()` is not a virtual function, so you can't override it in your subclass. Instead, you're supposed to override `uflow()`. But as the code above shows, `sbumpc()` will directly consume characters from the internal buffer `gptr() < egptr()` without calling `uflow()` unless the buffer is empty. That means your override of `uflow()` will not be called in most situations, and you have no reliable way to intercept every character being read.

You might think about disabling the internal buffer by calling `setg()` with a one-byte buffer, effectively forcing the stream to fall back to calling `uflow()` every time. But this breaks other standard functions, like `std::getline`, which assume a reasonably sized buffer and expect consistent behavior. It also comes with a performance cost unless you aggressively inline everything, because each read would go through the virtual `uflow()` function.

In short, the design of `std::streambuf` makes it impossible to cleanly intercept and track every read operation without resorting to fragile hacks. For this reason, mms does not subclass `std::istream` or `std::streambuf` in the final version. Instead, it provides a custom reader class that behaves like a stream but is implemented from scratch, giving you full control over reading, tracking, and integration with memory-mapped files.

If you're interested in exploring the internal logic of the standard C++ stream implementation, you can [find the source here](https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/include/std/streambuf).

## License

This project is licensed under the [MIT License](LICENSE).

## Copyright

© 2025 Tomaz Stih. All rights reserved.

[language.url]: https://isocpp.org/
[language.badge]: https://img.shields.io/badge/language-C++-blue.svg
[standard.url]: https://en.wikipedia.org/wiki/C%2B%2B#Standardization
[standard.badge]: https://img.shields.io/badge/C%2B%2B-20-blue.svg
[status.badge]: https://img.shields.io/badge/status-beta-orange.svg
