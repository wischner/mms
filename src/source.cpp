#include <mms/mms.h>

#include <cctype>
#include <stdexcept>
#include <string>

namespace mms
{

    source::source(const char *filename)
        : file_(filename) {}

    int source::get()
    {
        std::size_t pos = tracker_.position();
        if (pos >= file_.size())
            return EOF;

        char ch = file_.data()[pos];
        tracker_.update_position(ch);
        return static_cast<unsigned char>(ch);
    }

    int source::peek() const
    {
        std::size_t pos = tracker_.position();
        if (pos >= file_.size())
            return EOF;

        return static_cast<unsigned char>(file_.data()[pos]);
    }

    void source::putback()
    {
        if (tracker_.position() > 0)
        {
            char ch = file_.data()[tracker_.position() - 1];
            tracker_.adjust_position_on_putback(ch);
        }
    }

    source::operator bool() const
    {
        return tracker_.position() < file_.size();
    }

    std::size_t source::position() const
    {
        return tracker_.position();
    }

    int source::line() const
    {
        return tracker_.line();
    }

    int source::column() const
    {
        return tracker_.column();
    }

    bookmark source::mark() const
    {
        return bookmark(tracker_.position(), tracker_.line(), tracker_.column());
    }

    void source::seek(const bookmark &b)
    {
        tracker_.set_position(b);
    }

    const char *source::data() const
    {
        return file_.data();
    }

    std::size_t source::size() const
    {
        return file_.size();
    }

    // Stream-like operator>>

    source &operator>>(source &s, std::string &out)
    {
        out.clear();
        out.reserve(32); // Reasonable default to reduce reallocs

        // Skip leading whitespace
        int ch;
        while (s && std::isspace(ch = s.peek()))
            s.get();

        // Read word
        while (s && (ch = s.peek()) != EOF && !std::isspace(ch))
            out += static_cast<char>(s.get());

        return s;
    }

    source &operator>>(source &s, int &value)
    {
        value = 0;
        bool negative = false;

        // Skip whitespace
        int ch;
        while (s && std::isspace(ch = s.peek()))
            s.get();

        // Optional minus
        ch = s.peek();
        if (ch == '-')
        {
            negative = true;
            s.get();
        }

        // Read digits
        bool read_any = false;
        while (s && std::isdigit(ch = s.peek()))
        {
            read_any = true;
            value = value * 10 + (s.get() - '0');
        }

        if (!read_any)
            throw std::runtime_error("Invalid integer input");

        if (negative)
            value = -value;

        return s;
    }

    source &operator>>(source &s, char &ch)
    {
        // Skip leading whitespace
        int c;
        while (s && std::isspace(c = s.peek()))
            s.get();

        c = s.get();
        if (c == EOF)
            throw std::runtime_error("Unexpected EOF while reading char");

        ch = static_cast<char>(c);
        return s;
    }

} // namespace mms
