/**
 * @file SoundType.h
 * @brief 音の種類と、音のファイルのパス
 * @details 新しい音を増やすときは、種類ごとの namespace にパスを1行追加する。
 *          使用例 : SoundManager::GetInstance()->Play(EnSoundType::SE, se::DECIDE);
 */
#pragma once


namespace app
{
	namespace sound
	{
		/**
		 * @brief 音の種類
		 * @details 種類ごとに、鳴らし方と音量が分かれる
		 *          BGM   : ループする。同時に1つだけ(新しいBGMを鳴らすと、前のBGMは止まる)
		 *          SE    : 1回だけ鳴る。重ねて鳴らせる
		 *          Voice : 1回だけ鳴る。同時に1つだけ(新しいボイスを鳴らすと、前のボイスは止まる)
		 */
		enum class EnSoundType
		{
			BGM,
			SE,
			Voice
		};
		/** 音の種類の数(EnSoundType を増やしたら、ここも増やす) */
		constexpr int SOUND_TYPE_NUM = 3;




		/** BGMのファイル */
		namespace bgm
		{
			/** タイトル */
			constexpr const char* TITLE = "Assets/sound/BGM/AS_1695340_PopJazz.wav";
		}


		/** SEのファイル */
		namespace se
		{
			/** 決定 */
			constexpr const char* DECIDE = "Assets/sound/SE/AS_805112_Decide.wav";
			/** カメラのシャッター */
			constexpr const char* SHUTTER = "Assets/sound/SE/AS_1083733_Shutter.wav";
			/** 革靴の足音(1 歩分。元の長い足音から 1 歩だけ切り出した) */
			constexpr const char* FOOTSTEP = "Assets/sound/SE/AS_369829_Footstep.wav";
		}


		/** ボイスのファイル */
		namespace voice
		{
			/** 人ごみのざわめき(仮) */
			constexpr const char* CROWD = "Assets/sound/Voice/AS_1111642_Crowd.wav";
		}
	}
}
