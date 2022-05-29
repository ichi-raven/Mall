#ifndef MALL_UTILITY_TUPOINTER_HPP_
#define MALL_UTILITY_TUPOINTER_HPP_

#include <cassert>
#include <type_traits>

namespace mall
{
    template <typename T>
    class TUPointer
    {
    public:
        void create(T* ptr)
        {
            assert(ptr);
            mAddress = ptr;
        }

        T& get()
        {
            assert(mAddress);
            return *mAddress;
        }

        const T& get() const
        {
            assert(mAddress);
            return *mAddress;
        }

        T* data()
        {
            assert(mAddress);
            return mAddress;
        }

    private:
        T* mAddress;
    };
}  // namespace mall

#endif