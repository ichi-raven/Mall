#include "../../include/Mall/Engine/Input.hpp"

#include <Cutlass/Event.hpp>

#include <iostream>

namespace mall
{
    Input::Input(const std::shared_ptr<Cutlass::Context>& context)
        : mpContext(context)
    {
    }

    Input::~Input()
    {
        std::cerr << "Input system shut down\n";
    }

    bool Input::getKey(const Cutlass::Key key) const
    {
        // return !!mKeys[static_cast<size_t>(key)];
        return mpContext->getKey(key);
    }

    void Input::getCursorPos(double& x, double& y) const
    {
        mpContext->getMousePos(x, y);
    }

    void Input::update()
    {
        mpContext->updateInput();
        // for (const auto key : Cutlass::keyMap)
        //     mKeys[static_cast<uint32_t>(key)] += static_cast<uint32_t>(mpContext->getKey(key));
    }
}  // namespace mall