#pragma once
#include <string>
#include <DxLib.h>

class EnemyBase
{
public:
	//敵の種類
	enum class TYPE
	{
		BIRD,
		GHOST,
		MAX
	};

	EnemyBase(int baseModelId);	// コンストラクタ
	virtual ~EnemyBase(void);	// デストラクタ

	virtual void Init(void);		// 初期処理(最初の１回のみ実行)
	virtual void SetParam(void);	// パラメータ設定(純粋仮想関数)
	virtual void Update(void);		// 更新処理(毎フレーム実行)
	virtual void Draw(void);		// 描画処理(毎フレーム実行)
	virtual void Release(void);		// 解放処理(最後の１回のみ実行)

	VECTOR GetPos(void);		// 座標の取得
	void SetPos(VECTOR pos);	// 座標の設定

	bool IsAlive(void);			// 生存判定
	void SetAlive(bool alive);	// 生存判定

	void Damage(int damage);	// ダメージを与える

	VECTOR GetCollisionPos(void);	// 衝突用の中心座標の取得
	float GetCollisionRadius(void);	// 衝突用の球体半径の取得
protected:
	int baseModelId_[static_cast<int>(TYPE::MAX)];	// 元となる弾のモデルID
	int modelId_;	// 弾のモデルID

	VECTOR scl_;	// 大きさ
	VECTOR rot_;	// 角度
	VECTOR pos_;	// 表示座標

	float speed_;	// 移動速度
	VECTOR dir_;	// 移動方向

	int hp_;	// 体力
	int hpMax_;	// 体力最大値

	bool isAlive_;	// 生存判定

	int animAttachNo_;		// アニメーションをアタッチ番号
	float animTotalTime_;	// アニメーションの総再生時間
	float stepAnim_;		// 再生中のアニメーション時間
	float speedAnim_;		// アニメーション速度

	float collisionRadius_;		// 衝突判定用の球体半径
	VECTOR collisionLocalPos_;	// 衝突判定用の球体中心の調整座標
};