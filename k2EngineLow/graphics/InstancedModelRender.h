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
	};
}
