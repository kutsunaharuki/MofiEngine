/**
 * @file SceneLight.cpp
 * @brief シーンライトクラス
 */
#include "k2EngineLowPreCompile.h"
#include "SceneLight.h"


namespace nsK2EngineLow {
	void SceneLight::SetDirection(const Vector3& dir)
	{
		lightCB_.directionLight.direction = dir;
	}
	
	
	void SceneLight::SetColor(const Vector3 & color)
	{
		lightCB_.directionLight.color = color;
	}
	
	
	void SceneLight::SetAmbient(const Vector3 & ambient)
	{
		lightCB_.ambientLight.ambient = ambient;
	}
}