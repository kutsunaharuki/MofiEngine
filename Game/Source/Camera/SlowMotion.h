/**
 * @file SlowMotion.h
 * @brief ファインダーを覗いている間の、スローモーション
 */
#pragma once
#include "Source/Util/ParameterManager.h"
#include "Source/Util/WorldTime.h"


namespace app
{
	namespace camera
	{
		/**
		 * @brief スローモーションの調整パラメーター
		 * @details JSON : Assets/parameter/Camera/SlowMotion.json
		 */
		struct SlowMotionParameter
		{
			/** 遅くした時の世界の速さ(1.0 = 普通) */
			float timeScale;
			/** 普通の速さ ⇔ 遅い速さ、に変わるのにかける時間(本当の秒) */
			float blendTime;
			/** 続けて遅くできる時間(本当の秒。ゲージが満タンの時の量) */
			float maxTime;
			/** ゲージが空から満タンまで戻るのにかかる時間(本当の秒。遅くしていない間に戻る) */
			float recoverTime;
			/** ゲージを使い切った後、また遅くできるようになるのに要る量(本当の秒) */
			float restartTime;


			SlowMotionParameter()
				: timeScale(0.3f)
				, blendTime(0.2f)
				, maxTime(3.0f)
				, recoverTime(6.0f)
				, restartTime(1.0f)
			{}

			/**
			 * @brief JSONからパラメーターを読む
			 * @param file パラメーターファイル
			 */
			void Read(const util::ParameterFile& file)
			{
				timeScale = file.GetFloat("timeScale", timeScale);
				blendTime = file.GetFloat("blendTime", blendTime);
				maxTime = file.GetFloat("maxTime", maxTime);
				recoverTime = file.GetFloat("recoverTime", recoverTime);
				restartTime = file.GetFloat("restartTime", restartTime);
			}
		};


		/**
		 * @brief ファインダーを覗いている間の、スローモーション
		 * @details 世界(悪党・客)の時間の倍率(util::WorldTime)を変えるだけ。主人公・カメラ・UI は普通の速さのまま。
		 *          遅くできる量はゲージで決まる。遅くしている間は減り、遅くしていない間は戻る。
		 *          使い切ったら、覗いていても普通の速さ。restartTime の分だけ戻るまで、また遅くはならない
		 */
		class SlowMotion : public Noncopyable
		{
		public:
			/**
			 * @brief 初期化(パラメーターを読んで、普通の速さにする)
			 */
			void Initialize()
			{
				m_parameter.Initialize(PARAMETER_PATH);
				Reset();
			}
			/**
			 * @brief 普通の速さに戻す(シーンを抜ける時にも呼ぶ。倍率は全体で 1 つなので、残さない)
			 */
			void Reset()
			{
				m_scale = 1.0f;
				m_maxTime = m_parameter.Get().maxTime;
				m_restartTime = m_parameter.Get().restartTime;
				m_gauge = m_maxTime;
				m_isUsing = false;
				util::WorldTime::SetScale(m_scale);
			}
			/**
			 * @brief 更新処理(毎フレーム呼ぶ)
			 * @param isRequest 遅くしたいか(ファインダーを覗いている)
			 */
			void Update(const bool isRequest)
			{
				const SlowMotionParameter& parameter = m_parameter.Get();
				// ゲージやつなぎの時間は本当の時間で数える(遅くなった世界の時間で数えると、上限が伸びてしまう)
				const float deltaTime = g_gameTime->GetFrameDeltaTime();

				// 遅くするか: 使っている途中なら、ゲージがある間は続ける。使い始めは restartTime 以上ないと始めない
				// ※ 使い切った直後に、少し戻った所でまた遅くなって、すぐ切れる(ちらつく)のを防ぐ
				if (!isRequest)
				{
					m_isUsing = false;
				}
				else if (m_isUsing)
				{
					m_isUsing = m_gauge > 0.0f;
				}
				else
				{
					m_isUsing = m_gauge >= parameter.restartTime;
				}
				const bool isSlow = m_isUsing;

				// ゲージ: 遅くしている間は減って、それ以外は戻る
				if (isSlow)
				{
					m_gauge -= deltaTime;
					if (m_gauge < 0.0f) m_gauge = 0.0f;
				}
				else if (parameter.recoverTime > 0.0f)
				{
					m_gauge += parameter.maxTime * deltaTime / parameter.recoverTime;
				}
				else
				{
					m_gauge = parameter.maxTime;
				}
				// JSON で maxTime を小さくした時にも、はみ出さない
				if (m_gauge > parameter.maxTime) m_gauge = parameter.maxTime;
				m_maxTime = parameter.maxTime;
				m_restartTime = parameter.restartTime;

				// 目標の速さへ、blendTime かけて滑らかに変える
				const float targetScale = isSlow ? parameter.timeScale : 1.0f;
				const float range = 1.0f - parameter.timeScale;
				const float step = parameter.blendTime > 0.0f ? range * deltaTime / parameter.blendTime : 1.0f;
				if (m_scale < targetScale)
				{
					m_scale = m_scale + step < targetScale ? m_scale + step : targetScale;
				}
				else
				{
					m_scale = m_scale - step > targetScale ? m_scale - step : targetScale;
				}
				util::WorldTime::SetScale(m_scale);
			}
			/**
			 * @brief 遅くなっているか
			 * @return 普通の速さでなければ true
			 */
			bool IsSlow() const { return m_scale < 1.0f; }
			/**
			 * @brief ゲージの残りの割合(あとで UI に出す用)
			 * @return 0.0(空) ~ 1.0(満タン)
			 */
			float GetGaugeRate() const
			{
				// ※ Parameter::Get は const でない(読み直すため)ので、Update で覚えた満タンの量を使う
				return m_maxTime > 0.0f ? m_gauge / m_maxTime : 0.0f;
			}
			/**
			 * @brief 今、遅くしているか(ゲージを使っている途中か)
			 * @return 使っていれば true
			 */
			bool IsUsing() const { return m_isUsing; }
			/**
			 * @brief 覗けば遅くできるか
			 * @return 使っている途中か、ゲージが restartTime 以上あれば true(使い切った直後は false)
			 */
			bool CanStart() const { return m_isUsing || m_gauge >= m_restartTime; }


		private:
			/** パラメーターの JSON */
			static constexpr const char* PARAMETER_PATH = "Assets/parameter/Camera/SlowMotion.json";

			/** パラメーター */
			util::Parameter<SlowMotionParameter> m_parameter;
			/** 今の世界の速さ */
			float m_scale = 1.0f;
			/** ゲージ(あと何秒遅くできるか。本当の秒) */
			float m_gauge = 0.0f;
			/** ゲージが満タンの時の量(GetGaugeRate 用に覚えておく) */
			float m_maxTime = 0.0f;
			/** 使い切った後、また遅くできるのに要る量(CanStart 用に覚えておく) */
			float m_restartTime = 0.0f;
			/** 遅くするのを使っている途中か */
			bool m_isUsing = false;
		};
	}
}
