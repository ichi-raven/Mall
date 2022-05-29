#include "../../include/Mall/Engine/Audio.hpp"

#include <iostream>

#include "../../include/Mall/ComponentData/SoundData.hpp"

namespace mall
{
    Audio::Audio()
    {
        // Pa_Initialize();
        mSoloud.init();
    }

    Audio::~Audio()
    {
        // Pa_Terminate();
        mSoloud.stopAll();
        mSoloud.deinit();
    }

    void Audio::play(SoundData& sound)
    {
#ifdef _DEBUG
        if (!sound.loaded)
        {
            assert(!"did not loaded data!");
            return;
        }
#endif

        sound.playFlag = true;

        sound.runningHandle = mSoloud.play(sound.pWavData.get(), sound.volumeRate);
        mSoloud.setPause(sound.runningHandle, false);

        // auto err       = Pa_StartStream(sound.ppStream.get());
        // if (err != paNoError)
        // {  //エラー処理
        //     Pa_Terminate();
        //     std::cerr << "error : " << err << "\n";
        //     std::cerr << sound.format.format_id << "\n";
        //     std::cerr << sound.format.channel << "\n";
        //     std::cerr << sound.format.sampling_rate << "\n";
        //     std::cerr << sound.format.bytes_per_sec << "\n";
        //     std::cerr << sound.format.block_size << "\n";
        //     std::cerr << sound.format.bits_per_sample << "\n";
        //     assert(!"failed to open stream!");
        //     return;
        // }
    }

    void Audio::stop(SoundData& sound)
    {
        sound.playFlag = false;
        mSoloud.setPause(sound.runningHandle, true);

        // Pa_StopStream(sound.ppStream.get());
    }

    void Audio::reset(SoundData& sound)
    {
        sound.playingDuration = 0;
        mSoloud.stop(sound.runningHandle);
    }

}  // namespace mall