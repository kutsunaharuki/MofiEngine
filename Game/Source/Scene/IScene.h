/**
 * @file IScene.h
 * @brief シーンの基底クラス
 */
#pragma once
#include "Source/Util/CRC32.h"


/**
 * @details Hash32は文字列を数値に変換する関数。
 　　　　　 数値に変換する時に別の文字でも同じ数値になるケースがあるが重複しないようにしている。
 */
#define appScene(name)\
public:\
static constexpr uint32_t ID() { return Hash32(#name);}


namespace app
{
	/** 無効なシーンID */
	static constexpr uint32_t INVALID_SCENE_ID = 0xffffffff;


	/**
	 * @brief シーンの基底クラス
	 */
	class IScene : public Noncopyable
	{
	public:
		IScene(){}
		virtual ~IScene(){}

		/** 純粋仮想関数 */
		virtual bool Start() = 0;
		virtual void Update() = 0;
		virtual void Render(RenderContext& rc) = 0;

		/**
		 * @brief シーンのロードが完了したかどうかを返す
		 * @return ロードが完了したらtrue
		 */
		virtual bool IsLoaded() const { return true; }
		/**
		 * @brief シーンをリクエストする
		 * @param id シーンID
		 * @param waitTime シーン遷移するまでの時間
		 */
		virtual bool RequestScene(uint32_t& id, float& waitTime) = 0;
	};
}

