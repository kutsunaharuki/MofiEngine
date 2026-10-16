/**
 * @file JsonConverter.h
 * @brief JSONの値を、ゲームで使う型に変換する機能群
 * @details キーが無い、または型が違うときは、指定した既定値を返す。
 *          ベクトルは [x, y, z] の配列と {"x": .., "y": .., "z": ..} のオブジェクトの、どちらでも読める
 */
#pragma once
#include "nlohmann/json.hpp"
#include <ctime>
#include <string>


namespace app
{
	namespace util
	{
		/**
		 * @brief JSONの値を変換するクラス
		 */
		class JsonConverter
		{
		public:
			/**
			 * @brief JSONファイルを読み込む
			 * @param filePath ファイルパス
			 * @param out 読み込んだJSON
			 * @return 読み込めたかどうか(ファイルが無い、または書式が間違っていれば false)
			 */
			static bool LoadFile(const std::string& filePath, nlohmann::json& out);

			/**
			 * @brief ファイルの最終更新日時を取得
			 * @param filePath ファイルパス
			 * @return 最終更新日時(ファイルが無ければ 0)
			 */
			static time_t GetFileLastWriteTime(const std::string& filePath);


		public:
			/**
			 * @brief 文字列を取得
			 * @param item JSONのオブジェクト
			 * @param key キー
			 * @param defaultValue 無いときの値
			 * @return 文字列
			 */
			static std::string ToString(const nlohmann::json& item, const char* key, const std::string& defaultValue = "");
			/**
			 * @brief floatを取得
			 * @param item JSONのオブジェクト
			 * @param key キー
			 * @param defaultValue 無いときの値
			 * @return float
			 */
			static float ToFloat(const nlohmann::json& item, const char* key, const float defaultValue = 0.0f);
			/**
			 * @brief intを取得
			 * @param item JSONのオブジェクト
			 * @param key キー
			 * @param defaultValue 無いときの値
			 * @return int
			 */
			static int ToInt(const nlohmann::json& item, const char* key, const int defaultValue = 0);
			/**
			 * @brief boolを取得
			 * @param item JSONのオブジェクト
			 * @param key キー
			 * @param defaultValue 無いときの値
			 * @return bool
			 */
			static bool ToBool(const nlohmann::json& item, const char* key, const bool defaultValue = false);
			/**
			 * @brief Vector2を取得
			 * @param item JSONのオブジェクト
			 * @param key キー
			 * @param defaultValue 無いときの値
			 * @return Vector2
			 */
			static Vector2 ToVector2(const nlohmann::json& item, const char* key, const Vector2& defaultValue = Vector2::Zero);
			/**
			 * @brief Vector3を取得
			 * @param item JSONのオブジェクト
			 * @param key キー
			 * @param defaultValue 無いときの値
			 * @return Vector3
			 */
			static Vector3 ToVector3(const nlohmann::json& item, const char* key, const Vector3& defaultValue = Vector3::Zero);
			/**
			 * @brief Vector4を取得
			 * @param item JSONのオブジェクト
			 * @param key キー
			 * @param defaultValue 無いときの値
			 * @return Vector4
			 */
			static Vector4 ToVector4(const nlohmann::json& item, const char* key, const Vector4& defaultValue = Vector4::White);
		};
	}
}
