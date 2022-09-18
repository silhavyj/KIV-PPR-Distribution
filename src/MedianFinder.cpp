#include "MedianFinder.h"

namespace kiv_ppr
{
    template<class T>
    void Stream_Median_Finder<T>::Add_Value(T value)
    {
        m_left_half.push(value);

        if ((!m_left_half.empty() && !m_right_half.empty()) && (m_left_half.top() > m_right_half.top()))
        {
            m_right_half.push(m_left_half.top());
            m_left_half.pop();
        }
        if (m_left_half.size() > m_right_half.size() + 1)
        {
            m_right_half.push(m_left_half.top());
            m_left_half.pop();
        }
        if (m_left_half.size() + 1 < m_right_half.size())
        {
            m_left_half.push(m_right_half.top());
            m_right_half.pop();
        }
    }

    template<class T>
    [[nodiscard]] T Stream_Median_Finder<T>::Get_Median() const
    {
        if (m_left_half.size() > m_right_half.size())
        {
            return m_left_half.top();
        }
        else if (m_right_half.size() > m_left_half.size())
        {
            return m_right_half.top();
        }
        return (m_left_half.top() / 2.0) + (m_right_half.top() / 2.0);
    }

    template<class T>
    [[nodiscard]] bool Stream_Median_Finder<T>::Is_Empty() const
    {
        return m_left_half.empty() && m_right_half.empty();
    }

    template<class T>
    [[nodiscard]] T Stream_Median_Finder<T>::Pop_Value()
    {
        T value{};
        if (!m_left_half.empty())
        {
            value = m_left_half.top();
            m_left_half.pop();
        }
        else if (!m_right_half.empty())
        {
            value = m_right_half.top();
            m_right_half.pop();
        }
        return value;
    }

    template class Stream_Median_Finder<double>;
}