/// \file
/// \brief Memory‑Mapped Source (mms) — high‑performance C++ memory-mapped reader with line/column tracking.
///
/// Designed for modern C++ toolchains: compilers, assemblers, and linkers.
///
/// Copyright (c) 2024–2025 Tomaz Stih
/// SPDX-License-Identifier: MIT

#pragma once
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <cstddef>
#include <set>
#include <map>
#include <cstring>
#include <stdexcept>
#include <ios>
#include <streambuf>
#include <istream>
#include <codecvt>
#include <locale>
#include <iostream>
#include <optional>

namespace mms
{
    /// \brief Represents a bookmarked position in the input stream.
    ///
    /// Stores the file position, line number, and column number for later retrieval.
    class bookmark
    {
    public:
        /// \param pos    Absolute byte position in the file
        /// \param line   Line number at the bookmark
        /// \param column Column number at the bookmark
        bookmark(std::size_t pos, int line, int column);

        /// \return Stored byte position
        std::size_t position() const;

        /// \return Stored line number
        int line() const;

        /// \return Stored column number
        int column() const;

    private:
        std::size_t pos_;
        int line_;
        int column_;
    };

    /// \brief Tracks line and column numbers while reading a character stream.
    ///
    /// Supports updating positions on character consumption, putback,
    /// and allows bookmarks to speed up random seeks.
    class postrack
    {
    public:
        postrack();

        /// \brief Update tracker for a consumed character.
        void update_position(int ch);

        /// \brief Adjust tracker when a character is put back.
        void adjust_position_on_putback(char ch);

        /// \brief Set tracker to an arbitrary position (uses bookmarks or recalculation).
        void set_position(std::size_t pos);

        /// \brief Add a bookmark at the current position.
        bookmark add_bookmark();

        /// \brief Set position to bookmark.
        void set_position(const bookmark &b);

        /// \return Current line number
        int line() const;

        /// \return Current column number
        int column() const;

        /// \return Set of newline byte positions encountered
        const std::set<std::size_t> &newline_positions() const;

        /// \return Current position.
        std::size_t position() const;

    private:
        int line_;
        int column_;
        std::size_t current_pos_;
        std::set<std::size_t> newline_positions_;
        std::map<std::size_t, std::pair<int, int>> bookmarks_;
    };

    /// \brief RAII wrapper for POSIX memory-mapped file access.
    ///
    /// Opens a file, maps it into memory for read-only access,
    /// and unmaps/closes on destruction.
    class file
    {
    public:
        /// \param filename Path to the file to memory-map
        explicit file(const char *filename);
        ~file();

        /// \return Pointer to the mapped file data
        const char *data() const;

        /// \return Size of the mapped file in bytes
        std::size_t size() const;

        /// \return True if mapping succeeded
        bool is_open() const;

    private:
        int file_descriptor_;
        std::size_t file_size_;
        const char *mapped_data_;
    };

    /// \brief Provides a lightweight, stream-like interface for reading source files.
    ///
    /// The source class reads characters from a memory-mapped file while tracking
    /// the current position, line, and column. It supports peeking, putback,
    /// and bookmarking, making it suitable for use in lexical analyzers and compilers.
    ///
    /// This class does not inherit from std::istream to retain full control over
    /// behavior and efficiency.
    class source
    {
    public:
        /// \brief Open and prepare the source from a memory-mapped file.
        explicit source(const char *filename);

        /// \brief Read next character and advance position. Returns EOF on end.
        int get();

        /// \brief Peek next character without advancing. Returns EOF on end.
        int peek() const;

        /// \brief Put back the last character (1 level only).
        void putback();

        /// \brief Check if the source is still valid (i.e., not EOF).
        explicit operator bool() const;

        /// \brief Get current byte position in the file.
        std::size_t position() const;

        /// \brief Get current line number (1-based).
        int line() const;

        /// \brief Get current column number (1-based).
        int column() const;

        /// \brief Create a bookmark for the current location.
        bookmark mark() const;

        /// \brief Seek back to a previously stored bookmark.
        void seek(const bookmark &b);

        /// \brief Return raw pointer to mapped file data.
        const char *data() const;

        /// \brief Return total file size in bytes.
        std::size_t size() const;

    private:
        file file_;
        postrack tracker_;
    };

    /// \brief Extract the next word (non-whitespace token) from the stream.
    source &operator>>(source &s, std::string &out);

    /// \brief Extract an integer from the stream.
    source &operator>>(source &s, int &value);

    /// \brief Extract a single character from the stream.
    source &operator>>(source &s, char &ch);

} // namespace mms
