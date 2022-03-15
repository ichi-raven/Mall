#ifndef MALL_SYSTEM_AUDIOSYSTEM_HPP_
#define MALL_SYSTEM_AUDIOSYSTEM_HPP_

#include <MVECS/ISystem.hpp>

#include "../ComponentData/SoundData.hpp"
#include "../Engine.hpp"

namespace mall
{
    template <typename Key, typename Common, typename = std::is_base_of<Engine, Common>>
    class AudioSystem : public mvecs::ISystem<Key, Common>
    {
        SYSTEM(AudioSystem, Key, Common)

    public:
        virtual void onInit()
        {
            
        }

        virtual void onUpdate()
        {
            this->template forEach<mall::SoundData>(
                [&](mall::SoundData& sound)
                {
                    if (sound.playFlag)
                    {
                        sound.playingDuration += this->common().deltaTime;
                    }
                });
        }

        virtual void onEnd()
        {
        }

    };
}  // namespace mall

#endif