/**
 * @file IRender2D.h
 * @brief 2Dパスで描画するもののインターフェース
 */
#pragma once


namespace nsK2EngineLow
{
	/**
	 * @brief 2Dパスで描画するもののインターフェース
	 * @details SpriteRender / FontRender が継承する。
	 *          RenderingEngine は、登録された順番に OnRender2D を呼ぶので、
	 *          スプライトと文字を混ぜても、後から登録したものが手前に描画される
	 */
	class IRender2D
	{
	public:
		virtual ~IRender2D() {}

		/**
		 * @brief 2Dパスでの実際の描画
		 * @param rc レンダリングコンテキスト
		 * @details RenderingEngine の2Dパスから呼ばれる
		 */
		virtual void OnRender2D(RenderContext& rc) = 0;
	};
}
