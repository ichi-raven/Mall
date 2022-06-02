#ifndef MALL_INPUT_HPP_
#define MALL_INPUT_HPP_

#include <Cutlass/Context.hpp>
#include <memory>

namespace mall
{
    class Input
    {
    public:
        Input(const std::shared_ptr<Cutlass::Context>& context);

        ~Input();

        bool getKey(const Cutlass::Key key) const;

        void getCursorPos(double& x, double& y) const;

        // uint32_t getKeyFrame(const Cutlass::Key key);

        // bool getKeyDown(const Cutlass::Key key);

        // bool getKeyUp(const Cutlass::Key key);

        void update();

    private:
        // std::array<uint32_t, static_cast<size_t>(Cutlass::Key::LAST)> mKeys;
        std::shared_ptr<Cutlass::Context> mpContext;
    };
}  // namespace mall

#endif