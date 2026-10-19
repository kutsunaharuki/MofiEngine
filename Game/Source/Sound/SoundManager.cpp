/**
 * @file SoundManager.cpp
 * @brief 音を一元管理するクラス
 */
#include "stdafx.h"
#include "SoundManager.h"
#include <algorithm>
#include <unordered_set>


namespace app
{
	namespace sound
	{
		/** シングルトンインスタンス */
		SoundManager* SoundManager::m_instance = nullptr;


		namespace
		{
			/**
			 * @brief 音の種類ごとの鳴らし方
			 */
			struct SoundTypeSetting
			{
				/** ゲームオブジェクトの名前(鳴っている音を探すときに使う) */
				const char* objectName;
				/** ループするか */
				bool isLoop;
				/** 同時に1つだけにするか */
				bool isExclusive;
			};


			/** 音の種類ごとの鳴らし方の表(EnSoundType の順番と合わせる) */
			constexpr SoundTypeSetting SOUND_TYPE_SETTING_TABLE[] =
			{
				{ "SoundManager_BGM",	true,	true },		// BGM
				{ "SoundManager_SE",	false,	false },	// SE
				{ "SoundManager_Voice",	false,	true },		// Voice
			};
			static_assert(sizeof(SOUND_TYPE_SETTING_TABLE) / sizeof(SOUND_TYPE_SETTING_TABLE[0]) == SOUND_TYPE_NUM,
				"SOUND_TYPE_SETTING_TABLE と SOUND_TYPE_NUM の数が合っていません。");


			/** 音量の初期値 */
			constexpr float DEFAULT_VOLUME = 1.0f;
			/** 読み込めなかったときの番号 */
			constexpr int INVALID_BANK_NUMBER = -1;


			/**
			 * @brief 音の種類ごとの鳴らし方を取得
			 * @param type 音の種類
			 * @return 鳴らし方
			 */
			const SoundTypeSetting& GetSetting(const EnSoundType type)
			{
				return SOUND_TYPE_SETTING_TABLE[static_cast<int>(type)];
			}
		}


		SoundManager::SoundManager()
			: m_masterVolume(DEFAULT_VOLUME)
			, m_nextBankNumber(0)
		{
			m_volumeList.fill(DEFAULT_VOLUME);
		}


		SoundManager::~SoundManager()
		{
			StopAll();
		}


		void SoundManager::Play(const EnSoundType type, const char* filePath)
		{
			const int bankNumber = LoadAndGetBankNumber(filePath);
			if (bankNumber == INVALID_BANK_NUMBER) return;

			const SoundTypeSetting& setting = GetSetting(type);
			std::vector<PlayingSound>& playingList = m_playingList[ToIndex(type)];
			RemoveFinishedSound(type);

			if (setting.isExclusive)
			{
				// 同じBGMが鳴っていれば、最初に戻さずにそのまま続ける
				if (setting.isLoop && !playingList.empty() && playingList.back().bankNumber == bankNumber)
				{
					return;
				}
				// 同時に1つだけなので、前の音を止める
				Stop(type);
			}

			SoundSource* source = NewGO<SoundSource>(0, setting.objectName);
			source->Init(bankNumber);
			source->SetVolume(ComputeVolume(type));
			source->Play(setting.isLoop);

			playingList.push_back({ source, bankNumber });
		}


		void SoundManager::Stop(const EnSoundType type)
		{
			RemoveFinishedSound(type);

			std::vector<PlayingSound>& playingList = m_playingList[ToIndex(type)];
			for (PlayingSound& sound : playingList)
			{
				sound.source->Stop();
				DeleteGO(sound.source);
			}
			playingList.clear();
		}


		void SoundManager::StopAll()
		{
			for (int i = 0; i < SOUND_TYPE_NUM; ++i)
			{
				Stop(static_cast<EnSoundType>(i));
			}
		}


		bool SoundManager::IsPlaying(const EnSoundType type)
		{
			RemoveFinishedSound(type);
			return !m_playingList[ToIndex(type)].empty();
		}


		bool SoundManager::Load(const char* filePath)
		{
			return LoadAndGetBankNumber(filePath) != INVALID_BANK_NUMBER;
		}


		void SoundManager::SetVolume(const EnSoundType type, const float volume)
		{
			m_volumeList[ToIndex(type)] = TClamp<float>::Clamp(volume, 0.0f, 1.0f);
			ApplyVolume(type);
		}


		void SoundManager::SetMasterVolume(const float volume)
		{
			m_masterVolume = TClamp<float>::Clamp(volume, 0.0f, 1.0f);
			for (int i = 0; i < SOUND_TYPE_NUM; ++i)
			{
				ApplyVolume(static_cast<EnSoundType>(i));
			}
		}


		int SoundManager::LoadAndGetBankNumber(const char* filePath)
		{
			if (filePath == nullptr) return INVALID_BANK_NUMBER;

			// 読み込み済みなら、その番号を使う
			auto it = m_bankNumberMap.find(filePath);
			if (it != m_bankNumberMap.end())
			{
				return it->second;
			}

			if (m_nextBankNumber >= MAXWAVEFILENUMBER)
			{
				K2_ASSERT(false, "WaveFileBank に登録できる数を超えました。");
				return INVALID_BANK_NUMBER;
			}

			// 読み込みに失敗した音は鳴らさない
			// ※ Init に失敗した SoundSource は、鳴らないまま破棄もされずに残ってしまうため、ここで確認する
			const int bankNumber = m_nextBankNumber;
			g_soundEngine->ResistWaveFileBank(bankNumber, filePath);
			if (!g_soundEngine->GetWaveFileBank().FindWaveFile(bankNumber))
			{
				return INVALID_BANK_NUMBER;
			}

			m_nextBankNumber++;
			m_bankNumberMap.emplace(filePath, bankNumber);
			return bankNumber;
		}


		float SoundManager::ComputeVolume(const EnSoundType type) const
		{
			return m_masterVolume * GetVolume(type);
		}


		void SoundManager::ApplyVolume(const EnSoundType type)
		{
			RemoveFinishedSound(type);

			const float volume = ComputeVolume(type);
			for (PlayingSound& sound : m_playingList[ToIndex(type)])
			{
				sound.source->SetVolume(volume);
			}
		}


		void SoundManager::RemoveFinishedSound(const EnSoundType type)
		{
			std::vector<PlayingSound>& playingList = m_playingList[ToIndex(type)];
			if (playingList.empty()) return;

			// ゲームオブジェクトの一覧に、まだ残っている音を集める
			// ※ ゲーム終了中は何も見つからないので、全て外れる
			std::unordered_set<SoundSource*> aliveSet;
			QueryGOs<SoundSource>(GetSetting(type).objectName, [&](SoundSource* source)
				{
					if (!source->IsDead())
					{
						aliveSet.insert(source);
					}
					return true;
				});

			playingList.erase(
				std::remove_if(playingList.begin(), playingList.end(),
					[&](const PlayingSound& sound) { return aliveSet.count(sound.source) == 0; }),
				playingList.end());
		}
	}
}
