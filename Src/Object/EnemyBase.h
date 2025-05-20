#pragma once
#include <memory>
#include <string>
#include <vector>
#include <DxLib.h>
#include "ActorBase.h"

class Collider;
class Capsule;
class AnimationController;


class EnemyBase : public ActorBase
{
public:

	//影の大きさ
	static constexpr float ENEMY_SHADOW_SIZE = 30.0f;
	static constexpr float ENEMY_SHADOW_HEIGHT = 300.0f;

	static constexpr float TIME_ROT = 1.0f;	// 回転完了までの時間

	//敵の種類
	enum class TYPE
	{
		BIRD,
		GHOST,
		MAX
	};

	//状態管理
	enum class STATE
	{
		NONE,
		PLAY,
		DEAD,
		MAX
	};

	// アニメーション種別
	enum class ANIM_TYPE
	{
		RUN,
		ATTACK,
		DAMAGE,
		DEATH,
		MAX
	};

	EnemyBase(int baseModelId);	// コンストラクタ
	virtual ~EnemyBase(void);	// デストラクタ

	virtual void Init(void);		// 初期処理(最初の１回のみ実行)
	virtual void SetParam(void);	// パラメータ設定(純粋仮想関数)
	virtual void Update(void);		// 更新処理(毎フレーム実行)
	virtual void EnemyUpdate(void);		// 更新処理(毎フレーム実行)
	virtual void Draw(void);		// 描画処理(毎フレーム実行)
	virtual void Release(void);		// 解放処理(最後の１回のみ実行)

	VECTOR GetPos(void);		// 座標の取得
	void SetPos(VECTOR pos);	// 座標の設定

	bool IsAlive(void);			// 生存判定
	void SetAlive(bool alive);	// 生存判定

	void Damage(int damage);	// ダメージを与える

	const Capsule& GetCapsule(void) const;	// 衝突用カプセルの取得

	void SetCollisionPos(const VECTOR collision);//衝突判定用の球体
	VECTOR GetCollisionPos(void)const;		// 衝突用の中心座標の取得
	float GetCollisionRadius(void);		// 衝突用の球体半径の取得


	void DrawDebug(void);	//デバッグ用

protected:
	int baseModelId_[static_cast<int>(TYPE::MAX)];	// 元となる敵のモデルID
	int modelId_;	// 敵のモデルID

	VECTOR jumpPow_;// ジャンプ量
	float speed_;	// 移動速度
	VECTOR scl_;	// 大きさ
	VECTOR rot_;	// 角度
	VECTOR pos_;	// 表示座標
	VECTOR dir_;	// 移動方向

	VECTOR moveDir_;	// 移動方向
	VECTOR movePow_;	// 移動量
	VECTOR movedPos_;	// 移動後の座標

	VECTOR moveDiff_;	// フレームごとの移動値
	
	VECTOR spherePos_;	//スフィアの移動後座標

	// 回転
	Quaternion enemyRotY_;
	Quaternion goalQuaRot_;
	float stepRotTime_;

	int hp_;	// 体力
	int hpMax_;	// 体力最大値

	bool isAlive_;	// 生存判定

	STATE state_;	//状態管理
	
	int animAttachNo_;		// アニメーションをアタッチ番号
	float animTotalTime_;	// アニメーションの総再生時間
	float stepAnim_;		// 再生中のアニメーション時間
	float speedAnim_;		// アニメーション速度

	float collisionRadius_;		// 衝突判定用の球体半径
	VECTOR collisionLocalPos_;	// 衝突判定用の球体中心の調整座標
	
	std::vector <std::weak_ptr<Collider>> colliders_;// 衝突判定に用いられるコライダ

	std::unique_ptr<Capsule> capsule_;//カプセル

	// 衝突チェック 衝突用線分
	VECTOR gravHitPosDown_;
	VECTOR gravHitPosUp_;

	std::unique_ptr<AnimationController> animationController_;// アニメーション
	
	void InitLoad(void); //アニメーションロード用
	
	void Rotate(void);	 //回転

	void Collision(void);// 衝突判定
};