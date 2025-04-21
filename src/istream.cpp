// mmap_istream.cpp
#include <mms/mms.h>

namespace mms
{

    istream::istream(const char *filename, bool utf8_mode)
        : std::istream(&buffer_), buffer_(filename, utf8_mode) {}

    int istream::line() const
    {
        return buffer_.tracker().line();
    }

    int istream::column() const
    {
        return buffer_.tracker().column();
    }

} // namespace mms