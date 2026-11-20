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


			/********** カップル(1) **********/
			// ※ アニメーションがまだ無いので、VISITOR_MODEL_DATA_LIST では今あるモデルを仮に指している。
			//    animData/Visitor/CoupleA/ に 4 つ置いたら、一覧の行を &COUPLE_A_MODEL_DATA に差し替える

			/** カップル(1)のアニメーションデータ */
			const AnimationData COUPLE_A_ANIMATION_DATA[] =
			{
					{ "Assets/animData/Visitor/CoupleA/Idle.tka",     true } // 待機
				,	{ "Assets/animData/Visitor/CoupleA/Walk.tka",     true } // 歩行
				,	{ "Assets/animData/Visitor/CoupleA/Talk.tka",     true } // 会話
				,	{ "Assets/animData/Visitor/CoupleA/WalkTalk.tka", true } // 歩きながら会話
			};
			static_assert(sizeof(COUPLE_A_ANIMATION_DATA) / sizeof(COUPLE_A_ANIMATION_DATA[0]) == VISITOR_ANIMATION_NUM,
				"COUPLE_A_ANIMATION_DATA と VisitorAnimation の数が合っていません。");

			/** カップル(1)のモデルデータ */
			const ModelData COUPLE_A_MODEL_DATA =
			{
				"Assets/modelData/Visitor/Couple/CouplePerson01.tkm",
				COUPLE_A_ANIMATION_DATA,
				enModelUpAxisZ,
				VISITOR_ANIMATION_NUM
			};


			/********** カップル(2) **********/
			// ※ アニメーションがまだ無いので、VISITOR_MODEL_DATA_LIST では今あるモデルを仮に指している。
			//    animData/Visitor/CoupleB/ に 4 つ置いたら、一覧の行を &COUPLE_B_MODEL_DATA に差し替える

			/** カップル(2)のアニメーションデータ */
			const AnimationData COUPLE_B_ANIMATION_DATA[] =
			{
					{ "Assets/animData/Visitor/CoupleB/Idle.tka",     true } // 待機
				,	{ "Assets/animData/Visitor/CoupleB/Walk.tka",     true } // 歩行
				,	{ "Assets/animData/Visitor/CoupleB/Talk.tka",     true } // 会話
				,	{ "Assets/animData/Visitor/CoupleB/WalkTalk.tka", true } // 歩きながら会話
			};
			static_assert(sizeof(COUPLE_B_ANIMATION_DATA) / sizeof(COUPLE_B_ANIMATION_DATA[0]) == VISITOR_ANIMATION_NUM,
				"COUPLE_B_ANIMATION_DATA と VisitorAnimation の数が合っていません。");

			/** カップル(2)のモデルデータ */
			const ModelData COUPLE_B_MODEL_DATA =
			{
				"Assets/modelData/Visitor/Couple/CouplePerson02.tkm",
				COUPLE_B_ANIMATION_DATA,
				enModelUpAxisZ,
				VISITOR_ANIMATION_NUM
			};


			/********** カップル(3) **********/
			// ※ アニメーションがまだ無いので、VISITOR_MODEL_DATA_LIST では今あるモデルを仮に指している。
			//    animData/Visitor/CoupleC/ に 4 つ置いたら、一覧の行を &COUPLE_C_MODEL_DATA に差し替える

			/** カップル(3)のアニメーションデータ */
			const AnimationData COUPLE_C_ANIMATION_DATA[] =
			{
					{ "Assets/animData/Visitor/CoupleC/Idle.tka",     true } // 待機
				,	{ "Assets/animData/Visitor/CoupleC/Walk.tka",     true } // 歩行
				,	{ "Assets/animData/Visitor/CoupleC/Talk.tka",     true } // 会話
				,	{ "Assets/animData/Visitor/CoupleC/WalkTalk.tka", true } // 歩きながら会話
			};
			static_assert(sizeof(COUPLE_C_ANIMATION_DATA) / sizeof(COUPLE_C_ANIMATION_DATA[0]) == VISITOR_ANIMATION_NUM,
				"COUPLE_C_ANIMATION_DATA と VisitorAnimation の数が合っていません。");

			/** カップル(3)のモデルデータ */
			const ModelData COUPLE_C_MODEL_DATA =
			{
				"Assets/modelData/Visitor/Couple/CouplePerson03.tkm",
				COUPLE_C_ANIMATION_DATA,
				enModelUpAxisZ,
				VISITOR_ANIMATION_NUM
			};


			/********** カップル(4) **********/
			// ※ アニメーションがまだ無いので、VISITOR_MODEL_DATA_LIST では今あるモデルを仮に指している。
			//    animData/Visitor/CoupleD/ に 4 つ置いたら、一覧の行を &COUPLE_D_MODEL_DATA に差し替える

			/** カップル(4)のアニメーションデータ */
			const AnimationData COUPLE_D_ANIMATION_DATA[] =
			{
					{ "Assets/animData/Visitor/CoupleD/Idle.tka",     true } // 待機
				,	{ "Assets/animData/Visitor/CoupleD/Walk.tka",     true } // 歩行
				,	{ "Assets/animData/Visitor/CoupleD/Talk.tka",     true } // 会話
				,	{ "Assets/animData/Visitor/CoupleD/WalkTalk.tka", true } // 歩きながら会話
			};
			static_assert(sizeof(COUPLE_D_ANIMATION_DATA) / sizeof(COUPLE_D_ANIMATION_DATA[0]) == VISITOR_ANIMATION_NUM,
				"COUPLE_D_ANIMATION_DATA と VisitorAnimation の数が合っていません。");

			/** カップル(4)のモデルデータ */
			const ModelData COUPLE_D_MODEL_DATA =
			{
				"Assets/modelData/Visitor/Couple/CouplePerson04.tkm",
				COUPLE_D_ANIMATION_DATA,
				enModelUpAxisZ,
				VISITOR_ANIMATION_NUM
			};


			/********** カップル(5) **********/
			// ※ アニメーションがまだ無いので、VISITOR_MODEL_DATA_LIST では今あるモデルを仮に指している。
			//    animData/Visitor/CoupleE/ に 4 つ置いたら、一覧の行を &COUPLE_E_MODEL_DATA に差し替える

			/** カップル(5)のアニメーションデータ */
			const AnimationData COUPLE_E_ANIMATION_DATA[] =
			{
					{ "Assets/animData/Visitor/CoupleE/Idle.tka",     true } // 待機
				,	{ "Assets/animData/Visitor/CoupleE/Walk.tka",     true } // 歩行
				,	{ "Assets/animData/Visitor/CoupleE/Talk.tka",     true } // 会話
				,	{ "Assets/animData/Visitor/CoupleE/WalkTalk.tka", true } // 歩きながら会話
			};
			static_assert(sizeof(COUPLE_E_ANIMATION_DATA) / sizeof(COUPLE_E_ANIMATION_DATA[0]) == VISITOR_ANIMATION_NUM,
				"COUPLE_E_ANIMATION_DATA と VisitorAnimation の数が合っていません。");

			/** カップル(5)のモデルデータ */
			const ModelData COUPLE_E_MODEL_DATA =
			{
				"Assets/modelData/Visitor/Couple/CouplePerson05.tkm",
				COUPLE_E_ANIMATION_DATA,
				enModelUpAxisZ,
				VISITOR_ANIMATION_NUM
			};


			/********** 家族の大人(1) **********/
			// ※ アニメーションがまだ無いので、VISITOR_MODEL_DATA_LIST では今あるモデルを仮に指している。
			//    animData/Visitor/FamilyWomanA/ に 4 つ置いたら、一覧の行を &FAMILY_WOMAN_A_MODEL_DATA に差し替える

			/** 家族の大人(1)のアニメーションデータ */
			const AnimationData FAMILY_WOMAN_A_ANIMATION_DATA[] =
			{
					{ "Assets/animData/Visitor/FamilyWomanA/Idle.tka",     true } // 待機
				,	{ "Assets/animData/Visitor/FamilyWomanA/Walk.tka",     true } // 歩行
				,	{ "Assets/animData/Visitor/FamilyWomanA/Talk.tka",     true } // 会話
				,	{ "Assets/animData/Visitor/FamilyWomanA/WalkTalk.tka", true } // 歩きながら会話
			};
			static_assert(sizeof(FAMILY_WOMAN_A_ANIMATION_DATA) / sizeof(FAMILY_WOMAN_A_ANIMATION_DATA[0]) == VISITOR_ANIMATION_NUM,
				"FAMILY_WOMAN_A_ANIMATION_DATA と VisitorAnimation の数が合っていません。");

			/** 家族の大人(1)のモデルデータ */
			const ModelData FAMILY_WOMAN_A_MODEL_DATA =
			{
				"Assets/modelData/Visitor/Family/family_models/family_models/HumanoWoman01/HumanoWoman01.tkm",
				FAMILY_WOMAN_A_ANIMATION_DATA,
				enModelUpAxisZ,
				VISITOR_ANIMATION_NUM
			};


			/********** 家族の大人(2) **********/
			// ※ アニメーションがまだ無いので、VISITOR_MODEL_DATA_LIST では今あるモデルを仮に指している。
			//    animData/Visitor/FamilyWomanB/ に 4 つ置いたら、一覧の行を &FAMILY_WOMAN_B_MODEL_DATA に差し替える

			/** 家族の大人(2)のアニメーションデータ */
			const AnimationData FAMILY_WOMAN_B_ANIMATION_DATA[] =
			{
					{ "Assets/animData/Visitor/FamilyWomanB/Idle.tka",     true } // 待機
				,	{ "Assets/animData/Visitor/FamilyWomanB/Walk.tka",     true } // 歩行
				,	{ "Assets/animData/Visitor/FamilyWomanB/Talk.tka",     true } // 会話
				,	{ "Assets/animData/Visitor/FamilyWomanB/WalkTalk.tka", true } // 歩きながら会話
			};
			static_assert(sizeof(FAMILY_WOMAN_B_ANIMATION_DATA) / sizeof(FAMILY_WOMAN_B_ANIMATION_DATA[0]) == VISITOR_ANIMATION_NUM,
				"FAMILY_WOMAN_B_ANIMATION_DATA と VisitorAnimation の数が合っていません。");

			/** 家族の大人(2)のモデルデータ */
			const ModelData FAMILY_WOMAN_B_MODEL_DATA =
			{
				"Assets/modelData/Visitor/Family/family_models/family_models/HumanoWoman02/HumanoWoman02.tkm",
				FAMILY_WOMAN_B_ANIMATION_DATA,
				enModelUpAxisZ,
				VISITOR_ANIMATION_NUM
			};


			/********** 家族の大人(3) **********/
			// ※ アニメーションがまだ無いので、VISITOR_MODEL_DATA_LIST では今あるモデルを仮に指している。
			//    animData/Visitor/FamilyWomanC/ に 4 つ置いたら、一覧の行を &FAMILY_WOMAN_C_MODEL_DATA に差し替える

			/** 家族の大人(3)のアニメーションデータ */
			const AnimationData FAMILY_WOMAN_C_ANIMATION_DATA[] =
			{
					{ "Assets/animData/Visitor/FamilyWomanC/Idle.tka",     true } // 待機
				,	{ "Assets/animData/Visitor/FamilyWomanC/Walk.tka",     true } // 歩行
				,	{ "Assets/animData/Visitor/FamilyWomanC/Talk.tka",     true } // 会話
				,	{ "Assets/animData/Visitor/FamilyWomanC/WalkTalk.tka", true } // 歩きながら会話
			};
			static_assert(sizeof(FAMILY_WOMAN_C_ANIMATION_DATA) / sizeof(FAMILY_WOMAN_C_ANIMATION_DATA[0]) == VISITOR_ANIMATION_NUM,
				"FAMILY_WOMAN_C_ANIMATION_DATA と VisitorAnimation の数が合っていません。");

			/** 家族の大人(3)のモデルデータ */
			const ModelData FAMILY_WOMAN_C_MODEL_DATA =
			{
				"Assets/modelData/Visitor/Family/family_models/family_models/HumanoWoman03/HumanoWoman03.tkm",
				FAMILY_WOMAN_C_ANIMATION_DATA,
				enModelUpAxisZ,
				VISITOR_ANIMATION_NUM
			};


			/********** 家族の大人(4) **********/
			// ※ アニメーションがまだ無いので、VISITOR_MODEL_DATA_LIST では今あるモデルを仮に指している。
			//    animData/Visitor/FamilyWomanD/ に 4 つ置いたら、一覧の行を &FAMILY_WOMAN_D_MODEL_DATA に差し替える

			/** 家族の大人(4)のアニメーションデータ */
			const AnimationData FAMILY_WOMAN_D_ANIMATION_DATA[] =
			{
					{ "Assets/animData/Visitor/FamilyWomanD/Idle.tka",     true } // 待機
				,	{ "Assets/animData/Visitor/FamilyWomanD/Walk.tka",     true } // 歩行
				,	{ "Assets/animData/Visitor/FamilyWomanD/Talk.tka",     true } // 会話
				,	{ "Assets/animData/Visitor/FamilyWomanD/WalkTalk.tka", true } // 歩きながら会話
			};
			static_assert(sizeof(FAMILY_WOMAN_D_ANIMATION_DATA) / sizeof(FAMILY_WOMAN_D_ANIMATION_DATA[0]) == VISITOR_ANIMATION_NUM,
				"FAMILY_WOMAN_D_ANIMATION_DATA と VisitorAnimation の数が合っていません。");

			/** 家族の大人(4)のモデルデータ */
			const ModelData FAMILY_WOMAN_D_MODEL_DATA =
			{
				"Assets/modelData/Visitor/Family/family_models/family_models/HumanoWoman04/HumanoWoman04.tkm",
				FAMILY_WOMAN_D_ANIMATION_DATA,
				enModelUpAxisZ,
				VISITOR_ANIMATION_NUM
			};


			/********** 家族の子供 **********/
			// ※ アニメーションがまだ無いので、VISITOR_MODEL_DATA_LIST では今あるモデルを仮に指している。
			//    animData/Visitor/FamilyBoy/ に 4 つ置いたら、一覧の行を &FAMILY_BOY_MODEL_DATA に差し替える

			/** 家族の子供のアニメーションデータ */
			const AnimationData FAMILY_BOY_ANIMATION_DATA[] =
			{
					{ "Assets/animData/Visitor/FamilyBoy/Idle.tka",     true } // 待機
				,	{ "Assets/animData/Visitor/FamilyBoy/Walk.tka",     true } // 歩行
				,	{ "Assets/animData/Visitor/FamilyBoy/Talk.tka",     true } // 会話
				,	{ "Assets/animData/Visitor/FamilyBoy/WalkTalk.tka", true } // 歩きながら会話
			};
			static_assert(sizeof(FAMILY_BOY_ANIMATION_DATA) / sizeof(FAMILY_BOY_ANIMATION_DATA[0]) == VISITOR_ANIMATION_NUM,
				"FAMILY_BOY_ANIMATION_DATA と VisitorAnimation の数が合っていません。");

			/** 家族の子供のモデルデータ */
			const ModelData FAMILY_BOY_MODEL_DATA =
			{
				"Assets/modelData/Visitor/Family/family_models/family_models/LittleBoy/LittleBoy.tkm",
				FAMILY_BOY_ANIMATION_DATA,
				enModelUpAxisZ,
				VISITOR_ANIMATION_NUM
			};


			/********** 学生 **********/
			// ※ アニメーションがまだ無いので、VISITOR_MODEL_DATA_LIST では今あるモデルを仮に指している。
			//    animData/Visitor/Student/ に 4 つ置いたら、一覧の行を &STUDENT_MODEL_DATA に差し替える

			/** 学生のアニメーションデータ */
			const AnimationData STUDENT_ANIMATION_DATA[] =
			{
					{ "Assets/animData/Visitor/Student/Idle.tka",     true } // 待機
				,	{ "Assets/animData/Visitor/Student/Walk.tka",     true } // 歩行
				,	{ "Assets/animData/Visitor/Student/Talk.tka",     true } // 会話
				,	{ "Assets/animData/Visitor/Student/WalkTalk.tka", true } // 歩きながら会話
			};
			static_assert(sizeof(STUDENT_ANIMATION_DATA) / sizeof(STUDENT_ANIMATION_DATA[0]) == VISITOR_ANIMATION_NUM,
				"STUDENT_ANIMATION_DATA と VisitorAnimation の数が合っていません。");

			/** 学生のモデルデータ */
			const ModelData STUDENT_MODEL_DATA =
			{
				"Assets/modelData/Visitor/Student/student_models (1)/student_models/YoungBoy/YoungBoy.tkm",
				STUDENT_ANIMATION_DATA,
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
				,	&OFFICE_MAN_MODEL_DATA	// カップル(1)  TODO: アニメーションが来たら &COUPLE_A_MODEL_DATA に
				,	&WOMAN_EURO_MODEL_DATA	// カップル(2)  TODO: アニメーションが来たら &COUPLE_B_MODEL_DATA に
				,	&KAREN_MODEL_DATA	// カップル(3)  TODO: アニメーションが来たら &COUPLE_C_MODEL_DATA に
				,	&WOMAN_ASIAN_MODEL_DATA	// カップル(4)  TODO: アニメーションが来たら &COUPLE_D_MODEL_DATA に
				,	&WOMAN_AFRO_MODEL_DATA	// カップル(5)  TODO: アニメーションが来たら &COUPLE_E_MODEL_DATA に
				,	&WOMAN_ASIAN_MODEL_DATA	// 家族の大人(1)  TODO: アニメーションが来たら &FAMILY_WOMAN_A_MODEL_DATA に
				,	&WOMAN_EURO_MODEL_DATA	// 家族の大人(2)  TODO: アニメーションが来たら &FAMILY_WOMAN_B_MODEL_DATA に
				,	&WOMAN_AFRO_MODEL_DATA	// 家族の大人(3)  TODO: アニメーションが来たら &FAMILY_WOMAN_C_MODEL_DATA に
				,	&KAREN_MODEL_DATA	// 家族の大人(4)  TODO: アニメーションが来たら &FAMILY_WOMAN_D_MODEL_DATA に
				,	&OFFICE_MAN_MODEL_DATA	// 家族の子供  TODO: アニメーションが来たら &FAMILY_BOY_MODEL_DATA に
				,	&OFFICE_MAN_MODEL_DATA	// 学生  TODO: アニメーションが来たら &STUDENT_MODEL_DATA に
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
				,	"Assets/parameter/Actor/Character/Visitor/CoupleA/CoupleAParameter.json"
				,	"Assets/parameter/Actor/Character/Visitor/CoupleB/CoupleBParameter.json"
				,	"Assets/parameter/Actor/Character/Visitor/CoupleC/CoupleCParameter.json"
				,	"Assets/parameter/Actor/Character/Visitor/CoupleD/CoupleDParameter.json"
				,	"Assets/parameter/Actor/Character/Visitor/CoupleE/CoupleEParameter.json"
				,	"Assets/parameter/Actor/Character/Visitor/FamilyWomanA/FamilyWomanAParameter.json"
				,	"Assets/parameter/Actor/Character/Visitor/FamilyWomanB/FamilyWomanBParameter.json"
				,	"Assets/parameter/Actor/Character/Visitor/FamilyWomanC/FamilyWomanCParameter.json"
				,	"Assets/parameter/Actor/Character/Visitor/FamilyWomanD/FamilyWomanDParameter.json"
				,	"Assets/parameter/Actor/Character/Visitor/FamilyBoy/FamilyBoyParameter.json"
				,	"Assets/parameter/Actor/Character/Visitor/Student/StudentParameter.json"
			};

			/** 見た目の名前(VisitorType と同じ順番・同じ綴り。JSON で見た目を書く時に使う) */
			const char* VISITOR_TYPE_NAME_LIST[] =
			{
					"Karen"
				,	"OfficeMan"
				,	"WomanAfro"
				,	"WomanAsian"
				,	"WomanEuro"
				,	"CoupleA"
				,	"CoupleB"
				,	"CoupleC"
				,	"CoupleD"
				,	"CoupleE"
				,	"FamilyWomanA"
				,	"FamilyWomanB"
				,	"FamilyWomanC"
				,	"FamilyWomanD"
				,	"FamilyBoy"
				,	"Student"
			};

			static_assert(sizeof(VISITOR_TYPE_NAME_LIST) / sizeof(VISITOR_TYPE_NAME_LIST[0]) == static_cast<size_t>(Visitor::VisitorType::Max),
				"VISITOR_TYPE_NAME_LIST と VisitorType の数が合っていません。");

			// 追加漏れがあるとコンパイル時にエラーが出る
			static_assert(sizeof(VISITOR_PARAMETER_JSON_LIST) / sizeof(VISITOR_PARAMETER_JSON_LIST[0]) == static_cast<size_t>(Visitor::VisitorType::Max),
				"VISITOR_PARAMETER_JSON_LIST と VisitorType の数が合っていません。");
		}


		bool Visitor::ToVisitorType(const std::string& name, VisitorType& outType)
		{
			for (int i = 0; i < static_cast<int>(VisitorType::Max); ++i)
			{
				if (name != VISITOR_TYPE_NAME_LIST[i]) continue;

				outType = static_cast<VisitorType>(i);
				return true;
			}
			return false;
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