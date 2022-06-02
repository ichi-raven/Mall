#ifndef MALL_ENGINE_AUDIO_HPP_
#define MALL_ENGINE_AUDIO_HPP_

#include <portaudio.h>

#include "../Utility.hpp"

//#ifdef _WIN32
//#define WITH_XAUDIO2
//#endif

#include <Mall/ThirdParty/SoLoud/soloud.h>

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
    
    private:
        SoLoud::Soloud mSoloud;
    };
}  // namespace mall

#endif