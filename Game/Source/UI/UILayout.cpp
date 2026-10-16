/**
 * @file UILayout.cpp
 * @brief UIのレイアウト管理
 */
#include "stdafx.h"
#include "UILayout.h"
#include "Source/Util/JsonConverter.h"


namespace
{
	using app::util::JsonConverter;


	/**
	 * @brief どのUIでも使うパラメーター
	 */
	struct UICommonParameter
	{
		Vector3 position;
		Vector3 scale;
		Quaternion rotation;
		Vector4 color;
		Vector2 pivot;
		bool isDraw;
	};


	/**
	 * @brief どのUIでも使うパラメーターを読み込む
	 * @param item UIのJSON
	 * @return パラメーター(書いていないものは既定値)
	 */
	UICommonParameter ReadCommonParameter(const nlohmann::json& item)
	{
		UICommonParameter parameter;
		parameter.position = JsonConverter::ToVector3(item, "position", Vector3::Zero);
		parameter.scale = JsonConverter::ToVector3(item, "scale", Vector3::One);
		parameter.color = JsonConverter::ToVector4(item, "color", Vector4::White);
		parameter.pivot = JsonConverter::ToVector2(item, "pivot", Vector2(0.5f, 0.5f));
		parameter.isDraw = JsonConverter::ToBool(item, "visible", true);

		// 2DなのでZ軸回転だけ(度数法)
		parameter.rotation = Quaternion::Identity;
		parameter.rotation.SetRotationDegZ(JsonConverter::ToFloat(item, "rotation", 0.0f));
		return parameter;
	}


	/**
	 * @brief 画像を使うUI(UIImage / UIIcon / UIGauge)を作る
	 * @tparam T UIの型
	 * @param canvas 作る先のキャンバス
	 * @param key UIのキー
	 * @param item UIのJSON
	 * @return 作ったUI(画像が指定されていなければ nullptr)
	 */
	template <typename T>
	T* CreateImageUI(app::ui::UICanvas* canvas, const uint32_t key, const nlohmann::json& item)
	{
		const std::string asset = JsonConverter::ToString(item, "asset");
		// 画像が無いままスプライトを作ると落ちるので、作らない
		if (asset.empty()) return nullptr;

		const UICommonParameter parameter = ReadCommonParameter(item);
		const float width = JsonConverter::ToFloat(item, "width");
		const float height = JsonConverter::ToFloat(item, "height");

		T* ui = canvas->CreateUI<T>(key);
		ui->Initialize(asset.c_str(), width, height, parameter.position, parameter.scale, parameter.rotation, parameter.color, parameter.pivot);
		ui->SetIsDraw(parameter.isDraw);
		return ui;
	}


	/**
	 * @brief UIを作る
	 * @param canvas 作る先のキャンバス
	 * @param type UIの種類(JSON の "type")
	 * @param key UIのキー
	 * @param item UIのJSON
	 * @return 作ったUI(知らない種類なら nullptr)
	 */
	app::ui::UIBase* CreateUI(app::ui::UICanvas* canvas, const std::string& type, const uint32_t key, const nlohmann::json& item)
	{
		using namespace app::ui;

		if (type == "UIImage")
		{
			return CreateImageUI<UIImage>(canvas, key, item);
		}
		if (type == "UIIcon")
		{
			return CreateImageUI<UIIcon>(canvas, key, item);
		}
		if (type == "UIGauge")
		{
			UIGauge* gauge = CreateImageUI<UIGauge>(canvas, key, item);
			if (gauge) gauge->SetRate(JsonConverter::ToFloat(item, "rate", 1.0f));
			return gauge;
		}
		if (type == "UIDigit")
		{
			const std::string asset = JsonConverter::ToString(item, "asset");
			if (asset.empty()) return nullptr;

			const UICommonParameter parameter = ReadCommonParameter(item);
			const int digit = JsonConverter::ToInt(item, "digit", 1);
			const int number = JsonConverter::ToInt(item, "number", 0);
			const float width = JsonConverter::ToFloat(item, "width");
			const float height = JsonConverter::ToFloat(item, "height");

			UIDigit* ui = canvas->CreateUI<UIDigit>(key);
			ui->Initialize(asset.c_str(), digit, number, width, height, parameter.position, parameter.scale, parameter.rotation);
			ui->SetColor(parameter.color);
			ui->SetPivot(parameter.pivot);
			ui->SetIsDraw(parameter.isDraw);
			return ui;
		}
		if (type == "UIDummy")
		{
			const UICommonParameter parameter = ReadCommonParameter(item);

			UIDummy* ui = canvas->CreateUI<UIDummy>(key);
			ui->SetPosition(parameter.position);
			ui->SetScale(parameter.scale);
			ui->SetRotation(parameter.rotation);
			ui->SetColor(parameter.color);
			return ui;
		}
		return nullptr;
	}
}


namespace app
{
	namespace ui
	{
		UILayout::UILayout()
#ifdef APP_ENABLE_LAYOUT_HOTRELOAD
			: m_lastWriteTime(0)
#endif
		{}


		UILayout::~UILayout()
		{}


		void UILayout::Update()
		{
			if (m_menu == nullptr) return;

#ifdef APP_ENABLE_LAYOUT_HOTRELOAD
			// 作り直したUIも、このフレームの Update を通してから描画されるように、先に確認する
			const time_t writeTime = util::JsonConverter::GetFileLastWriteTime(m_filePath);
			if (writeTime != 0 && writeTime != m_lastWriteTime)
			{
				m_lastWriteTime = writeTime;
				Reload();
			}
#endif

			m_menu->Update();
		}


		void UILayout::Render(RenderContext& rc)
		{
			if (m_menu == nullptr) return;
			m_menu->Render(rc);
		}


		bool UILayout::Reload()
		{
			if (m_menu == nullptr) return false;

			nlohmann::json root;
			if (!util::JsonConverter::LoadFile(m_filePath, root)) return false;

			auto canvasIt = root.find("canvas");
			if (canvasIt == root.end() || !canvasIt->is_object()) return false;
			const nlohmann::json& canvasItem = *canvasIt;

			// JSON から消したUIが残らないように、キャンバスごと作り直す
			m_menu->ResetCanvas();
			UICanvas* canvas = m_menu->GetCanvas();
			canvas->SetPosition(util::JsonConverter::ToVector3(canvasItem, "position", Vector3::Zero));
			canvas->SetScale(util::JsonConverter::ToVector3(canvasItem, "scale", Vector3::One));

			auto elementsIt = canvasItem.find("elements");
			if (elementsIt != canvasItem.end() && elementsIt->is_array())
			{
				for (const nlohmann::json& item : *elementsIt)
				{
					const std::string type = util::JsonConverter::ToString(item, "type");
					const std::string name = util::JsonConverter::ToString(item, "name");
					if (type.empty() || name.empty()) continue;

					// 同じ名前のUIは、最初のものだけ使う(FindUI で区別できないため)
					const uint32_t key = Hash32(name.c_str());
					if (m_menu->HasUI(key)) continue;

					CreateUI(canvas, type, key, item);
				}
			}

			// UIのポインタが変わったので、メニューに持ち直してもらう
			m_menu->InitializeLogic();
			return true;
		}


		void UILayout::InitializeImpl(const std::string& filePath, std::unique_ptr<UIMenuBase> menu)
		{
			m_filePath = filePath;
			m_menu = std::move(menu);

#ifdef APP_ENABLE_LAYOUT_HOTRELOAD
			// 最初の更新日時を覚えて、1フレーム目に読み直さないようにする
			m_lastWriteTime = util::JsonConverter::GetFileLastWriteTime(m_filePath);
#endif

			Reload();
		}
	}
}
