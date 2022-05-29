#ifndef MALL_UTILITY_TUARRAY_HPP_
#define MALL_UTILITY_TUARRAY_HPP_

#include <cassert>
#include <cstdint>
#include <iterator>
#include <type_traits>

namespace mall
{
    template <typename T>
    class TUArray
    {
    public:
        using iterator         = T*;
        using const_iterator   = const T*;
        using reverse_iterator = std::reverse_iterator<iterator>;

        void create(T* ptr, std::size_t size)
        {
            assert(size >= 1);
            mAddress = ptr;
            mSize    = size;
        }

        T& operator[](std::size_t index)
        {
            assert(mAddress);
            assert(index < mSize);
            return mAddress[index];
        }

        const T& operator[](std::size_t index) const
        {
            assert(mAddress);
            assert(index < mSize);
            return mAddress[index];
        }

        T* data()
        {
            assert(mAddress);
            return mAddress;
        }

        std::size_t size() const
        {
            return mSize;
        }

        iterator begin() noexcept
        {
            assert(mAddress);
            return mAddress;
        }

        iterator end() noexcept
        {
            assert(mAddress);
            return mAddress + mSize;
        }

        const_iterator cbegin() const noexcept
        {
            assert(mAddress);
            return mAddress;
        }

        const_iterator cend() const noexcept
        {
            assert(mAddress);
            return mAddress + mSize;
        }

        reverse_iterator rbegin() noexcept
        {
            assert(mAddress);
            return reverse_iterator{mAddress + mSize};
        }

        reverse_iterator rend() noexcept
        {
            assert(mAddress);
            return reverse_iterator{mAddress};
        }

    private:
        T* mAddress;
        std::size_t mSize;
    };
}  // namespace mall

#endif