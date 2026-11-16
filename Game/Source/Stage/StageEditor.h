/**
 * @file StageEditor.h
 * @brief ステージエディタ(Debug の時だけ使う)
 */
#pragma once
#include <string>
#include <vector>


/** 前方宣言(ImGui) */
struct ImDrawList;


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class StageObject;
	}


	namespace stage
	{
		/** 前方宣言 */
		class Stage;


		/**
		 * @brief ステージエディタ
		 * @details ImGui のウィンドウで置物を選んで、数値かギズモ(ImGuizmo)で動かす。
		 *          1 / 2 / 3 キーで、移動 / 回転 / スケールを切り替える
		 *          ※ W/A/S/D はプレイヤーの移動に使っているので、3ds Max の W/E/R は使わない
		 *
		 *          「4 Views」をオンにすると、真上・正面・横(平行投影)とパースの 4 つの画面を並べた 1 つのウィンドウが出る
		 *          (動かせる・大きさを変えられる。画面は 16:9 のまま、ウィンドウに合わせて大きさが変わる)。
		 *          平行投影の画面：ホイールでズーム、右(中)ドラッグで移動
		 *          パースの画面　：右ドラッグで見回す、右を押したまま WASD(Q/E で下/上、Shift で速く)で動く、
		 *                          Alt + 右ドラッグで回す、中ドラッグで移動、ホイールで近づく・離れる
		 *          どの画面でも、左クリックで選ぶ(Place by click なら置く)
		 *          Delete キーで選んでいる物を消す。Ctrl + Z で 1 つ前に戻す(消した・置いた・動かした)
		 */
		class StageEditor
		{
		public:
			/**
			 * @brief コンストラクタ
			 * @param stage 編集するステージ
			 * @param filePath 保存する JSON のパス
			 * @param tklFilePath 読み直す tkl のパス
			 * @param modelDirectory モデルのフォルダ(最後に / を付ける)
			 */
			StageEditor(Stage* stage, const std::string& filePath, const std::string& tklFilePath, const std::string& modelDirectory);
			~StageEditor();

			/**
			 * @brief 更新処理(ImGui のウィンドウとギズモを出す)
			 * @details ImGui::NewFrame と ImGui::Render の間で呼ぶ(Update の中なら大丈夫)
			 */
			void Update();

			/**
			 * @brief パースの画面で、カメラを飛ばしているか(右クリックを押したまま)
			 * @return 飛ばしているなら true(WASD をカメラに使うので、主人公を動かさないようにする)
			 */
			bool IsCameraFlying() const;


		private:
			/**
			 * @brief 元に戻す操作の種類
			 */
			enum class UndoType : uint8_t
			{
				Add,		// 置いた(戻すと消える)
				Remove,		// 消した(戻すと元の値で作り直す)
				Transform,	// 動かした(戻すと前の位置・回転・スケールにする)
			};

			/**
			 * @brief 置物の中身(消した物を作り直す・動かす前に戻すため)
			 */
			struct ObjectData
			{
				std::string name;
				std::string modelPath;
				Vector3 position;
				Vector3 rotationDeg;
				Vector3 scale;
				bool isCollision = true;
			};

			/**
			 * @brief 元に戻すための記録
			 */
			struct UndoRecord
			{
				/** 操作の種類 */
				UndoType type;
				/** 操作した置物(消した物は、消す前のアドレス。作り直したら、新しいアドレスに置き換える) */
				const actor::StageObject* object;
				/** 操作の前の中身 */
				ObjectData before;
			};


		private:
			/**
			 * @brief 視点の種類
			 * ※ RenderingEngine のサブビューの番号として、そのまま使う
			 */
			enum ViewType : int
			{
				ViewTop,			// 真上(Y の下向きに見る。平行投影)
				ViewFront,			// 正面(Z の奥向きに見る。平行投影)
				ViewSide,			// 横(X の奥向きに見る。平行投影)
				ViewPerspective,	// パース(自由に回せる透視投影)
				ViewCount			// 視点の数
			};

			/**
			 * @brief 1 つの視点の状態
			 */
			struct ViewState
			{
				/**
				 * 見ている中心
				 * 平行投影：見る向きの値は使わず、断面の位置 cut を使う
				 * パース　：回る中心(注視点)
				 */
				Vector3 center;
				/** ズーム(1 ピクセルあたりの長さ。大きいほど広く映る。平行投影だけ) */
				float zoom;
				/** 断面の位置(真上は Y、正面は Z、横は X。カメラをここに置くので、これより手前の物は映らない。平行投影だけ) */
				float cut;
				/** 横の回転(度数。パースだけ) */
				float yawDeg;
				/** 縦の回転(度数。プラスで見下ろす。パースだけ) */
				float pitchDeg;
				/** 注視点からカメラまでの距離(パースだけ) */
				float distance;
				/** 画像の左上の位置(画面のピクセル) */
				float imageX;
				float imageY;
				/** 画像の大きさ(画面のピクセル) */
				float imageWidth;
				float imageHeight;
			};


		private:
			/** ファイルの操作(保存・読み直し・tkl から読み直し) */
			void DrawFileMenu();
			/** 置物の一覧 */
			void DrawObjectList();
			/** ポリゴン数(全体の合計と、モデルごとの内訳) */
			void DrawPolygonStats();
			/** 選んでいる置物の値 */
			void DrawInspector();
			/** 置物の追加(パレットから選んで、追加ボタンかクリックで置く) */
			void DrawAddObject();
			/** ギズモの設定(移動・回転・スケール、ワールド・ローカル、スナップ) */
			void DrawGizmoSetting();
			/** 4 つの画面の設定(開く・閉じる) */
			void DrawViewSetting();
			/** ゲームの画面にギズモを出す(視点にギズモがある時は出さない) */
			void UpdateGizmo();
			/** キーでギズモの操作を切り替える */
			void UpdateShortcut();

			/** 変更があったことを記録する(ウィンドウの名前に * が付く) */
			void MarkDirty() { m_isDirty = true; }


		private:
			/** パレットを作り直す(モデルのフォルダの tkm を集める) */
			void RefreshPalette();
			/** 4 つの画面を並べた 1 つのウィンドウを出す(動かせる・大きさを変えられる) */
			void UpdateViews();
			/**
			 * @brief 1 つの画面(見出し・設定・絵)を出して、マウスの操作を受け取る
			 * @param viewIndex 視点の番号
			 * @param cellWidth 絵の幅(16:9)
			 * @param cellHeight 絵の高さ
			 */
			void UpdateView(const int viewIndex, const float cellWidth, const float cellHeight);
			/**
			 * @brief 平行投影の画面のマウスの操作(ズーム・移動)
			 * @param viewIndex 視点の番号
			 * @param isHovered 絵の上にマウスがあるか
			 */
			void UpdateOrthographicInput(const int viewIndex, const bool isHovered);
			/**
			 * @brief パースの画面のマウスとキーの操作(見回す・WASD で飛ぶ・回す・移動・近づく)
			 * @param isHovered 絵の上にマウスがあるか
			 */
			void UpdatePerspectiveInput(const bool isHovered);
			/**
			 * @brief パースのカメラを、その場で見回す(カメラの位置は変えずに、注視点を動かす)
			 * @param deltaYawDeg 横の回転の量(度数)
			 * @param deltaPitchDeg 縦の回転の量(度数)
			 */
			void LookAroundPerspective(const float deltaYawDeg, const float deltaPitchDeg);
			/**
			 * @brief パースのカメラを、WASD(Q/E)で飛ばす(注視点も一緒に動かす)
			 */
			void FlyPerspective();
			/**
			 * @brief 視点のカメラを決める(平行投影は断面の位置に置く。パースは注視点のまわりに置く)
			 * @param viewIndex 視点の番号
			 */
			void UpdateViewCamera(const int viewIndex);
			/**
			 * @brief 画面の位置を通るレイを作る
			 * @param viewIndex 視点の番号
			 * @param screenX 画面の X(ピクセル)
			 * @param screenY 画面の Y(ピクセル)
			 * @param start レイの始まり(近平面の上。平行投影なら断面の上)
			 * @param end レイの終わり(遠平面の上)
			 */
			void ScreenToRay(const int viewIndex, const float screenX, const float screenY, Vector3& start, Vector3& end);
			/**
			 * @brief クリックした所に、パレットで選んだモデルを置く
			 * @param viewIndex 視点の番号
			 * @param screenX 画面の X(ピクセル)
			 * @param screenY 画面の Y(ピクセル)
			 * @details クリックした所を通るレイを飛ばして、最初に当たった面(床・壁)の上に置く
			 */
			void PlaceObject(const int viewIndex, const float screenX, const float screenY);
			/**
			 * @brief クリックした所の置物を選ぶ
			 * @param viewIndex 視点の番号
			 * @param screenX 画面の X(ピクセル)
			 * @param screenY 画面の Y(ピクセル)
			 * @details 囲む球がクリックした所に重なる置物のうち、一番小さい物を選ぶ(床より上の小物を選びやすく)
			 */
			void PickObject(const int viewIndex, const float screenX, const float screenY);
			/**
			 * @brief 選んでいる置物を、ギズモで動かす
			 * @param viewMatrix ビュー行列
			 * @param projectionMatrix プロジェクション行列
			 * @param drawList ギズモを描く所
			 * @param x 描く範囲の左
			 * @param y 描く範囲の上
			 * @param width 描く範囲の幅
			 * @param height 描く範囲の高さ
			 * @param isOrthographic 平行投影か
			 */
			void ManipulateSelected(const Matrix& viewMatrix, const Matrix& projectionMatrix, ImDrawList* drawList,
				const float x, const float y, const float width, const float height, const bool isOrthographic);


		private:
			/** 選んでいる置物を消す(元に戻せるように記録する) */
			void DeleteSelected();
			/**
			 * @brief 置物を追加した後に、元に戻せるように記録して選ぶ
			 * @param object 追加した置物
			 */
			void OnObjectAdded(const actor::StageObject* object);
			/** 1 つ前に戻す */
			void Undo();
			/** 元に戻す記録を全部消す(読み直しなどで、置物が全部作り直された時) */
			void ClearUndo();
			/**
			 * @brief 元に戻す記録を足す(多すぎたら古い物から消す)
			 * @param record 記録
			 */
			void PushUndo(const UndoRecord& record);
			/**
			 * @brief 動かし終わったら、動かす前の値を記録する
			 * @details ギズモか数値の欄を触っている間を「動かしている」とみなす。触っていない間は、今の値を覚えておく
			 */
			void UpdateTransformUndo();
			/**
			 * @brief 置物の番号を探す
			 * @param object 置物
			 * @return 番号(無ければ -1)
			 */
			int FindObjectIndex(const actor::StageObject* object) const;
			/**
			 * @brief 置物の中身を取り出す
			 * @param object 置物
			 * @return 中身
			 */
			static ObjectData MakeObjectData(const actor::StageObject& object);
			/**
			 * @brief 置物の位置・回転・スケールを、中身の値にする(描画のグループとコリジョンも作り直す)
			 * @param object 置物
			 * @param data 中身
			 */
			void ApplyTransform(actor::StageObject* object, const ObjectData& data);


		private:
			/** 編集するステージ */
			Stage* m_stage;
			/** 保存する JSON のパス */
			std::string m_filePath;
			/** 読み直す tkl のパス */
			std::string m_tklFilePath;
			/** モデルのフォルダ */
			std::string m_modelDirectory;
			/** 選んでいる置物の番号(選んでいなければ -1) */
			int m_selectedIndex;
			/** 保存していない変更があるか */
			bool m_isDirty;
			/** ギズモの操作(ImGuizmo::OPERATION) */
			int m_gizmoOperation;
			/** ギズモの向き(ImGuizmo::MODE) */
			int m_gizmoMode;
			/** スナップするか */
			bool m_isSnap;
			/** 移動のスナップ */
			float m_snapTranslation;
			/** 回転のスナップ(度数) */
			float m_snapRotation;
			/** スケールのスナップ */
			float m_snapScale;
			/** 前のフレームでギズモを使っていたか(放した瞬間にコリジョンを作り直すため) */
			bool m_isGizmoUsingLastFrame;
			/** 一覧の絞り込み */
			char m_filter[64];
			/** 追加するモデルのパス */
			char m_addModelPath[256];
			/** 最後の操作の結果(画面に出す) */
			std::string m_message;

			/** パレット(モデルのフォルダの tkm の一覧) */
			std::vector<std::string> m_paletteList;
			/** パレットを作ったか */
			bool m_isPaletteLoaded;
			/** パレットの絞り込み */
			char m_paletteFilter[64];
			/** 視点の中の左クリックで置くか(false なら選ぶ) */
			bool m_isPlaceMode;

			/** 4 つの画面を出しているか */
			bool m_isViewOpen;
			/** 視点の状態 */
			ViewState m_viewList[ViewCount];
			/** 前のフレームでマウスが乗っていた視点(乗っていなければ -1) */
			int m_hoveredViewIndex;
			/** ギズモを出している視点(-1 ならゲームの画面) */
			int m_gizmoViewIndex;
			/** 右・中ドラッグで動かしている視点(していなければ -1。ドラッグ中に画面の外へ出ても続ける) */
			int m_dragViewIndex;

			/** 元に戻す記録(後ろほど新しい) */
			std::vector<UndoRecord> m_undoList;
			/** 数値の欄(位置・回転・スケール)を触っているか */
			bool m_isInspectorEditing;
			/** 前のフレームで、動かしていたか(ギズモか数値の欄) */
			bool m_isTransformEditing;
			/** 動かす前の置物(触っていない間、選んでいる物を覚えておく) */
			const actor::StageObject* m_transformBeforeObject;
			/** 動かす前の中身 */
			ObjectData m_transformBefore;
		};
	}
}
