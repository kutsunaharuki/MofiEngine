/**
 * @file PhotoSampleRenderer.h
 * @brief ミッションの見本の絵をオフスクリーンで1回だけ描く
 */
#pragma once
#include "Source/Mission/MissionData.h"
#include "Source/Mission/ShotMeasure.h"
#include "Source/Util/ParameterManager.h"
#include <memory>
#include <vector>


namespace app
{
	namespace mission
	{
		/**
		 * @brief ミッションの見本の絵をオフスクリーンで1回だけ描く
		 * @details 悪党と同じモデルの人形を盗むアニメーションの瞬間で止めて、
		 *		    見本の数値(sample)通りの位置にカメラを置いて描く。描いた絵は UI として扱う(UIに出す)
		 *          人形は Draw を呼ばない(本編・ファインダーには出ない)。当たり判定も持たない
		 */
		class PhotoSampleRenderer : public Noncopyable
		{
		public:
			PhotoSampleRenderer();
			~PhotoSampleRenderer();


			/**
			 * @brief 見本の人形・描き先・カメラを作る
			 * @param mission ミッション(見本が無いミッションなら何もしない)
			 */
			void Initialize(const MissionData& mission);
			/**
			 * @brief 更新処理(描き終わるまで毎フレーム呼ぶ)
			 * @details 人形のポーズ → カメラ合わせ → 見本の確かめ → 描く頼み、の順に進む
			 */
			void Update();
			/**
			 * @brief 描き終わったか
			 * @return 描く頼みを出し終わっていれば true(このフレームの描画で描かれる)
			 */
			bool IsRendered() const { return m_isRendered; }
			/**
			 * @brief 見本のテクスチャを取得
			 * @return 見本のテクスチャ
			 */
			Texture& GetTexture() { return m_renderTarget.GetRenderTargetTexture(); }


		private:
			/**
			 * @brief セットの 1 つ(床・壁・絵画)
			 */
			struct SetPiece
			{
				/** モデル(Draw は呼ばない。見本にだけ描く) */
				std::unique_ptr<ModelRender> render;
				/** 基準からの位置(床・壁・壁の絵画: 壁の根元が原点、手に持つ物: 両手の真ん中が原点。どちらも部屋(人形の正面)の方が +Z、+Y が上) */
				Vector3 position;
				/** 基準の中の回転 */
				Quaternion rotation;
				/** スケール */
				Vector3 scale;
				/** 手に持つ物か(盗んだ絵画) */
				bool isHeld = false;
			};
			/**
			 * @brief セットの客(画面の端で見切れている人)
			 */
			struct SetPerson
			{
				/** モデル(Draw は呼ばない。見本にだけ描く) */
				std::unique_ptr<ModelRender> render;
				/** アニメーション(待機など 1 つ) */
				std::unique_ptr<AnimationClip[]> animationClips;
				/** 画面の横の位置(0 = 左端 ~ 1 = 右端。端に置くと見切れる) */
				float screenX = 0.0f;
				/** 奥行き(カメラから人形までの距離に対する割合。1 = 人形と同じ、1 より大きいと人形より奥) */
				float depthRate = 1.0f;
				/** 壁の方を向いた向きから、さらに回す角度(度) */
				float turnDeg = 0.0f;
				/** スケール */
				Vector3 scale;
			};


		private:
			/**
			 * @brief 見本の数値からカメラの位置と向きを決める(測り直して直す)
			 */
			void PhotoCamera();
			/**
			 * @brief カメラを注視点と「頭 → カメラ」の向き・距離で設定
			 * @param target 注視点
			 * @param headPosition 頭の位置(カメラはここから toCamera の向きに distance 離れた所)
			 * @param toCamera 頭からカメラへの向き(長さ 1)
			 * @param distance 距離
			 */
			void SetCamera(const Vector3& target, const Vector3& headPosition, const Vector3& toCamera, const float distance);
			/**
			 * @brief 今のカメラで人形の形を測る
			 * @return 測った値(頭のボーンが取れなければ isMeasured が false)
			 */
			ShotMeasure MeasureDummy();
			/**
			 * @brief 見本そのものを照合にかけて顔が見えているかを確かめる
			 * @param measure 写り方を測った値
			 * @details どう撮っても成功できない見本なら、止めて知らせる(JSON の書き間違いに遊ぶ前に気づける)
			 */
			void CheckSample(const ShotMeasure& measure);
			/**
			 * @brief セット(床・壁・絵画)を JSON から読んで作る
			 */
			void InitializeSet();
			/**
			 * @brief セットを置く
			 * @details カメラを決めてから呼ぶ。決定的瞬間 = 絵画をつかんで、部屋の方へ振り向いた所。
			 *          壁は人形の背中側(盗んだ壁)に、部屋の方を向けて置く。壁には周りの絵画が掛かっている。
			 *          盗んだ絵画("hold":true)は、人形の両手の間に持たせる
			 */
			void PlaceSet();
			/**
			 * @brief 1 つの置物を、基準の位置と向きで置く
			 * @param piece 置物
			 * @param origin 基準の位置
			 * @param frontZ 基準の +Z の向き(水平・長さ 1)
			 */
			void PlacePiece(SetPiece& piece, const Vector3& origin, const Vector3& frontZ);
			/**
			 * @brief 客を、画面の端の位置に置く(人形には近づけない)
			 * @param wallOrigin 壁の根元(客が壁にめり込まないようにする)
			 * @param wallFront 壁の正面の向き(客はこの逆 = 壁の絵画の方を向く)
			 */
			void PlacePeople(const Vector3& wallOrigin, const Vector3& wallFront);
			/**
			 * @brief 両手の真ん中の位置を取る
			 * @param outPosition 両手の真ん中(片手しか取れなければ、その手)
			 * @return 取れたら true
			 */
			bool CalcHandCenter(Vector3& outPosition) const;


		private:
			/** 見本の人形 */
			ModelRender m_dummy;
			/** 人形のアニメーション */
			std::unique_ptr<AnimationClip[]> m_animationClips;
			/** 見本の絵の描き先 */
			RenderTarget m_renderTarget;
			/** 見本のカメラ */
			Camera m_camera;
			/** 見本の数値と許容値 */
			MissionData m_mission;
			/** 頭のボーンの番号 */
			int m_headBoneNo;
			/** 頭のボーンの中で「顔の前」の向き */
			Vector3 m_headForwardLocal;
			/** 左手のボーンの番号(-1 なら無い) */
			int m_leftHandBoneNo;
			/** 右手のボーンの番号(-1 なら無い) */
			int m_rightHandBoneNo;
			/** セット(床・壁・絵画) */
			std::vector<SetPiece> m_setPieceList;
			/** セットの客 */
			std::vector<SetPerson> m_setPersonList;
			/** セットの JSON(ホットリロードで書き換わったら、描き直す) */
			util::ParameterFile* m_setFile;
			/** セットを読んだ時の、JSON の変わった回数 */
			uint32_t m_setVersion;
			/** 人形の足元から、背中側の壁までの距離 */
			float m_wallDistance;
			/** 盗むアニメーションを始めてからの時間 */
			float m_poseTimer;
			/** 見本があって、人形を作ったか */
			bool m_isInitialized;
			/** ポーズを止めたか */
			bool m_isPosed;
			/** 描き終わったか */
			bool m_isRendered;
		};
	}
}
