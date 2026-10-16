/**
 * @file UIAnimation.h
 * @brief UIAnimationの機能群
 * @details UIParts の UIBase に対して、カラー・スケール・座標・回転のアニメーションを行う。
 *          アニメーションは UIAnimator が所有し、UIAnimationSequence で順番に再生できる
 */
#pragma once
#include "Source/Util/Curve.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>


namespace app
{
	namespace ui
	{
		/** 前方宣言 */
		class UIBase;


		/** 長いので省略 */
		template <typename T>
		using UIAnimationApplyFunc = std::function<void(const T&)>;


		/**
		 * @brief UIアニメーションの基底クラス
		 */
		class UIAnimationBase : public Noncopyable
		{
		public:
			UIAnimationBase()
				: m_ui(nullptr)
				, m_key(0)
			{}

			virtual ~UIAnimationBase()
			{}


			/** 更新処理 */
			virtual void Update() = 0;
			/** 再生 */
			virtual void PlayAnimation() = 0;
			/** 停止 */
			virtual void StopAnimation() = 0;
			/** 再生中か */
			virtual bool IsPlayAnimation() const = 0;


		public:
			/**
			 * @brief UIの設定
			 * @param ui アニメーションさせるUI
			 */
			void SetUI(UIBase* ui) { m_ui = ui; }
			/**
			 * @brief UIの取得
			 * @return アニメーションさせるUI
			 */
			UIBase* GetUI() { return m_ui; }

			/**
			 * @brief キーの設定
			 * @param key キー
			 */
			void SetKey(const uint32_t key) { m_key = key; }
			/**
			 * @brief キーの取得
			 * @return キー
			 */
			uint32_t GetKey() const { return m_key; }


		protected:
			/** アニメーションさせるUI */
			UIBase* m_ui;
			/** キー */
			uint32_t m_key;
		};




		/*****************************************************/


		/**
		 * @brief 値をカーブで変化させるアニメーション
		 * @tparam T アニメーションさせる値の型(float / Vector2 / Vector3 / Vector4)
		 * @details 再生中は、毎フレーム、カーブの値を SetFunc で設定した関数に渡す
		 */
		template <typename T>
		class UIValueAnimation : public UIAnimationBase
		{
		public:
			UIValueAnimation()
				: m_start()
				, m_end()
				, m_timeSec(0.0f)
				, m_easingType(util::EasingType::Linear)
				, m_loopMode(util::LoopMode::Once)
			{}

			~UIValueAnimation() override
			{}


			/** 更新処理 */
			void Update() override
			{
				if (!m_curve.IsPlaying()) return;

				m_curve.Update(g_gameTime->GetFrameDeltaTime());
				// 停止した瞬間のフレームも、最後の値を反映する
				Apply(m_curve.GetCurrentValue());
			}


			/**
			 * @brief 再生
			 * @details 毎回、最初から再生する
			 */
			void PlayAnimation() override
			{
				OnPlay();

				// Curve::Play は時間を戻さないので、初期化し直してから再生する
				m_curve.Initialize(m_start, m_end, m_timeSec, m_easingType, m_loopMode);
				m_curve.Play();
				Apply(m_curve.GetCurrentValue());
			}


			/** 停止 */
			void StopAnimation() override
			{
				m_curve.Stop();
			}


			/** 再生中か */
			bool IsPlayAnimation() const override
			{
				return m_curve.IsPlaying();
			}


		public:
			/**
			 * @brief UIアニメーションのパラメーターを設定
			 * @param start 最初の値
			 * @param end 最後の値
			 * @param timeSec 秒数
			 * @param type イージングタイプ
			 * @param loopMode ループモード
			 */
			void SetParameter(
				const T& start
				, const T& end
				, const float timeSec
				, const util::EasingType type = util::EasingType::Linear
				, const util::LoopMode loopMode = util::LoopMode::Once
			)
			{
				m_start = start;
				m_end = end;
				m_timeSec = timeSec;
				m_easingType = type;
				m_loopMode = loopMode;
				m_curve.Initialize(m_start, m_end, m_timeSec, m_easingType, m_loopMode);
			}

			/**
			 * @brief アニメーション中の現在の値を取得
			 * @return 現在の値
			 */
			T GetCurrentValue() const { return m_curve.GetCurrentValue(); }

			/**
			 * @brief カーブの値をUIに適用する関数を設定
			 * @param func 適用する関数
			 */
			void SetFunc(const UIAnimationApplyFunc<T>& func) { m_applyFunc = func; }


		protected:
			/**
			 * @brief 再生する直前に呼ばれる
			 * @details 再生開始時のUIの状態を覚えておきたいときに使う
			 */
			virtual void OnPlay() {}


		private:
			/**
			 * @brief 値をUIに適用する
			 * @param value 適用する値
			 */
			void Apply(const T& value)
			{
				if (m_ui == nullptr || !m_applyFunc) return;
				m_applyFunc(value);
			}


		protected:
			/** カーブ */
			util::Curve<T> m_curve;
			/** 最初の値 */
			T m_start;
			/** 最後の値 */
			T m_end;
			/** 秒数 */
			float m_timeSec;
			/** イージングタイプ */
			util::EasingType m_easingType;
			/** ループモード */
			util::LoopMode m_loopMode;
			/** カーブの値をUIに適用する関数 */
			UIAnimationApplyFunc<T> m_applyFunc;
		};


		/** 型ごとの省略 */
		using UIFloatAnimation = UIValueAnimation<float>;
		using UIVector2Animation = UIValueAnimation<Vector2>;
		using UIVector3Animation = UIValueAnimation<Vector3>;
		using UIVector4Animation = UIValueAnimation<Vector4>;




		/*****************************************************/


		/**
		 * @brief カラーアニメーション
		 */
		class UIColorAnimation : public UIVector4Animation
		{
		public:
			UIColorAnimation();
			~UIColorAnimation() override {}
		};


		/**
		 * @brief 拡縮アニメーション
		 */
		class UIScaleAnimation : public UIVector3Animation
		{
		public:
			UIScaleAnimation();
			~UIScaleAnimation() override {}
		};


		/**
		 * @brief 座標アニメーション
		 */
		class UITranslateAnimation : public UIVector3Animation
		{
		public:
			UITranslateAnimation();
			~UITranslateAnimation() override {}
		};


		/**
		 * @brief 差分アニメーション
		 * @details 再生開始時の座標に、カーブの値を足した座標にする
		 */
		class UITranslateOffsetAnimation : public UIVector3Animation
		{
		public:
			UITranslateOffsetAnimation();
			~UITranslateOffsetAnimation() override {}


		protected:
			void OnPlay() override;


		private:
			/** 再生開始時の座標 */
			Vector3 m_basePosition;
		};


		/**
		 * @brief 回転アニメーション(Z軸、度数法)
		 */
		class UIRotationAnimation : public UIFloatAnimation
		{
		public:
			UIRotationAnimation();
			~UIRotationAnimation() override {}
		};




		/*****************************************************/


		/**
		 * @brief UIアニメーションをまとめて持つクラス
		 * @details 持っているアニメーションは、全て同じUIを動かす。
		 *          UIの Update より先に Update を呼ぶと、そのフレームの値が描画に反映される
		 */
		class UIAnimator : public Noncopyable
		{
		public:
			UIAnimator() : m_ui(nullptr) {}
			~UIAnimator() {}


			/** 更新処理 */
			void Update()
			{
				for (auto& animation : m_animationList)
				{
					animation->Update();
				}
			}


		public:
			/**
			 * @brief アニメーションさせるUIを設定
			 * @param ui アニメーションさせるUI
			 * @details 作成済みのアニメーションにも反映する
			 */
			void SetUI(UIBase* ui)
			{
				m_ui = ui;
				for (auto& animation : m_animationList)
				{
					animation->SetUI(m_ui);
				}
			}
			/**
			 * @brief アニメーションさせるUIを取得
			 * @return アニメーションさせるUI
			 */
			UIBase* GetUI() { return m_ui; }

			/**
			 * @brief アニメーションを作成する
			 * @tparam T アニメーションの型(UIAnimationBase の派生クラス)
			 * @param key アニメーションのキー
			 * @return 作成したアニメーションのポインタ(所有は UIAnimator が持つ)
			 */
			template <typename T>
			T* CreateAnimation(const uint32_t key)
			{
				static_assert(std::is_base_of<UIAnimationBase, T>::value, "T は UIAnimationBase の派生クラスにしてください。");

				auto animation = std::make_unique<T>();
				T* rawPtr = animation.get();
				animation->SetKey(key);
				animation->SetUI(m_ui);
				m_animationList.push_back(std::move(animation));
				return rawPtr;
			}

			/**
			 * @brief アニメーションを削除
			 * @param key アニメーションのキー
			 */
			void RemoveAnimation(const uint32_t key)
			{
				auto it = std::find_if(m_animationList.begin(), m_animationList.end(),
					[&](const std::unique_ptr<UIAnimationBase>& animation) { return animation->GetKey() == key; });

				if (it != m_animationList.end())
				{
					m_animationList.erase(it);
				}
			}

			/**
			 * @brief アニメーションを探す
			 * @tparam T アニメーションの型
			 * @param key アニメーションのキー
			 * @return 見つかったアニメーションのポインタ(なければ、または型が違えば nullptr)
			 */
			template <typename T = UIAnimationBase>
			T* FindAnimation(const uint32_t key)
			{
				auto it = std::find_if(m_animationList.begin(), m_animationList.end(),
					[&](const std::unique_ptr<UIAnimationBase>& animation) { return animation->GetKey() == key; });

				if (it != m_animationList.end())
				{
					return dynamic_cast<T*>(it->get());
				}
				return nullptr;
			}

			/**
			 * @brief アニメーションを再生
			 * @param key アニメーションのキー
			 */
			void Play(const uint32_t key)
			{
				UIAnimationBase* animation = FindAnimation(key);
				if (animation) animation->PlayAnimation();
			}

			/**
			 * @brief 全てのアニメーションを停止
			 */
			void StopAll()
			{
				for (auto& animation : m_animationList)
				{
					animation->StopAnimation();
				}
			}


		private:
			/** アニメーションさせるUI */
			UIBase* m_ui;
			/** アニメーションのリスト */
			std::vector<std::unique_ptr<UIAnimationBase>> m_animationList;
		};




		/*****************************************************/


		/**
		 * @brief シーケンス内の1ステップ
		 * @details シーケンスアニメーションで使用する構造体
		 */
		struct UIAnimationStep
		{
			/** 再生するアニメーションのキー */
			uint32_t animationKey = 0;
			/** このステップ開始前の待機時間(秒) */
			float delayBefore = 0.0f;
			/** 開始時コールバック */
			std::function<void()> onStart;
			/** 終了時コールバック */
			std::function<void()> onComplete;
		};


		/**
		 * @brief UIAnimator のアニメーションを、追加した順番に再生するクラス
		 * @details LoopMode が Once 以外のアニメーションは終わらないので、次のステップに進まない
		 */
		class UIAnimationSequence : public Noncopyable
		{
		public:
			UIAnimationSequence()
				: m_currentIndex(-1)
				, m_isPlaying(false)
				, m_delayTimer(0.0f)
				, m_waitingDelay(false)
				, m_animator(nullptr)
			{}

			~UIAnimationSequence() {}


			/** 更新処理 */
			void Update();


		public:
			/**
			 * @brief ステップを追加
			 * @param animationKey 再生するアニメーションのキー
			 * @param delayBefore このステップ開始前の待機時間(秒)
			 * @param onStart ステップ開始時のコールバック
			 * @param onComplete ステップ終了時のコールバック
			 * @return 自身の参照
			 */
			UIAnimationSequence& Add(
				const uint32_t animationKey
				, const float delayBefore = 0.0f
				, std::function<void()> onStart = nullptr
				, std::function<void()> onComplete = nullptr
			)
			{
				UIAnimationStep step;
				step.animationKey = animationKey;
				step.delayBefore = delayBefore;
				step.onStart = std::move(onStart);
				step.onComplete = std::move(onComplete);
				m_steps.push_back(std::move(step));
				return *this;
			}

			/**
			 * @brief シーケンス全体の完了コールバックを設定
			 * @param callback シーケンス全体の完了時のコールバック
			 * @return 自身の参照
			 */
			UIAnimationSequence& OnComplete(std::function<void()> callback)
			{
				m_onSequenceComplete = std::move(callback);
				return *this;
			}

			/**
			 * @brief 再生開始
			 * @param animator アニメーションを持っている UIAnimator
			 */
			void Play(UIAnimator* animator)
			{
				if (m_steps.empty() || animator == nullptr) return;

				m_animator = animator;
				m_currentIndex = -1;
				m_isPlaying = true;
				m_waitingDelay = false;
				AdvanceToNext();
			}

			/**
			 * @brief 停止
			 * @details 再生中のステップのアニメーションも停止する
			 */
			void Stop();

			/**
			 * @brief 再生中かどうか
			 * @return 再生中かどうか
			 */
			bool IsPlaying() const { return m_isPlaying; }

			/**
			 * @brief ステップを全てクリアして、停止する
			 */
			void Clear()
			{
				Stop();
				m_steps.clear();
			}


		private:
			/** 次のステップに進む */
			void AdvanceToNext();
			/** 現在のステップを開始する */
			void StartCurrentStep();
			/** 現在のステップのインデックスが有効か */
			bool IsValidIndex() const
			{
				return m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_steps.size());
			}


		private:
			/** ステップのリスト(再生する順番) */
			std::vector<UIAnimationStep> m_steps;
			/** 現在再生しているステップのインデックス */
			int m_currentIndex;
			/** 再生中かどうか */
			bool m_isPlaying;
			/** ステップ開始前の待機時間の残り(秒) */
			float m_delayTimer;
			/** ステップ開始前の待機中かどうか */
			bool m_waitingDelay;
			/** アニメーションを持っている UIAnimator */
			UIAnimator* m_animator;
			/** シーケンス全体の完了時のコールバック */
			std::function<void()> m_onSequenceComplete;
		};
	}
}
