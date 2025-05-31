/// \file
/// \brief Implementation of the `mms::file` class, a memory-mapped file reader.
///
/// This file provides the definition of the `file` class, which maps files into memory
/// using POSIX APIs (`open`, `mmap`, `munmap`, etc.) for high-performance sequential reading.
/// It is designed for use in source-processing tools like compilers and assemblers.
///
/// Copyright (c) 2024–2025 Tomaz Stih
/// SPDX-License-Identifier: MIT

#include <fcntl.h>    // open
#include <unistd.h>   // close, lseek
#include <sys/mman.h> // mmap, munmap, madvise
#include <cstring>    // strerror
#include <stdexcept>  // std::ios_base::failure

#include <mms/mms.h>

namespace mms
{

    file::file(const char *filename)
        : file_descriptor_(-1), file_size_(0), mapped_data_(nullptr)
    {
        // Open the file
        file_descriptor_ = open(filename, O_RDONLY);
        if (file_descriptor_ == -1)
        {
            throw std::ios_base::failure("Error opening file: " + std::string(strerror(errno)));
        }

        // Get the file size
        file_size_ = lseek(file_descriptor_, 0, SEEK_END);
        if (file_size_ == static_cast<std::size_t>(-1))
        {
            close(file_descriptor_);
            throw std::ios_base::failure("Error determining file size: " + std::string(strerror(errno)));
        }

        // Memory-map the file if not empty
        if (file_size_ > 0)
        {
            mapped_data_ = static_cast<const char *>(
                mmap(nullptr, file_size_, PROT_READ, MAP_PRIVATE, file_descriptor_, 0));
            if (mapped_data_ == MAP_FAILED)
            {
                close(file_descriptor_);
                throw std::ios_base::failure("Error mapping file: " + std::string(strerror(errno)));
            }

            // Advise sequential access if supported
#ifdef POSIX_MADV_SEQUENTIAL
            posix_madvise(const_cast<char *>(mapped_data_), file_size_, POSIX_MADV_SEQUENTIAL);
#endif
        }
        else
        {
            mapped_data_ = nullptr; // Nothing to map
        }
    }

    file::~file()
    {
        if (mapped_data_ && mapped_data_ != MAP_FAILED)
        {
            munmap(const_cast<char *>(mapped_data_), file_size_);
        }
        if (file_descriptor_ != -1)
        {
            close(file_descriptor_);
        }
    }

    const char *file::data() const
    {
        return mapped_data_;
    }

    std::size_t file::size() const
    {
        return file_size_;
    }

    bool file::is_open() const
    {
        return file_descriptor_ != -1;
    }

} // namespace mms
