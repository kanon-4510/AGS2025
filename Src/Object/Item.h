#pragma once
#include "ActorBase.h"
#include <memory>
#include <vector>

class Player;
class Collider;

class Item : public ActorBase
{
public:

	//敵の種類
	enum class TYPE
	{
		NONE,
		WATER,	//水
		POWER,	//パワーアップ
		SPEED,	//スピードアップ
		HEAL,	//ヒール
		MUTEKI,	//無敵
		MAX
	};

	// コンストラクタ
	Item(Player& player, const Transform& transform, TYPE itemType);

	// デストラクタ
	~Item(void);

	void Init(void);
	void Update(void);
	void Draw(void);

	VECTOR GetPos(void);		// 座標の取得
	void SetPos(VECTOR pos);	// 座標の設定

	void SetCollisionPos(const VECTOR collision);//衝突判定用の球体
	VECTOR GetCollisionPos(void)const;		// 衝突用の中心座標の取得
	float GetCollisionRadius(void);		// 衝突用の球体半径の取得

	void SetIsAlive(bool isAlive);
	bool GetIsAlive();

	void SetScale(float scale);			//itemのスケールを設定
	void Respawn(const VECTOR& newPos);	//itemを再利用

	void DrawDebug(void);	//デバッグ用
private:
	Player& player_;
	TYPE itemType_;	//ドロップするアイテムの種類

	int modelId_;//モデルの格納

	VECTOR scl_;	// 大きさ
	VECTOR rot_;	// 角度
	VECTOR pos_;	// 表示座標
	VECTOR dir_;	// 移動方向

	bool isAlive_;

	float floatTimer_ = 0.0f;  // アニメーション用のタイマー
	float floatHeight_ = 20.0f; // 上下に動く振幅（高さ）
	float floatSpeed_ = 4.0f;   // 動きの速さ
	float baseY_ = 0.0f;

	float collisionRadius_;		// 衝突判定用の球体半径
	VECTOR collisionLocalPos_;	// 衝突判定用の球体中心の調整座標

	// 衝突判定
	void Collision(void);
	
	void InitModel(void);
};