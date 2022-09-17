#pragma once

#include <queue>
#include <vector>

namespace kiv_ppr
{
    template<class T>
    class Stream_Median_Finder
    {
    public:
        void Add_Value(T value);
        [[nodiscard]] T Get_Median() const;
        [[nodiscard]] bool Is_Empty() const;
        [[nodiscard]] T Pop_Value();

    private:
        std::priority_queue<T> m_left_half;
        std::priority_queue<T, std::vector<T>, std::greater<T>> m_right_half;
    };
}