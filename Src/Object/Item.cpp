#include <DxLib.h>
#include <vector>
#include "../Application.h"
#include "../Manager/ResourceManager.h"
#include "../Utility/AsoUtility.h"
#include "./Common/Capsule.h"
#include "ActorBase.h"
#include "Player.h"
#include "Tree.h"
#include "Item.h"

Item::Item(Player& player, const Transform& transform, TYPE itemType, Tree& tree):
	player_(player), pos_(transform.pos), itemType_(itemType), tree_(tree)
{
	transform_.dir = {};
	transform_.modelId = VALUE_ZERO;

	collisionLocalPos_ = {};
	collisionRadius_ = VALUE_ZERO;

	isAlive_ = false;
}

Item::~Item(void)
{
}

void Item::Init(void)
{
	InitModel();

	// モデルの基本設定
	transform_.scl = { ITEM_MODEL_SCALE, ITEM_MODEL_SCALE, ITEM_MODEL_SCALE };	// 大きさの設定
	transform_.rot = { AsoUtility::VECTOR_ZERO};	// 角度の設定
	transform_.dir = { AsoUtility::VECTOR_ZERO };	// 右方向に移動する

	isAlive_ = false;

	float lowestY = transform_.pos.y - floatHeight_;
	float targetMinY = ITEM_GROUND_Y + ITEM_MODEL_BOTTOM_OFFSET;
	float safetyMargin = (targetMinY > lowestY) ? (targetMinY - lowestY) : VALUE_ZERO;
	baseY_ = transform_.pos.y + safetyMargin;

	collisionRadius_ = COLLISION_SIZE;	// 衝突判定用の球体半径
	collisionLocalPos_ = COLLISION_POS;	// 衝突判定用の球体中心の調整座標
}

void Item::Update(void)
{
	if (!isAlive_) {
		return;
	}

	// アニメーションタイマー更新
	floatTimer_ += floatSpeed_ * AsoUtility::DEG2RAD;
	if (floatTimer_ > AsoUtility::FULL_ROTATION_RAD) floatTimer_ -= AsoUtility::FULL_ROTATION_RAD;

	// Y座標を振幅ぶん上下させる（baseY_ からの相対位置）
	transform_.pos.y = baseY_ + sinf(floatTimer_) * floatHeight_;

	// プレイヤーの位置と当たり判定半径を取得
	VECTOR playerPos = player_.GetCollisionPos();			// プレイヤーの位置
	float playerRadius = player_.GetCollisionRadius();		// プレイヤーの当たり半径

	 // アイテムの位置と半径
	VECTOR itemPos = transform_.pos;
	float itemRadius = collisionRadius_;					// アイテムの当たり半径

	// 球体同士の当たり判定
	if (AsoUtility::IsHitSpheres(playerPos, playerRadius, itemPos, itemRadius))
	{
		// Water アイテムの場合だけ、water量をチェック
		if (itemType_ == Item::TYPE::WATER && player_.GetWater() >= player_.WATER_MAX) {
			return; // 取得しない
		}

		//範囲に入った
		ItemUse();
		isAlive_ = false;
		return;
	}

	Collision();
}

void Item::Draw(void)
{
	if (isAlive_)
	{

		MV1SetScale(transform_.modelId, transform_.scl);		// ３Ｄモデルの大きさを設定(引数は、x, y, zの倍率)
		MV1SetRotationXYZ(transform_.modelId, transform_.rot);	// ３Ｄモデルの向き(引数は、x, y, zの回転量。単位はラジアン。)
		MV1SetPosition(transform_.modelId, transform_.pos);		// ３Ｄモデルの位置(引数は、３Ｄ座標)

		// モデルの描画
		MV1DrawModel(transform_.modelId);
		//DrawDebug();
	}
}

VECTOR Item::GetPos(void)
{
	return transform_.pos;
}

void Item::SetPos(VECTOR pos)
{
	transform_.pos = pos;
}

void Item::SetCollisionPos(const VECTOR collision)
{
	collisionLocalPos_ = collision;
}

VECTOR Item::GetCollisionPos(void) const
{
	return VAdd(collisionLocalPos_, transform_.pos);
}

float Item::GetCollisionRadius(void)
{
	return collisionRadius_;
}

void Item::SetIsAlive(bool isAlive)
{
	isAlive_ = isAlive;
}

bool Item::GetIsAlive()
{
	return isAlive_;
}

void Item::SetScale(float scale)
{
	transform_.scl = VGet(scale, scale, scale);
}

void Item::Respawn(const VECTOR& newPos)
{
	// 新しい位置で復活
	// 地面に触れないよう、baseY_ を調整しておく
	baseY_ = ITEM_GROUND_Y + ITEM_MODEL_BOTTOM_OFFSET + floatHeight_;

	transform_.pos = newPos;
	transform_.pos.y = baseY_;
	isAlive_ = true;
	floatTimer_ = VALUE_ZERO;

	// 必要なら他の状態もリセット
}

Item::TYPE Item::GetItemType(void) const
{
	return itemType_;
}

void Item::Collision(void)
{
	collisionLocalPos_ = pos_;
}

void Item::InitModel(void)
{
	switch (itemType_)
	{
	case Item::TYPE::NONE:
		break;
	case Item::TYPE::WATER:
		transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(ResourceManager::SRC::WATER));
		break;
	case Item::TYPE::POWER:
		transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(ResourceManager::SRC::POWER));
		break;
	case Item::TYPE::SPEED:
		transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(ResourceManager::SRC::SPEED));
		break;
	case Item::TYPE::HEAL:
		transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(ResourceManager::SRC::HEAL));
		break;
	case Item::TYPE::MUTEKI:
		transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(ResourceManager::SRC::MUTEKI));
		break;
	case Item::TYPE::ALL:
		transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(ResourceManager::SRC::ALL));
		break;
	default:
		break;
	}
}

void Item::ItemUse(void)
{
	switch (itemType_)
	{
	case Item::TYPE::WATER:
		player_.wHit(transform_.scl.x);
		break;
	case Item::TYPE::POWER:
		player_.PowerUp();
		break;
	case Item::TYPE::SPEED:
		player_.SpeedUp();
		break;
	case Item::TYPE::HEAL:
		player_.Heal();
		break;
	case Item::TYPE::MUTEKI:
		tree_.Guard();
		break;
	case Item::TYPE::ALL:
		player_.wHit(transform_.scl.x);
		player_.PowerUp();
		player_.SpeedUp();
		player_.Heal();
		tree_.Guard();
		break;
	
	default:
		break;
	}
}

void Item::DrawDebug(void)
{
#ifdef _DEBUG

	VECTOR v;

	// キャラ基本情報
	//-------------------------------------------------------
	// キャラ座標
	v = transform_.pos;
	DrawFormatString(20, 90, COLOR_WHITE, "水の座標 ： (%0.2f   , %0.2f   , %0.2f   )",
		v.x, v.y, v.z
	);

	DrawSphere3D(transform_.pos, collisionRadius_, DEBUG_SPHERE_DIV, COLOR_BLUE, COLOR_BLUE, false);

#endif //DEBUG
}