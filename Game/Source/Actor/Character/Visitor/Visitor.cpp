/**
 * @file Visitor.cpp
 * @brief 美術館の訪問者(一般客)
 */
#include "stdafx.h"
#include "Visitor.h"
#include "VisitorStateMachine.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** アニメーションの数 */
			constexpr uint8_t VISITOR_ANIMATION_NUM = static_cast<uint8_t>(Visitor::VisitorAnimation::Max);


			/********** カレン **********/

			/** カレンのアニメーションデータ */
			const AnimationData KAREN_ANIMATION_DATA[] =
			{
					{ "Assets/animData/Visitor/Karen/Idle.tka",     true } // 待機
				,	{ "Assets/animData/Visitor/Karen/Walk.tka",     true } // 歩行
				,	{ "Assets/animData/Visitor/Karen/Talk.tka",     true } // 会話
				,	{ "Assets/animData/Visitor/Karen/WalkTalk.tka", true } // 歩きながら会話
			};
			// KAREN_ANIMATION_DATA の数と、VisitorAnimation の数があっているかをコンパイル時にチェック
			static_assert(sizeof(KAREN_ANIMATION_DATA) / sizeof(KAREN_ANIMATION_DATA[0]) == VISITOR_ANIMATION_NUM,
				"KAREN_ANIMATION_DATA と VisitorAnimation の数が合っていません。");

			/** カレンのモデルデータ */
			const ModelData KAREN_MODEL_DATA =
			{
				"Assets/modelData/Visitor/Karen/Karen.tkm",
				KAREN_ANIMATION_DATA,
				enModelUpAxisZ,
				VISITOR_ANIMATION_NUM
			};


			/********** 会社勤めの男性 **********/

			/** 会社勤めの男性のアニメーションデータ */
			const AnimationData OFFICE_MAN_ANIMATION_DATA[] =
			{
					{ "Assets/animData/Visitor/OfficeMan/Idle.tka",     true } // 待機
				,	{ "Assets/animData/Visitor/OfficeMan/Walk.tka",     true } // 歩行
				,	{ "Assets/animData/Visitor/OfficeMan/Talk.tka",     true } // 会話
				,	{ "Assets/animData/Visitor/OfficeMan/WalkTalk.tka", true } // 歩きながら会話
			};
			// OFFICE_MAN_ANIMATION_DATA の数と、VisitorAnimation の数があっているかをコンパイル時にチェック
			static_assert(sizeof(OFFICE_MAN_ANIMATION_DATA) / sizeof(OFFICE_MAN_ANIMATION_DATA[0]) == VISITOR_ANIMATION_NUM,
				"OFFICE_MAN_ANIMATION_DATA と VisitorAnimation の数が合っていません。");

			/** 会社勤めの男性のモデルデータ */
			const ModelData OFFICE_MAN_MODEL_DATA =
			{
				"Assets/modelData/Visitor/OfficeMan/OfficeMan.tkm",
				OFFICE_MAN_ANIMATION_DATA,
				enModelUpAxisZ,
				VISITOR_ANIMATION_NUM
			};


			/********** アフロの女性 **********/

			/** アフロの女性のアニメーションデータ */
			const AnimationData WOMAN_AFRO_ANIMATION_DATA[] =
			{
					{ "Assets/animData/Visitor/WomanAfro/Idle.tka",     true } // 待機
				,	{ "Assets/animData/Visitor/WomanAfro/Walk.tka",     true } // 歩行
				,	{ "Assets/animData/Visitor/WomanAfro/Talk.tka",     true } // 会話
				,	{ "Assets/animData/Visitor/WomanAfro/WalkTalk.tka", true } // 歩きながら会話
			};
			// WOMAN_AFRO_ANIMATION_DATA の数と、VisitorAnimation の数があっているかをコンパイル時にチェック
			static_assert(sizeof(WOMAN_AFRO_ANIMATION_DATA) / sizeof(WOMAN_AFRO_ANIMATION_DATA[0]) == VISITOR_ANIMATION_NUM,
				"WOMAN_AFRO_ANIMATION_DATA と VisitorAnimation の数が合っていません。");

			/** アフロの女性のモデルデータ */
			const ModelData WOMAN_AFRO_MODEL_DATA =
			{
				"Assets/modelData/Visitor/WomanAfro/WomanAfro.tkm",
				WOMAN_AFRO_ANIMATION_DATA,
				enModelUpAxisZ,
				VISITOR_ANIMATION_NUM
			};


			/********** アジア系の女性 **********/

			/** アジア系の女性のアニメーションデータ */
			const AnimationData WOMAN_ASIAN_ANIMATION_DATA[] =
			{
					{ "Assets/animData/Visitor/WomanAsian/Idle.tka",     true } // 待機
				,	{ "Assets/animData/Visitor/WomanAsian/Walk.tka",     true } // 歩行
				,	{ "Assets/animData/Visitor/WomanAsian/Talk.tka",     true } // 会話
				,	{ "Assets/animData/Visitor/WomanAsian/WalkTalk.tka", true } // 歩きながら会話
			};
			// WOMAN_ASIAN_ANIMATION_DATA の数と、VisitorAnimation の数があっているかをコンパイル時にチェック
			static_assert(sizeof(WOMAN_ASIAN_ANIMATION_DATA) / sizeof(WOMAN_ASIAN_ANIMATION_DATA[0]) == VISITOR_ANIMATION_NUM,
				"WOMAN_ASIAN_ANIMATION_DATA と VisitorAnimation の数が合っていません。");

			/** アジア系の女性のモデルデータ */
			const ModelData WOMAN_ASIAN_MODEL_DATA =
			{
				"Assets/modelData/Visitor/WomanAsian/WomanAsian.tkm",
				WOMAN_ASIAN_ANIMATION_DATA,
				enModelUpAxisZ,
				VISITOR_ANIMATION_NUM
			};


			/********** ヨーロッパ系の女性 **********/

			/** ヨーロッパ系の女性のアニメーションデータ */
			const AnimationData WOMAN_EURO_ANIMATION_DATA[] =
			{
					{ "Assets/animData/Visitor/WomanEuro/Idle.tka",     true } // 待機
				,	{ "Assets/animData/Visitor/WomanEuro/Walk.tka",     true } // 歩行
				,	{ "Assets/animData/Visitor/WomanEuro/Talk.tka",     true } // 会話
				,	{ "Assets/animData/Visitor/WomanEuro/WalkTalk.tka", true } // 歩きながら会話
			};
			// WOMAN_EURO_ANIMATION_DATA の数と、VisitorAnimation の数があっているかをコンパイル時にチェック
			static_assert(sizeof(WOMAN_EURO_ANIMATION_DATA) / sizeof(WOMAN_EURO_ANIMATION_DATA[0]) == VISITOR_ANIMATION_NUM,
				"WOMAN_EURO_ANIMATION_DATA と VisitorAnimation の数が合っていません。");

			/** ヨーロッパ系の女性のモデルデータ */
			const ModelData WOMAN_EURO_MODEL_DATA =
			{
				"Assets/modelData/Visitor/WomanEuro/WomanEuro.tkm",
				WOMAN_EURO_ANIMATION_DATA,
				enModelUpAxisZ,
				VISITOR_ANIMATION_NUM
			};


			/****************************/

			/** 訪問者(一般客)のモデルデータの一覧(VisitorType と同じ順番) */
			const ModelData* VISITOR_MODEL_DATA_LIST[] =
			{
					&KAREN_MODEL_DATA		// カレン
				,	&OFFICE_MAN_MODEL_DATA	// 会社勤めの男性
				,	&WOMAN_AFRO_MODEL_DATA	// アフロの女性
				,	&WOMAN_ASIAN_MODEL_DATA	// アジア系の女性
				,	&WOMAN_EURO_MODEL_DATA	// ヨーロッパ系の女性
			};

			// VISITOR_MODEL_DATA_LIST の数と、VisitorType の数があっているかをコンパイル時にチェック
			static_assert(sizeof(VISITOR_MODEL_DATA_LIST) / sizeof(VISITOR_MODEL_DATA_LIST[0]) == static_cast<size_t>(Visitor::VisitorType::Max),
				"VISITOR_MODEL_DATA_LIST と VisitorType の数が合っていません。");

			/** パラメーターのリスト */
			const char* VISITOR_PARAMETER_JSON_LIST[] =
			{
					"Assets/parameter/Actor/Character/Visitor/Karen/KarenParameter.json"
				,	"Assets/parameter/Actor/Character/Visitor/OfficeMan/OfficeManParameter.json"
				,	"Assets/parameter/Actor/Character/Visitor/WomanAfro/WomanAfroParameter.json"
				,	"Assets/parameter/Actor/Character/Visitor/WomanAsian/WomanAsianParameter.json"
				,	"Assets/parameter/Actor/Character/Visitor/WomanEuro/WomanEuroParameter.json"
			};

			// 追加漏れがあるとコンパイル時にエラーが出る
			static_assert(sizeof(VISITOR_PARAMETER_JSON_LIST) / sizeof(VISITOR_PARAMETER_JSON_LIST[0]) == static_cast<size_t>(Visitor::VisitorType::Max),
				"VISITOR_PARAMETER_JSON_LIST と VisitorType の数が合っていません。");
		}


		Visitor::Visitor(const VisitorType type)
			: m_type(type)
			, m_moveDirection(Vector3::Zero)
			, m_moveAmount(0.0f)
			, m_isTalking(false)
			, m_lookDirection(Vector3::Zero)
		{}
		
		
		Visitor::~Visitor()
		{}


		bool Visitor::Start()
		{
			// 訪問者(特徴を持っている人) パラメーターを読み込む
			m_parameter.Initialize(VISITOR_PARAMETER_JSON_LIST[static_cast<int>(m_type)]);
			const VisitorParameter& parameter = m_parameter.Get();

			// 初期スケールを設定
			SetScale(parameter.scale);

			// モデルとアニメーションを読み込む
			Init(*VISITOR_MODEL_DATA_LIST[static_cast<int>(m_type)]);

			// 訪問者(一般客)のステートマシンを作成
			m_characterStateMachine = std::make_unique<VisitorStateMachine>(this);

			return CharacterBase::Start();
		}
		
		
		void Visitor::Update()
		{
			// 訪問者のパラメーターを取得
			// ※ Get() を呼ぶと、JSON が変わっていれば読み直す(ホットリロード)
			const VisitorParameter& parameter = m_parameter.Get();

			// JSON でスケールを変えるとすぐ反映
			SetScale(parameter.scale);

			CharacterBase::Update();
		}
	}
}