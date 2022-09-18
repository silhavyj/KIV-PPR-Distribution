#include "MedianFinder.h"

namespace kiv_ppr
{
    template<class T>
    void Stream_Median_Finder<T>::Add_Value(T value)
    {
        m_min_heap.push(value);

        if ((!m_min_heap.empty() && !m_max_heap.empty()) && (m_min_heap.top() > m_max_heap.top()))
        {
            m_max_heap.push(m_min_heap.top());
            m_min_heap.pop();
        }
        if (m_min_heap.size() > m_max_heap.size() + 1)
        {
            m_max_heap.push(m_min_heap.top());
            m_min_heap.pop();
        }
        if (m_min_heap.size() + 1 < m_max_heap.size())
        {
            m_min_heap.push(m_max_heap.top());
            m_max_heap.pop();
        }
    }

    template<class T>
    [[nodiscard]] T Stream_Median_Finder<T>::Get_Median() const
    {
        if (m_min_heap.size() > m_max_heap.size())
        {
            return m_min_heap.top();
        }
        else if (m_max_heap.size() > m_min_heap.size())
        {
            return m_max_heap.top();
        }
        return (m_min_heap.top() / 2.0) + (m_max_heap.top() / 2.0);
    }

    template<class T>
    [[nodiscard]] bool Stream_Median_Finder<T>::Is_Empty() const
    {
        return m_min_heap.empty() && m_max_heap.empty();
    }

    template<class T>
    [[nodiscard]] T Stream_Median_Finder<T>::Pop_Value()
    {
        T value{};
        if (!m_min_heap.empty())
        {
            value = m_min_heap.top();
            m_min_heap.pop();
        }
        else if (!m_max_heap.empty())
        {
            value = m_max_heap.top();
            m_max_heap.pop();
        }
        return value;
    }

    template class Stream_Median_Finder<double>;
}