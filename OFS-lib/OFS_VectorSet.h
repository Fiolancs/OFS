#pragma once
#include <vector>
#include <algorithm>

template <typename T, typename Comparison = std::less<T>, typename Allocator = std::allocator<T>>
class vector_set : public std::vector<T, Allocator>
{
public:
    inline void sort() noexcept
    {
        std::sort(this->begin(), this->end());
    }

    template<typename... Args>
    inline bool emplace(Args&&... args)
    {
        T obj(std::forward<Args>(args)...);

        if (auto it = this->lower_bound(obj); it != this->end())
        {
            if (Comparison comp{}; !comp(*it, obj) && !comp(obj, *it))
            {
                this->insert(it, std::move(obj));
                return true;
            }
        }
        return false;
    }

    inline void emplace_back_unsorted(const T& a)
    {
        this->emplace_back(a);
    }

    inline auto find(const T& a) noexcept
    {
        auto it = lower_bound(a);
        if (it != this->end() && *it == a) {
            return it;
        }
        return this->end();
    }

    inline auto find(const T& a) const noexcept
    {
        auto it = lower_bound(a);
        if (it != this->end() && *it == a) {
            return it;
        }
        return this->end();
    }

    inline auto lower_bound(const T& a) noexcept
    {
        return std::lower_bound(this->begin(), this->end(), a, Comparison{});
    }

    inline auto lower_bound(const T& a) const noexcept
    {
        return std::lower_bound(this->begin(), this->end(), a, Comparison{});
    }

    inline auto upper_bound(const T& a) noexcept
    {
        return std::upper_bound(this->begin(), this->end(), a, Comparison{});
    }

    inline auto upper_bound(const T& a) const noexcept
    {
        return std::upper_bound(this->begin(), this->end(), a, Comparison{});
    }
};
