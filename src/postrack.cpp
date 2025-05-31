// postrack.cpp
#include <mms/mms.h>

namespace mms
{

    postrack::postrack()
        : line_(1), column_(1), current_pos_(0) {}

    void postrack::update_position(int ch)
    {
        if (ch == '\n')
        {
            newline_positions_.insert(current_pos_);
            ++line_;
            column_ = 1;
        }
        else
        {
            ++column_;
        }
        ++current_pos_;
    }

    void postrack::adjust_position_on_putback(char c)
    {
        --current_pos_;

        if (c == '\n')
        {
            --line_;
            auto it = newline_positions_.lower_bound(current_pos_);
            if (it == newline_positions_.begin())
            {
                column_ = current_pos_ + 1;
            }
            else
            {
                --it;
                column_ = current_pos_ - *it;
            }
        }
        else
        {
            --column_;
        }
    }

    void postrack::set_position(std::size_t pos)
    {
        current_pos_ = pos;

        // Check if the position is bookmarked
        auto it = bookmarks_.find(pos);
        if (it != bookmarks_.end())
        {
            line_ = it->second.first;
            column_ = it->second.second;
        }
        else
        {
            // Recalculate line and column for non-bookmarked positions
            line_ = 1;
            column_ = 1;
            for (auto newline_pos : newline_positions_)
            {
                if (newline_pos < pos)
                {
                    ++line_;
                    column_ = pos - newline_pos; // Calculate the column correctly
                }
                else
                {
                    break;
                }
            }
        }
    }

    bookmark postrack::add_bookmark()
    {
        bookmark b(current_pos_, line_, column_);
        bookmarks_[current_pos_] = {line_, column_};
        return b;
    }

    void postrack::set_position(const bookmark &b)
    {
        current_pos_ = b.position();
        line_ = b.line();
        column_ = b.column();
    }

    int postrack::line() const
    {
        return line_;
    }

    int postrack::column() const
    {
        return column_;
    }

    const std::set<std::size_t> &postrack::newline_positions() const
    {
        return newline_positions_;
    }

    std::size_t postrack::position() const
    {
        return current_pos_;
    }

} // namespace mms