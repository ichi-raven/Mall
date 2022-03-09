#ifndef MALL_ENGINE_AUDIO_HPP_
#define MALL_ENGINE_AUDIO_HPP_

#include <portaudio.h>

#include "../Utility.hpp"

namespace mall
{
    struct SoundData;
    
    class Audio
    {
    public:
        Audio();

        ~Audio();

        void play(SoundData& soundData);

        void stop(SoundData& soundData);

        void reset(SoundData& soundData);
    };
}  // namespace mall

#endif