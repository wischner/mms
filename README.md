# mms

**mms** (Memory‑Mapped Streams) is a lightweight, header‑only C++17 library providing drop‑in replacements for `std::istream`/`std::ostream`, backed by POSIX memory‑mapping and featuring precise, real‑time line/column position tracking. Designed from the ground up for toolchains—compilers, assemblers, and linkers—**mms** delivers both throughput and accuracy in modern C++ codebases.

## Key Features

- **Blazing Performance**  
  Leverage `mmap` for zero‑copy file access and ultra‑fast reads—perfect when parsing megabyte‑scale source files in compiler and assembler pipelines.
- **Accurate Source Locations**  
  Built‑in tracker records line/column numbers (with bookmarks) as you consume or rewind characters, critical for error reporting in compilers and linkers.
- **UTF‑8 Aware**  
  Optional UTF‑8 decoding mode that correctly advances by multi‑byte sequences, ensuring correct handling of internationalized identifiers.
- **STL‑Style API**  
  Familiar names in the `mms` namespace:
  - `mms::file`
  - `mms::streambuf`
  - `mms::istream` / `mms::ostream` / `mms::iostream`
- **Zero Dependencies**  
  Pure C++17 (or later), no external libraries required—easy to integrate into existing build systems and toolchains.

## Quick Start

```cpp
#include <iostream>
#include <string>

#include <mms/istream>

int main() {
    // Create a UTF-8 aware memory-mapped input stream
    mms::istream in("source.cpp", /*utf8=*/true);
    std::string line;
    while (std::getline(in, line)) {
        std::cout << "[Line " << in.line()
                  << ", Col "  << in.column() << "] "
                  << line << "\n";
    }
    return 0;
}
```
