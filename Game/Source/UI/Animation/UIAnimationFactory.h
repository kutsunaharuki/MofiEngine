/**
 * @file UIAnimationFactory.h
 * @brief UIAnimationParameter(JSON)の値で、UI のアニメーションを作る
 * @details UIAnimation.h / UIAnimationParameter.h はこのファイルを知らないので、include が循環しない
 *
 * 使い方(InitializeLogic の中)
 *   UIAnimationFactory::Attach<UIScaleAnimation>(text, m_animationParameter, anim_key::SELECT);
 * ホットリロードで JSON が変わったら、同じ Attach をもう一度呼ぶ(作り直さずに、値だけ入れ直す)
 */
#pragma once
#include "Source/UI/Animation/UIAnimation.h"
#include "Source/UI/Animation/UIAnimationParameter.h"
#include "Source/UI/UIParts.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief JSON の値で、UI のアニメーションを作るクラス
		 */
		class UIAnimationFactory
		{
		public:
			/**
			 * @brief UI のアニメーターに、JSON の値でアニメーションを登録する
			 * @tparam T アニメーションの型(UIScaleAnimation など)
			 * @param ui アニメーションさせる UI
			 * @param parameter 値を持っている UIAnimationParameter
			 * @param key アニメーションのキー(JSON の "key" と同じ)
			 * @return 登録したアニメーション(UI が無い、または JSON にキーが無ければ nullptr)
			 * @details 同じキーのアニメーションがもうあれば、作らずに値だけ入れ直す
			 */
			template <typename T>
			static T* Attach(UIBase* ui, const UIAnimationParameter& parameter, const uint32_t key)
			{
				if (ui == nullptr) return nullptr;

				const UIAnimationDef* def = parameter.Find(key);
				if (def == nullptr) return nullptr;

				UIAnimator& animator = ui->GetAnimator();
				T* animation = animator.FindAnimation<T>(key);
				if (animation == nullptr)
				{
					animation = animator.CreateAnimation<T>(key);
				}

				ApplyParameter(animation, *def);
				return animation;
			}


		private:
			/**
			 * @brief アニメーションに値を入れる
			 * @tparam V 値の型(float / Vector2 / Vector3 / Vector4)。アニメーションのクラスから決まる
			 */
			template <typename V>
			static void ApplyParameter(UIValueAnimation<V>* animation, const UIAnimationDef& def)
			{
				V start;
				V end;
				ToValue(def.start, start);
				ToValue(def.end, end);
				animation->SetParameter(start, end, def.duration, def.easingType, def.loopMode);
			}


			/** Vector4 から、必要な分だけ取り出す */
			static void ToValue(const Vector4& value, float& out) { out = value.x; }
			static void ToValue(const Vector4& value, Vector2& out) { out = Vector2(value.x, value.y); }
			static void ToValue(const Vector4& value, Vector3& out) { out = Vector3(value.x, value.y, value.z); }
			static void ToValue(const Vector4& value, Vector4& out) { out = value; }
		};
	}
}
