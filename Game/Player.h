/**
 * @file Player.h
 * @brief 簡単なプレイヤークラス
 */
#pragma once

namespace app
{
	namespace actor
	{
		class Player : public IGameObject
		{
		public:
			bool Start() override;


		};
	}
}