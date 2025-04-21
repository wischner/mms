![status.badge] [![language.badge]][language.url] [![standard.badge]][standard.url]

# mms

**mms** (Memory‑Mapped Streams) is a lightweight, header‑only C++17 library providing drop‑in replacements for `std::istream`/`std::ostream`, backed by POSIX memory‑mapping and featuring precise, real‑time line/column position tracking. Designed from the ground up for toolchains—compilers, assemblers, and linkers—**mms** delivers both throughput and accuracy in modern C++ codebases.

---

## Key Features

- **Blazing Performance**  
  Leverage `mmap` for zero‑copy file access and ultra‑fast reads—perfect when parsing megabyte‑scale source files.

- **Accurate Source Locations**  
  Built‑in tracker records line/column (with bookmarks) as you consume or rewind characters, critical for precise error reporting.

- **UTF‑8 Aware**  
  Optional UTF‑8 decoding mode correctly advances by multi‑byte sequences, ensuring correct handling of international identifiers.

- **STL‑Style API**  
  Familiar classes in the `mms` namespace:

  - `mms::file`
  - `mms::streambuf`
  - `mms::istream`

- **Zero Dependencies**  
  Pure C++17 (or later), no third‑party libraries—easy to integrate into any existing build system.

---

## Installation

Clone and build:

```bash
git clone https://github.com/youruser/mms.git
cd mms
mkdir build && cd build
cmake -G "Unix Makefiles" ..
make
```

This produces:

- Library: `bin/libmms.a` (or `.so` if built shared)
- Test executable: `bin/test-mms`
- Headers under `include/mms/`

To install globally:

```bash
sudo make install
```

---

## Usage

### Basic Example

```cpp
#include <iostream>
#include <string>
#include <mms/mms.h>

int main() {
    // Open a file in UTF-8 mode
    mms::istream in("example.txt", /* utf8_mode = */ true);
    if (!in) {
        std::cerr << "Failed to open file\n";
        return 1;
    }

    std::string line;
    while (std::getline(in, line)) {
        std::cout << "[Line " << in.line()
                  << ", Col "  << in.column() << "] "
                  << line << "\n";
    }
}
```

---

## Testing

We use GoogleTest. Build and run:

```bash
cd build
make             # builds library and tests
./bin/test-mms   # runs the suite
```

Integrate with CTest:

```bash
ctest --output-on-failure
```

---

## License

This project is licensed under the [MIT License](LICENSE).

[language.url]: https://isocpp.org/
[language.badge]: https://img.shields.io/badge/language-C++-blue.svg
[standard.url]: https://en.wikipedia.org/wiki/C%2B%2B#Standardization
[standard.badge]: https://img.shields.io/badge/C%2B%2B-20-blue.svg
[status.badge]: https://img.shields.io/badge/status-unstable-red.svg
