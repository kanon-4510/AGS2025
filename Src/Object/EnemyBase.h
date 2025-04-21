#pragma once
#include <string>
#include <DxLib.h>

class EnemyBase
{
public:

	//敵の種類
	enum class TYPE
	{
		BIRD
		, GHOST
		, MAX
	};


	// コンストラクタ
	EnemyBase(int baseModelId);
	// デストラクタ
	virtual ~EnemyBase(void);

	// 初期処理(最初の１回のみ実行)
	virtual void Init(void);

	// パラメータ設定(純粋仮想関数)
	virtual void SetParam(void);

	// 更新処理(毎フレーム実行)
	virtual void Update(void);

	// 描画処理(毎フレーム実行)
	virtual void Draw(void);

	// 解放処理(最後の１回のみ実行)
	virtual void Release(void);

	// 座標の取得
	VECTOR GetPos(void);

	// 座標の設定
	void SetPos(VECTOR pos);

	// 生存判定
	bool IsAlive(void);

	// 生存判定
	void SetAlive(bool alive);

	// ダメージを与える
	void Damage(int damage);

	// 衝突用の中心座標の取得
	VECTOR GetCollisionPos(void);

	// 衝突用の球体半径の取得
	float GetCollisionRadius(void);

protected:
	// 元となる弾のモデルID
	int baseModelId_[static_cast<int>(TYPE::MAX)];
	// 弾のモデルID
	int modelId_;
	// 大きさ
	VECTOR scl_;
	// 角度
	VECTOR rot_;
	// 表示座標
	VECTOR pos_;

	// 移動速度
	float speed_;

	// 移動方向
	VECTOR dir_;

	// 体力
	int hp_;

	// 体力最大値
	int hpMax_;

	// 生存判定
	bool isAlive_;

	// アニメーションをアタッチ番号
	int animAttachNo_;
	// アニメーションの総再生時間
	float animTotalTime_;
	// 再生中のアニメーション時間
	float stepAnim_;
	// アニメーション速度
	float speedAnim_;

	// 衝突判定用の球体半径
	float collisionRadius_;

	// 衝突判定用の球体中心の調整座標
	VECTOR collisionLocalPos_;

};
