#include <mms/mms.h>
#include <ios> // for ios_base::failure

namespace mms
{

    streambuf::streambuf(const char *filename, bool utf8_mode)
        : file_(filename), tracker_(), utf8_mode_(utf8_mode), offset_(0)
    {
        if (!file_.is_open())
            throw std::ios_base::failure("Failed to open or map file");

        // Start with empty get area to force STL to call underflow()
        setg(nullptr, nullptr, nullptr);
    }

    streambuf::~streambuf() = default;

    const postrack &streambuf::tracker() const
    {
        return tracker_;
    }

    streambuf::int_type streambuf::underflow()
    {
        if (offset_ >= file_.size())
            return traits_type::eof();

        const char *base = file_.data();
        char *p = const_cast<char *>(&base[offset_]);

        setg(p, p, p + 1); // Prepare 1-char buffer

        return traits_type::to_int_type(*p);
    }

    streambuf::int_type streambuf::uflow()
    {
        int_type ch = underflow();
        if (traits_type::eq_int_type(ch, traits_type::eof()))
            return ch;

        tracker_.update_position(static_cast<unsigned char>(*gptr()));
        ++offset_;
        gbump(1); // move gptr() forward
        return ch;
    }

    std::streambuf::int_type streambuf::pbackfail(int_type ch)
    {
        // Check if we can un-read one character
        if (offset_ == 0)
            return traits_type::eof(); // can't put back before beginning

        --offset_; // move back one byte in file

        const char *base = file_.data();
        char c = base[offset_];

        // If a specific char was requested, verify match
        if (ch != traits_type::eof() &&
            c != traits_type::to_char_type(ch))
        {
            ++offset_; // undo
            return traits_type::eof();
        }

        // Adjust tracking
        tracker_.adjust_position_on_putback(c);

        // Re-create 1-char buffer at this position
        char *p = const_cast<char *>(&base[offset_]);
        setg(p, p, p + 1);

        return traits_type::to_int_type(c);
    }

    std::streamsize streambuf::xsgetn(char_type *s, std::streamsize n)
    {
        std::streamsize count = 0;

        while (count < n)
        {
            int_type ch = underflow();
            if (traits_type::eq_int_type(ch, traits_type::eof()))
                break;

            *s++ = traits_type::to_char_type(ch);
            tracker_.update_position(static_cast<unsigned char>(*gptr()));
            ++offset_;
            gbump(1);
            ++count;
        }

        return count;
    }

    streambuf::pos_type streambuf::seekoff(off_type off,
                                           std::ios_base::seekdir dir,
                                           std::ios_base::openmode which)
    {
        if (which != std::ios_base::in)
            return pos_type(off_type(-1));

        std::size_t new_pos;

        if (dir == std::ios_base::beg)
            new_pos = off;
        else if (dir == std::ios_base::cur)
            new_pos = offset_;
        else
            return pos_type(off_type(-1)); // only beg and cur supported

        if (new_pos > file_.size())
            return pos_type(off_type(-1));

        offset_ = new_pos;

        // Clear buffer so STL forces underflow again
        setg(nullptr, nullptr, nullptr);
        tracker_.set_position(offset_);

        return pos_type(offset_);
    }

    streambuf::pos_type streambuf::seekpos(pos_type sp,
                                           std::ios_base::openmode which)
    {
        return seekoff(off_type(sp), std::ios_base::beg, which);
    }

} // namespace mms
