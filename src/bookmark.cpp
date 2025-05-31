/// \file
/// \brief Implementation of the `mms::bookmark` class.
///
/// The `bookmark` class represents a saved position within a source stream,
/// storing the byte offset, line, and column. Used for fast seek and recovery
/// during parsing operations.
///
/// Copyright (c) 2024–2025 Tomaz Stih
/// SPDX-License-Identifier: MIT

#include <mms/mms.h>

namespace mms
{

    bookmark::bookmark(std::size_t pos, int line, int column)
        : pos_(pos), line_(line), column_(column) {}

    std::size_t bookmark::position() const
    {
        return pos_;
    }

    int bookmark::line() const
    {
        return line_;
    }

    int bookmark::column() const
    {
        return column_;
    }

} // namespace mms
