#include "../../include/Engine/Audio.hpp"

#include "../../include/ComponentData/SoundData.hpp"

#include <iostream>

namespace mall
{
    Audio::Audio()
    {
        Pa_Initialize();
    }

    Audio::~Audio()
    {
        Pa_Terminate();
    }

    void Audio::play(SoundData& sound)
    {
        if (!sound.loaded || !sound.ppStream.get())
        {
            assert(!"did not loaded data!");
            return;
        }

        sound.playFlag = true;
        auto err       = Pa_StartStream(sound.ppStream.get());
        if (err != paNoError)
        {  //エラー処理
            Pa_Terminate();
            std::cerr << "error : " << err << "\n";
            std::cerr << sound.format.format_id << "\n";
            std::cerr << sound.format.channel << "\n";
            std::cerr << sound.format.sampling_rate << "\n";
            std::cerr << sound.format.bytes_per_sec << "\n";
            std::cerr << sound.format.block_size << "\n";
            std::cerr << sound.format.bits_per_sample << "\n";
            assert(!"failed to open stream!");
            return;
        }
    }

    void Audio::stop(SoundData& sound)
    {
        sound.playFlag = false;
        Pa_StopStream(sound.ppStream.get());
    }
    
    void Audio::reset(SoundData& sound)
    {
        sound.bufPos = 0;
        stop(sound);
    }

}  // namespace mall