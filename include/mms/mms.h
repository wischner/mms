/*
 * mms.h
 *
 * Memory‑Mapped Streams (mms) — high‑performance C++ memory‑mapped iostreams
 * with built‑in line and column tracking.
 *
 * NOTES:
 *   Designed for modern C++ toolchains: compilers, assemblers, and linkers.
 *
 * SPDX‑License‑Identifier: MIT
 * Copyright (c) 2025 Wischner Ltd.
 *
 * Tomaz Stih, April 2025
 */
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

namespace mms
{

    /**
     * @brief Represents a bookmarked position in the input stream.
     *
     * Stores the file position, line number, and column number for later retrieval.
     */
    class bookmark
    {
    public:
        /**
         * @param pos    Absolute byte position in the file
         * @param line   Line number at the bookmark
         * @param column Column number at the bookmark
         */
        bookmark(std::size_t pos, int line, int column);

        /** @return Stored byte position */
        std::size_t position() const;
        /** @return Stored line number */
        int line() const;
        /** @return Stored column number */
        int column() const;

    private:
        std::size_t pos_;
        int line_;
        int column_;
    };

    /**
     * @brief Tracks line and column numbers while reading a character stream.
     *
     * Supports updating positions on character consumption, putback,
     * and allows bookmarks to speed up random seeks.
     */
    class postrack
    {
    public:
        postrack();

        /** @brief Update tracker for a consumed character. */
        void update_position(int ch);
        /** @brief Adjust tracker when a character is put back. */
        void adjust_position_on_putback(char ch);
        /** @brief Set tracker to an arbitrary position (uses bookmarks or recalculation). */
        void set_position(std::size_t pos);

        /** @brief Add a bookmark at the current position. */
        bookmark add_bookmark();
        /** @brief Set position to bookmark. */
        void set_position(const bookmark &b);

        /** @return Current line number */
        int line() const;
        /** @return Current column number */
        int column() const;
        /** @return Set of newline byte positions encountered */
        const std::set<std::size_t> &newline_positions() const;

    private:
        int line_;
        int column_;
        std::size_t current_pos_;
        std::set<std::size_t> newline_positions_;
        std::map<std::size_t, std::pair<int, int>> bookmarks_;
    };

    /**
     * @brief RAII wrapper for POSIX memory-mapped file access.
     *
     * Opens a file, maps it into memory for read-only access,
     * and unmaps/closes on destruction.
     */
    class file
    {
    public:
        /** @param filename Path to the file to memory-map */
        explicit file(const char *filename);
        ~file();

        /** @return Pointer to the mapped file data */
        const char *data() const;
        /** @return Size of the mapped file in bytes */
        std::size_t size() const;
        /** @return True if mapping succeeded */
        bool is_open() const;

    private:
        int file_descriptor_;
        std::size_t file_size_;
        const char *mapped_data_;
    };

    /**
     * @brief A std::streambuf over a memory-mapped file with line/column tracking.
     *
     * Leverages POSIX mmap for zero-copy file access, updating line/column on
     * each character extraction and supporting putback operations.
     *
     * @param filename Path to the file to open and map
     * @param utf8_mode If true, enables UTF-8 decoding (future support)
     */
    class streambuf : public std::streambuf
    {
    public:
        /**
         * @brief Construct a memory-mapped stream buffer.
         * @param filename  Path to the file to open and map
         * @param utf8_mode If true, enables UTF-8 decoding
         */
        explicit streambuf(const char *filename, bool utf8_mode = false);

        /**
         * @brief Destructor unmaps and closes the file.
         */
        ~streambuf();

        /**
         * @brief Access the internal position tracker.
         * @return Reference to the postrack containing current line/column
         */
        const postrack &tracker() const;

    protected:
        /**
         * @brief Called to peek the next character without consuming it.
         * @return Next character or EOF if at end of file
         */
        int_type underflow() override;

        /**
         * @brief Called to extract the next character and advance the stream.
         * Updates the line and column tracker.
         * @return Extracted character or EOF if at end of file
         */
        int_type uflow() override;

        /**
         * @brief Called to put back one character into the get area.
         * Adjusts the tracker to reflect the position before extraction.
         * @param ch Character to put back (or EOF)
         * @return The put-back character or EOF on failure
         */
        int_type pbackfail(int_type ch = traits_type::eof()) override;

        std::streamsize xsgetn(char_type *s, std::streamsize n) override;
        pos_type seekoff(off_type off, std::ios_base::seekdir dir,
                         std::ios_base::openmode which) override;
        pos_type seekpos(pos_type sp,
                         std::ios_base::openmode which) override;

    private:
        file file_;        ///< Underlying memory-mapped file
        postrack tracker_; ///< Line and column tracker
        bool utf8_mode_;   ///< Reserved for future UTF-8 support
    };

    /**
     * @brief Input stream that reads from a streambuf and exposes position info.
     *
     * Drop-in replacement for std::istream with memory-mapped performance
     * and real-time line/column reporting via line() and column().
     */
    class istream : public std::istream
    {
    public:
        /**
         * @param filename  Path to file
         * @param utf8_mode Enable UTF-8 decoding
         */
        explicit istream(const char *filename, bool utf8_mode = false);

        /** @return Current line number in the stream */
        int line() const;
        /** @return Current column number in the stream */
        int column() const;

    private:
        streambuf buffer_;
    };

} // namespace mms
