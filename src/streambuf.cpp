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

    std::streamsize streambuf::xsgetn(char_type *s, std::streamsize n)
    {
        std::streamsize count = 0;
        while (count < n && gptr() < egptr())
        {
            char c = *gptr();
            tracker_.update_position(static_cast<unsigned char>(c));
            *s++ = c;
            gbump(1);
            ++count;
        }
        return count;
    }

    streambuf::pos_type streambuf::seekoff(off_type off,
                                           std::ios_base::seekdir dir,
                                           std::ios_base::openmode which)
    {
        if (dir != std::ios_base::beg || which != std::ios_base::in)
            return pos_type(off_type(-1));

        const char *base = file_.data();
        const char *end = base + file_.size();
        const char *target = base + off;

        if (target < base || target > end)
            return pos_type(off_type(-1));

        setg(const_cast<char *>(base),
             const_cast<char *>(target),
             const_cast<char *>(end));

        tracker_.set_position(off);
        return pos_type(off);
    }

    streambuf::pos_type streambuf::seekpos(pos_type sp,
                                           std::ios_base::openmode which)
    {
        return seekoff(sp, std::ios_base::beg, which);
    }

} // namespace mms
