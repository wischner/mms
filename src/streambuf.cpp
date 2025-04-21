#include <mms/mms.h>
#include <ios> // for ios_base::failure

namespace mms
{

    streambuf::streambuf(const char *filename, bool utf8_mode)
        : file_(filename), tracker_(), utf8_mode_(utf8_mode)
    {
        if (file_.is_open())
        {
            // Initialize get area to the entire mapped file
            setg(const_cast<char *>(file_.data()),
                 const_cast<char *>(file_.data()),
                 const_cast<char *>(file_.data()) + file_.size());
        }
        else
        {
            throw std::ios_base::failure("Failed to open or map file");
        }
    }

    streambuf::~streambuf() = default;

    const postrack &streambuf::tracker() const
    {
        return tracker_;
    }

    std::streambuf::int_type streambuf::underflow()
    {
        if (gptr() == egptr())
        {
            return traits_type::eof();
        }
        // Peek without consuming
        return traits_type::to_int_type(*gptr());
    }

    std::streambuf::int_type streambuf::uflow()
    {
        // Extract next character
        auto c = underflow();
        if (traits_type::eq_int_type(c, traits_type::eof()))
        {
            return c;
        }
        // Consume and track
        char_type ch = traits_type::to_char_type(c);
        gbump(1);
        tracker_.update_position(static_cast<unsigned char>(ch));
        return c;
    }

    std::streambuf::int_type streambuf::pbackfail(int_type ch)
    {
        if (gptr() > eback())
        {
            // Move get pointer back
            gbump(-1);
            unsigned char uc = static_cast<unsigned char>(*gptr());
            // If caller specified a char, verify it matches
            if (ch != traits_type::eof() && uc != static_cast<unsigned char>(ch))
            {
                return traits_type::eof();
            }
            // Adjust tracker
            tracker_.adjust_position_on_putback(static_cast<char>(uc));
            return traits_type::to_int_type(uc);
        }
        return traits_type::eof();
    }

} // namespace mms
