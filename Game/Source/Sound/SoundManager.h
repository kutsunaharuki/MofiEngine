/**
 * @file SoundManager.h
 * @brief 音を一元管理するクラス
 */
#pragma once
#include "Source/Sound/SoundType.h"
#include <array>
#include <string>
#include <unordered_map>
#include <vector>


namespace app
{
	namespace sound
	{
		/**
		 * @brief 音を一元管理するクラス
		 * @details 鳴らす音は、SoundType.h のパスと EnSoundType で指定する。
		 *          ファイルは、初めて鳴らすときに自動で読み込む(大きいファイルは Load で先に読み込んでおく)。
		 *          鳴っている音は種類ごとに覚えているので、音量の変更と Stop は、鳴っている途中の音にも効く
		 */
		class SoundManager : public Noncopyable
		{
		public:
			/**
			 * @brief インスタンスを作成する
			 */
			static void CreateInstance()
			{
				if (m_instance == nullptr)
				{
					m_instance = new SoundManager;
				}
			}

			/**
			 * @brief インスタンスを取得する
			 */
			static SoundManager* GetInstance()
			{
				return m_instance;
			}

			/**
			 * @brief インスタンスを破棄する
			 */
			static void DestroyInstance()
			{
				if (m_instance != nullptr)
				{
					delete m_instance;
					m_instance = nullptr;
				}
			}


		public:
			/**
			 * @brief 音を鳴らす
			 * @param type 音の種類(鳴らし方が変わる。SoundType.h を参照)
			 * @param filePath waveファイルのパス(SoundType.h の定数を使う)
			 * @details BGM は、同じファイルが鳴っていれば、最初に戻さずにそのまま続ける
			 */
			void Play(const EnSoundType type, const char* filePath);
			/**
			 * @brief その種類の音を、全て止める
			 * @param type 音の種類
			 */
			void Stop(const EnSoundType type);
			/**
			 * @brief 全ての音を止める
			 */
			void StopAll();
			/**
			 * @brief その種類の音が鳴っているか
			 * @param type 音の種類
			 * @return 1つでも鳴っていれば true
			 */
			bool IsPlaying(const EnSoundType type);

			/**
			 * @brief waveファイルを先に読み込んでおく
			 * @param filePath waveファイルのパス
			 * @return 読み込めたかどうか
			 * @details 読み込みはその場で行われるので、ロード中など、止まってもよいときに呼ぶ
			 */
			bool Load(const char* filePath);

			/**
			 * @brief 種類ごとの音量を設定する
			 * @param type 音の種類
			 * @param volume 音量(0.0〜1.0)
			 */
			void SetVolume(const EnSoundType type, const float volume);
			/**
			 * @brief 種類ごとの音量を取得する
			 * @param type 音の種類
			 * @return 音量(0.0〜1.0)
			 */
			float GetVolume(const EnSoundType type) const { return m_volumeList[ToIndex(type)]; }
			/**
			 * @brief 全体の音量(マスター音量)を設定する
			 * @param volume 音量(0.0〜1.0)
			 */
			void SetMasterVolume(const float volume);
			/**
			 * @brief 全体の音量(マスター音量)を取得する
			 * @return 音量(0.0〜1.0)
			 */
			float GetMasterVolume() const { return m_masterVolume; }


		private:
			SoundManager();
			~SoundManager();


		private:
			/**
			 * @brief 鳴っている音1つ分の情報
			 */
			struct PlayingSound
			{
				/** 音のソース */
				SoundSource* source;
				/** waveファイルの、WaveFileBank の番号 */
				int bankNumber;
			};


		private:
			/**
			 * @brief EnSoundType を配列の番号にする
			 * @param type 音の種類
			 * @return 配列の番号
			 */
			static int ToIndex(const EnSoundType type) { return static_cast<int>(type); }

			/**
			 * @brief waveファイルを読み込んで、WaveFileBank の番号を返す
			 * @param filePath waveファイルのパス
			 * @return WaveFileBank の番号(読み込めなければ -1)
			 */
			int LoadAndGetBankNumber(const char* filePath);
			/**
			 * @brief 実際に鳴らす音量を計算する
			 * @param type 音の種類
			 * @return マスター音量を掛けた音量
			 */
			float ComputeVolume(const EnSoundType type) const;
			/**
			 * @brief 鳴っている音の音量を、今の設定に合わせる
			 * @param type 音の種類
			 */
			void ApplyVolume(const EnSoundType type);
			/**
			 * @brief 鳴り終わって破棄された音を、リストから外す
			 * @param type 音の種類
			 * @details SE / ボイスは、鳴り終わるとエンジンが自動で破棄するので、触る前に必ず呼ぶ。
			 *          ゲーム終了中は全て外れるので、破棄済みの音には触らない
			 */
			void RemoveFinishedSound(const EnSoundType type);


		private:
			/** シングルトンインスタンス */
			static SoundManager* m_instance;
			/** 種類ごとの、鳴っている音のリスト */
			std::array<std::vector<PlayingSound>, SOUND_TYPE_NUM> m_playingList;
			/** 種類ごとの音量 */
			std::array<float, SOUND_TYPE_NUM> m_volumeList;
			/** 全体の音量 */
			float m_masterVolume;
			/** 読み込んだwaveファイルの、パスと WaveFileBank の番号 */
			std::unordered_map<std::string, int> m_bankNumberMap;
			/** 次に使う WaveFileBank の番号 */
			int m_nextBankNumber;
		};
	}
}
