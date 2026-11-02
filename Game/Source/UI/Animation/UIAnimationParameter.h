/**
 * @file UIAnimationParameter.h
 * @brief UIアニメーションの値を JSON から読み込む(ホットリロードあり)
 * @details 1つの JSON ファイルに、キーごとのアニメーションの値(最初の値・最後の値・秒数など)を書いておく。
 *          メニューごとに持つ(TitleEventMenu なら TitleSelectAnimation.json)。
 *          UIAnimationFactory を通して、UIAnimator のアニメーションに値を入れる
 *
 * JSON の書き方
 *   {
 *     "animations": [
 *       {
 *         "key": "Select",              // UIAnimationTypes.h の anim_key と同じ名前(ハッシュにして使う)
 *         "startValue": [1, 1, 1],      // float なら 1.0、ベクトルなら配列 か {"x":..} のオブジェクト
 *         "endValue": [1.15, 1.15, 1],
 *         "duration": 0.4,              // 秒
 *         "easing": "EaseInOut",        // Linear / EaseIn / EaseOut / EaseInOut / BounceOut / BounceIn
 *         "loop": "PingPong"            // Once / Loop / PingPong
 *       }
 *     ]
 *   }
 */
#pragma once
#include "Source/Util/CRC32.h"
#include "Source/Util/Curve.h"
#include "Source/Util/JsonConverter.h"
#include <ctime>
#include <string>
#include <unordered_map>


#ifdef K2_DEBUG
#define APP_ENABLE_UI_ANIMATION_HOTRELOAD
#endif


namespace app
{
	namespace ui
	{
		/**
		 * @brief 1つのアニメーションの値
		 * @details 値の型(float / Vector2 / Vector3 / Vector4)は、使うアニメーションのクラスで決まる。
		 *          ここでは全て Vector4 で持っておき、使うときに必要な分だけ取り出す(float なら x だけ)
		 */
		struct UIAnimationDef
		{
			/** キー(名前のハッシュ) */
			uint32_t key = 0;
			/** 最初の値 */
			Vector4 start = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
			/** 最後の値 */
			Vector4 end = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
			/** 秒数 */
			float duration = 0.3f;
			/** イージングタイプ */
			util::EasingType easingType = util::EasingType::Linear;
			/** ループモード */
			util::LoopMode loopMode = util::LoopMode::Once;
		};




		/*****************************************************/


		/**
		 * @brief UIアニメーションの値を、JSON ファイルから読み込むクラス
		 */
		class UIAnimationParameter : public Noncopyable
		{
		public:
			UIAnimationParameter()
				: m_lastWriteTime(0)
				, m_checkTimer(0.0f)
			{}

			~UIAnimationParameter()
			{}


			/**
			 * @brief JSON ファイルを読み込む
			 * @param filePath JSON ファイルのパス
			 * @return 読み込めたかどうか
			 */
			bool Load(const std::string& filePath)
			{
				m_filePath = filePath;
				m_lastWriteTime = util::JsonConverter::GetFileLastWriteTime(m_filePath);
				return Parse();
			}


			/**
			 * @brief ファイルが保存し直されていたら、読み込み直す
			 * @return 読み込み直したかどうか(true のときは、Factory で値を入れ直す)
			 * @details 毎フレーム呼ぶ。K2_DEBUG の構成でだけ、ファイルの保存を確認する
			 */
			bool ReloadIfModified()
			{
#ifdef APP_ENABLE_UI_ANIMATION_HOTRELOAD
				// 毎フレーム、ファイルを見に行くと重いので、少し間を空ける
				m_checkTimer -= g_gameTime->GetFrameDeltaTime();
				if (m_checkTimer > 0.0f) return false;
				m_checkTimer = CHECK_INTERVAL;

				const time_t writeTime = util::JsonConverter::GetFileLastWriteTime(m_filePath);
				if (writeTime == 0 || writeTime == m_lastWriteTime) return false;

				m_lastWriteTime = writeTime;
				return Parse();
#else
				return false;
#endif
			}


			/**
			 * @brief アニメーションの値を探す
			 * @param key アニメーションのキー
			 * @return 見つかった値(なければ nullptr)
			 */
			const UIAnimationDef* Find(const uint32_t key) const
			{
				auto it = m_defMap.find(key);
				if (it != m_defMap.end())
				{
					return &it->second;
				}
				return nullptr;
			}


		private:
			/**
			 * @brief JSON を読んで、アニメーションの値を作り直す
			 * @return 読み込めたかどうか(失敗したときは、今の値のまま)
			 */
			bool Parse()
			{
				nlohmann::json root;
				// 書きかけで書式が間違っているときも、今の値のまま動かす
				if (!util::JsonConverter::LoadFile(m_filePath, root)) return false;

				auto it = root.find("animations");
				if (it == root.end() || !it->is_array()) return false;

				// JSON から消したアニメーションが残らないように、全て作り直す
				m_defMap.clear();

				for (const auto& item : *it)
				{
					const std::string keyName = util::JsonConverter::ToString(item, "key");
					if (keyName.empty()) continue;

					UIAnimationDef def;
					def.key = Hash32(keyName.c_str());
					def.start = ToValue(item, "startValue");
					def.end = ToValue(item, "endValue");
					def.duration = util::JsonConverter::ToFloat(item, "duration", def.duration);
					def.easingType = ToEasingType(util::JsonConverter::ToString(item, "easing", "Linear"));
					def.loopMode = ToLoopMode(util::JsonConverter::ToString(item, "loop", "Once"));

					m_defMap[def.key] = def;
				}
				return true;
			}


			/**
			 * @brief 値を Vector4 で読む
			 * @details 数字1つなら x に入れる(float のアニメーション用)。配列 / オブジェクトなら、ある分だけ入れる
			 */
			static Vector4 ToValue(const nlohmann::json& item, const char* key)
			{
				const Vector4 zero(0.0f, 0.0f, 0.0f, 0.0f);

				auto it = item.find(key);
				if (it == item.end()) return zero;
				if (it->is_number()) return Vector4(it->get<float>(), 0.0f, 0.0f, 0.0f);

				return util::JsonConverter::ToVector4(item, key, zero);
			}


			/** 文字列をイージングタイプに変換(知らない文字列は Linear) */
			static util::EasingType ToEasingType(const std::string& name)
			{
				if (name == "EaseIn") return util::EasingType::EaseIn;
				if (name == "EaseOut") return util::EasingType::EaseOut;
				if (name == "EaseInOut") return util::EasingType::EaseInOut;
				if (name == "BounceOut") return util::EasingType::BounceOut;
				if (name == "BounceIn") return util::EasingType::BounceIn;
				return util::EasingType::Linear;
			}


			/** 文字列をループモードに変換(知らない文字列は Once) */
			static util::LoopMode ToLoopMode(const std::string& name)
			{
				if (name == "Loop") return util::LoopMode::Loop;
				if (name == "PingPong") return util::LoopMode::PingPong;
				return util::LoopMode::Once;
			}


		private:
			/** ファイルの更新を確認する間隔(秒) */
			static constexpr float CHECK_INTERVAL = 0.5f;


		private:
			/** JSON ファイルのパス */
			std::string m_filePath;
			/** キーごとのアニメーションの値 */
			std::unordered_map<uint32_t, UIAnimationDef> m_defMap;
			/** ファイルの最終更新日時 */
			time_t m_lastWriteTime;
			/** ファイルの更新を確認するまでの残り時間 */
			float m_checkTimer;
		};
	}
}
