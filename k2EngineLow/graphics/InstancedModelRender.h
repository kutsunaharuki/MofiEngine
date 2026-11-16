/**
 * @file InstancedModelRender.h
 * @brief インスタンス化モデルの描画
 */
#pragma once


namespace nsK2EngineLow
{
	/**
	 * @brief 1種類のモデルを行列のリストでまとめて描く(インスタンシング描画)
	 * @details 同じモデルを何個置いても、Model(GPU リソース一式)は deferred 用と finder 用の 2 つだけ作る。
	 *          置く場所はワールド行列のリスト(StructuredBuffer)で渡し、頂点シェーダー(VSMainInstancing)が
	 *          SV_InstanceID 番目の行列を読んで、1回の描画命令でまとめて描く
	 *
	 * 使い方(毎フレーム)
	 *   ClearInstance() → 映っている物だけ AddInstance(ワールド行列) → Draw(rc)
	 */
	class InstancedModelRender : public Noncopyable
	{
	public:
		InstancedModelRender();
		~InstancedModelRender();

		/**
		 * @brief 初期化
		 * @param tkmFilePath モデルのファイルパス
		 * @param maxInstance 最大インスタンス数(行列リストの大きさ)
		 * @param upAxis モデルの上方向の軸
		 * @param cullMode カリングのモード(反転したモデルは D3D12_CULL_MODE_FRONT)
		 * @details もう一度呼ぶと、Model と行列リストを作り直す(最大インスタンス数を増やす時に使う)
		 */
		void Init(const char* tkmFilePath, int maxInstance, EnModelUpAxis upAxis, D3D12_CULL_MODE cullMode);
		/**
		 * @brief フレームの最初に空にする
		 */
		void ClearInstance();
		/**
		 * @brief 映っている行列を追加
		 * @param worldMatrix ワールド行列
		 * @details 最大インスタンス数を超えた分は描かない
		 */
		void AddInstance(const Matrix& worldMatrix);
		/**
		 * @brief サブビュー用の行列を追加(映る範囲で絞らずに、全部追加する)
		 * @param worldMatrix ワールド行列
		 * @details サブビューを使っている時だけ呼ぶ。最大インスタンス数を超えた分は描かない
		 */
		void AddSubViewInstance(const Matrix& worldMatrix);
		/**
		 * @brief 描画(行列を GPU に送って、RenderingEngine に登録する)
		 * @param rc 描画コンテキスト
		 */
		void Draw(RenderContext& rc);
		/**
		 * @brief モデルを取得
		 * @return モデル(tkm とワールド行列の計算、コリジョンの作成に使う)
		 */
		Model& GetModel() { return *m_deferredModel; }
		/**
		 * @brief モデルを取得(const版)
		 * @return モデル
		 */
		const Model& GetModel() const { return *m_deferredModel; }
		/**
		 * @brief 最大インスタンス数を取得
		 * @return 最大インスタンス数
		 */
		int GetMaxInstance() const { return m_maxInstance; }


	private:
		/** サブビューの数(RenderingEngine::MAX_SUB_VIEW と同じにする。.cpp で確かめる) */
		static const int SUB_VIEW_COUNT = 4;

		/**
		 * @brief サブビュー用のモデルと行列リストを作る
		 * @details サブビューを使い始めた時に、初めて作る(使わない時は、読み込みも VRAM も増やさない)
		 */
		void InitSubViewModels();


	private:
		/** モデルのファイルパス(サブビュー用のモデルをあとから作るために覚えておく) */
		std::string m_tkmFilePath;
		/** モデルの上方向の軸 */
		EnModelUpAxis m_upAxis;
		/** カリングのモード */
		D3D12_CULL_MODE m_cullMode;

		/**
		 * ディファードモデル
		 * ※ 作り直す時に丸ごと新しくするので unique_ptr で持つ(Model は Init を2回呼べない)
		 */
		std::unique_ptr<Model> m_deferredModel;
		/** ファインダーモデル */
		std::unique_ptr<Model> m_finderModel;
		/** 拡張SRVの4番にインスタンスデータを格納(t14) */
		StructuredBuffer m_worldMatrixSB;
		/**
		 * 今フレームの行列(CPU側)
		 * ※ StructuredBuffer::Update は最大数ぶんコピーするので、大きさは最大インスタンス数で固定する
		 */
		std::vector<Matrix> m_worldMatrices;
		/** 今フレームのインスタンスの数 */
		int m_numInstance;
		/** 最大インスタンス数 */
		int m_maxInstance;

		/**
		 * サブビュー用のモデル(サブビューごとに 1 つ)
		 * ※ Model の行列の定数バッファは 1 つしかないので、同じフレームに別のカメラで描くなら Model を分ける
		 */
		std::unique_ptr<Model> m_subViewModels[SUB_VIEW_COUNT];
		/** サブビュー用の行列リスト(映る範囲で絞らない全部。全部のサブビューで共有する) */
		StructuredBuffer m_subViewMatrixSB;
		/** 今フレームのサブビュー用の行列(CPU側) */
		std::vector<Matrix> m_subViewMatrices;
		/** 今フレームのサブビュー用のインスタンスの数 */
		int m_numSubViewInstance;
		/** サブビュー用のモデルを作ったか */
		bool m_isSubViewModelCreated;
	};
}
