/**
 * @file Game.cpp
 * @brief ゲームの根幹クラス
 * @details ゲームに必要なシーンやサウンド、パラメーターの管理者をここで作ったり破棄したりする
 */
#pragma once
#include "Source/Core/TransformNode.h"


class Game : public IGameObject
{
public:
	Game();
	~Game();
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;
};