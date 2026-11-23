/***
 * @file FinderMenu.h
 * @brief ファインダーを構えている時の UI
 */
#pragma once
#include "Source/UI/UIMenu.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief ファインダーを構えている時の UI
		 * @details 外枠・真ん中のグリッド・ピントのバーとつまみ・ズームのスライダーとつまみ・被写体の顔のまわりの丸を出す。
		 *          何を表示するか(ピントの量・丸の位置など)は、FinderController が毎フレーム渡す。
		 *          JSON : Assets/parameter/UI/Finder/Finder.json
		 */
		class FinderMenu : public UIMenuBase
		{
		public:
			FinderMenu();
			~FinderMenu() override;

			/** 更新処理 */
			void Update() override;
			/** 初期化処理(ホットリロードのたびにも呼ばれる) */
			void InitializeLogic() override;


		public:
			/**
			 * @brief 開く(ファインダーを構えた時)
			 */
			void Open();
			/**
			 * @brief 閉じる(ファインダーをやめた時)
			 */
			void Close();
			/**
			 * @brief 開いているか
			 * @return 開いているかどうか
			 */
			bool IsOpen() const { return m_isOpen; }

			/**
			 * @brief ピントの量を設定(ピントのつまみが動く)
			 * @param rate 0.0(一番近い) ~ 1.0(一番遠い)
			 */
			void SetFocusRate(const float rate) { m_focusRate = rate; }
			/**
			 * @brief ズームの量を設定(ズームのつまみが動く)
			 * @param rate 0.0(ズームなし) ~ 1.0(一番寄った)
			 */
			void SetZoomRate(const float rate) { m_zoomRate = rate; }

			/**
			 * @brief 被写体の顔のまわりに丸を出す
			 * @param screenPosition 顔の画面の位置(画面の真ん中が原点、上が +Y)
			 * @param radius 丸の半径(ピクセル)
			 * @param isInRange 真ん中の十字が丸の中に入っているか(入っていれば細い丸、外なら太い丸)
			 */
			void ShowSubjectCircle(const Vector2& screenPosition, const float radius, const bool isInRange);
			/**
			 * @brief 被写体の丸を消す(候補の被写体がいない時)
			 */
			void HideSubjectCircle();

			/**
			 * @brief 真ん中のグリッドの大きさの半分を取得
			 * @return 横幅の半分(x)と縦幅の半分(y)。グリッドが無ければ (0, 0)
			 * @details この中に顔がある被写体だけが、ピントの候補になる
			 */
			Vector2 GetGridHalfSize();

			/**
			 * @brief スローモーションのゲージを設定
			 * @param rate 残りの割合(0.0 = 空 ~ 1.0 = 満タン)
			 * @param canStart 覗けば遅くできるか(使い切った直後は false → ゲージの色を変える)
			 */
			void SetSlowGauge(const float rate, const bool canStart)
			{
				m_slowGaugeRate = rate;
				m_canSlowStart = canStart;
			}
			/**
			 * @brief 撮った時の、画面全体のフラッシュを始める
			 * @details 一気に白くなって、すぐに消えていく
			 */
			void PlayShotFlash() { m_shotFlashTimer = 0.0f; }


		private:
			/**
			 * @brief つまみを、バーの量の位置に動かす
			 * @param knob つまみ
			 * @param knobBasePosition つまみの JSON での位置(x はこのまま使う)
			 * @param bar バー
			 * @param rate 量(0.0 = バーの下 ~ 1.0 = バーの上)
			 */
			void ApplyKnob(UIIcon* knob, const Vector3& knobBasePosition, UIIcon* bar, const float rate);
			/**
			 * @brief 丸の画像を、位置と半径に合わせる
			 * @param circle 丸の画像
			 * @param screenPosition 位置
			 * @param radius 半径
			 */
			void ApplyCircle(UIIcon* circle, const Vector2& screenPosition, const float radius);
			/**
			 * @brief スローモーションのゲージを、残りの量と色に合わせる
			 */
			void ApplySlowGauge();
			/**
			 * @brief 撮った時のフラッシュを、時間に合わせて薄くしていく
			 */
			void ApplyShotFlash();


		private:
			/** ピントのバー */
			UIIcon* m_focusBar;
			/** ピントのつまみ */
			UIIcon* m_focusKnob;
			/** ズームのスライダー */
			UIIcon* m_zoomBar;
			/** ズームのつまみ */
			UIIcon* m_zoomKnob;
			/** ファインダーの大枠 */
			UIIcon* m_focusFrame;
			/** 真ん中のグリッド */
			UIIcon* m_grid;
			/** 真ん中のグリッドの中心 */
			UIIcon* m_gridCenter;
			/** 被写体の丸(十字が中に入っている時) */
			UIIcon* m_inRangeCircle;
			/** 被写体の丸(十字が外にある時) */
			UIIcon* m_outRangeCircle;
			/** ピントのつまみの、JSON での位置 */
			Vector3 m_focusKnobBasePosition;
			/** ズームのつまみの、JSON での位置 */
			Vector3 m_zoomKnobBasePosition;

			/** ピントの量(0 ~ 1) */
			float m_focusRate;
			/** ズームの量(0 ~ 1) */
			float m_zoomRate;
			/** スローモーションのゲージの中身(残りの量だけ横に伸びる) */
			UIIcon* m_slowGauge;
			/** スローモーションのゲージの中身の、JSON での色(遅くできる時の色) */
			Vector4 m_slowGaugeColor;
			/** スローモーションのゲージの残り(0 ~ 1) */
			float m_slowGaugeRate;
			/** 覗けば遅くできるか */
			bool m_canSlowStart;
			/** 撮った時に、画面全体を白く光らせる板 */
			UIFrame* m_shotFlash;
			/** 撮った時のフラッシュを始めてからの時間(マイナスなら光っていない) */
			float m_shotFlashTimer;
			/** 開いているか */
			bool m_isOpen;
		};
	}
}
