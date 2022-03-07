#ifndef MALL_COMPONENTDATA_SOUNDDATA_HPP_
#define MALL_COMPONENTDATA_SOUNDDATA_HPP_

#include <portaudio.h>

#include <MVECS/IComponentData.hpp>

#include "../Utility.hpp"

namespace mall
{
    struct SoundData : public mvecs::IComponentData
    {
        COMPONENT_DATA(SoundData)

        struct WaveFormat
        {
            unsigned short format_id;        //フォーマットID
            unsigned short channel;          //チャンネル数 monaural=1 , stereo=2
            unsigned long sampling_rate;     //１秒間のサンプル数，サンプリングレート(Hz),だいたいは44.1kHz
            unsigned long bytes_per_sec;     //１秒間のデータサイズ   44.1kHz 16bit ステレオ ならば44100×2×2 = 176400
            unsigned short block_size;       //１ブロックのサイズ．8bit:nomaural=1byte , 16bit:stereo=4byte
            unsigned short bits_per_sample;  //１サンプルのビット数 8bit or 16bit
        };

        template <typename T>
        T readData()
        {
            // PCMデータを読んで波形を返す
            if (loopFlag)
            {
                if (!loaded)
                    bufPos = 0;

                if (format.bits_per_sample == 8 && bufPos >= (int)data_8bit.size())
                    bufPos = 0;
                else if (format.bits_per_sample == 16 && bufPos >= (int)data_16bit.size())
                    bufPos = 0;
            }
            if (format.bits_per_sample == 8)
            {
                // 8bit
                return data_8bit[bufPos++];
            }
            else if (format.bits_per_sample == 16)
            {
                // 16bit
                return data_16bit[bufPos++];
            }

            return 0;
        }

        bool loaded;

        // void pointer
        TUPointer<PaStream*> ppStream;
        float volumeRate;

        WaveFormat format;                 //フォーマット情報
        TUArray<unsigned char> data_8bit;  //音データ(8bitの場合)
        TUArray<short> data_16bit;         //音データ(16bitの場合)

        int RIFFFileSize;  // RIFFヘッダから読んだファイルサイズ
        int PCMDataSize;   //実際のデータサイズ
        int bufPos;        //バッファポジション
        bool loopFlag;     //ループフラグ
        bool playFlag;     //再生中かどうか
        double playingDuration;
    };
}  // namespace mall

#endif