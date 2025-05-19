#pragma once
#include "ActorBase.h"
#include <memory>
#include <vector>

class Collider;

class Item : public ActorBase
{
public:
	// コンストラクタ
	Item(void);

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

	void DrawDebug(void);	//デバッグ用
private:
	int modelId_;//モデルの格納

	VECTOR scl_;	// 大きさ
	VECTOR rot_;	// 角度
	VECTOR pos_;	// 表示座標
	VECTOR dir_;	// 移動方向

	float collisionRadius_;		// 衝突判定用の球体半径
	VECTOR collisionLocalPos_;	// 衝突判定用の球体中心の調整座標

	// 衝突判定に用いられるコライダ
	std::vector <std::weak_ptr<Collider>> colliders_;

	// 衝突判定
	void Collision(void);

};

