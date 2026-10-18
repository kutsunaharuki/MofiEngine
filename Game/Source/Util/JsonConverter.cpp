/**
 * @file JsonConverter.cpp
 * @brief JSONの値を、ゲームで使う型に変換する機能群
 */
#include "stdafx.h"
#include "JsonConverter.h"
#include <fstream>
#include <sys/stat.h>


namespace
{
	/**
	 * @brief ベクトルの要素を読み込む
	 * @param value ベクトルのJSON(配列 or オブジェクト)
	 * @param out 読み込んだ要素を入れる配列
	 * @param count 要素数(2〜4)
	 * @details 足りない要素は、out に入っている値のまま残す
	 */
	void ReadVectorElements(const nlohmann::json& value, float* out, const int count)
	{
		static const char* NAMES[] = { "x", "y", "z", "w" };

		for (int i = 0; i < count; ++i)
		{
			const nlohmann::json* element = nullptr;
			if (value.is_array())
			{
				if (i < static_cast<int>(value.size())) element = &value[i];
			}
			else if (value.is_object())
			{
				auto it = value.find(NAMES[i]);
				if (it != value.end()) element = &(*it);
			}

			if (element && element->is_number())
			{
				out[i] = element->get<float>();
			}
		}
	}


	/**
	 * @brief キーの値を探す
	 * @param item JSONのオブジェクト
	 * @param key キー
	 * @return 見つかった値(なければ nullptr)
	 */
	const nlohmann::json* FindValue(const nlohmann::json& item, const char* key)
	{
		if (!item.is_object()) return nullptr;

		auto it = item.find(key);
		if (it == item.end() || it->is_null()) return nullptr;
		return &(*it);
	}
}


namespace app
{
	namespace util
	{
		bool JsonConverter::LoadFile(const std::string& filePath, nlohmann::json& out)
		{
			std::ifstream file(filePath);
			if (!file.is_open()) return false;

			// 書式が間違っていても、例外で落ちないようにする(ホットリロード中の書きかけ対策)
			nlohmann::json json = nlohmann::json::parse(file, nullptr, false);
			if (json.is_discarded()) return false;

			out = std::move(json);
			return true;
		}


		time_t JsonConverter::GetFileLastWriteTime(const std::string& filePath)
		{
			struct _stat64 status;
			if (_stat64(filePath.c_str(), &status) != 0) return 0;
			return status.st_mtime;
		}


		std::string JsonConverter::ToString(const nlohmann::json& item, const char* key, const std::string& defaultValue)
		{
			const nlohmann::json* value = FindValue(item, key);
			if (value == nullptr || !value->is_string()) return defaultValue;
			return value->get<std::string>();
		}


		std::wstring JsonConverter::ToWString(const nlohmann::json& item, const char* key, const std::wstring& defaultValue)
		{
			const nlohmann::json* value = FindValue(item, key);
			if (value == nullptr || !value->is_string()) return defaultValue;

			const std::string& utf8 = value->get_ref<const std::string&>();
			if (utf8.empty()) return std::wstring();

			// 1回目で必要な長さを調べて、2回目で変換する
			const int utf8Length = static_cast<int>(utf8.size());
			const int wideLength = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), utf8Length, nullptr, 0);
			if (wideLength <= 0) return defaultValue;

			std::wstring wide(wideLength, L'\0');
			MultiByteToWideChar(CP_UTF8, 0, utf8.data(), utf8Length, &wide[0], wideLength);
			return wide;
		}


		float JsonConverter::ToFloat(const nlohmann::json& item, const char* key, const float defaultValue)
		{
			const nlohmann::json* value = FindValue(item, key);
			if (value == nullptr || !value->is_number()) return defaultValue;
			return value->get<float>();
		}


		int JsonConverter::ToInt(const nlohmann::json& item, const char* key, const int defaultValue)
		{
			const nlohmann::json* value = FindValue(item, key);
			if (value == nullptr || !value->is_number()) return defaultValue;
			return value->get<int>();
		}


		bool JsonConverter::ToBool(const nlohmann::json& item, const char* key, const bool defaultValue)
		{
			const nlohmann::json* value = FindValue(item, key);
			if (value == nullptr || !value->is_boolean()) return defaultValue;
			return value->get<bool>();
		}


		Vector2 JsonConverter::ToVector2(const nlohmann::json& item, const char* key, const Vector2& defaultValue)
		{
			const nlohmann::json* value = FindValue(item, key);
			if (value == nullptr) return defaultValue;

			float elements[2] = { defaultValue.x, defaultValue.y };
			ReadVectorElements(*value, elements, 2);
			return Vector2(elements[0], elements[1]);
		}


		Vector3 JsonConverter::ToVector3(const nlohmann::json& item, const char* key, const Vector3& defaultValue)
		{
			const nlohmann::json* value = FindValue(item, key);
			if (value == nullptr) return defaultValue;

			float elements[3] = { defaultValue.x, defaultValue.y, defaultValue.z };
			ReadVectorElements(*value, elements, 3);
			return Vector3(elements[0], elements[1], elements[2]);
		}


		Vector4 JsonConverter::ToVector4(const nlohmann::json& item, const char* key, const Vector4& defaultValue)
		{
			const nlohmann::json* value = FindValue(item, key);
			if (value == nullptr) return defaultValue;

			float elements[4] = { defaultValue.x, defaultValue.y, defaultValue.z, defaultValue.w };
			ReadVectorElements(*value, elements, 4);
			return Vector4(elements[0], elements[1], elements[2], elements[3]);
		}
	}
}
