#include <DxLib.h>
#include <vector>
#include "../Application.h"
#include "../Manager/ResourceManager.h"
#include "../Utility/AsoUtility.h"
#include "./Common/Capsule.h"
#include "ActorBase.h"
#include "Player.h"
#include "Item.h"

Item::Item(Player& player, const Transform& transform):player_(player), pos_(transform.pos)
{
	transform_.dir = {};
	transform_.modelId = 0;
}

Item::~Item(void)
{
}

void Item::Init(void)
{
	// モデルの基本設定
	transform_.modelId = MV1LoadModel((Application::PATH_MODEL + "Item/bottle.mv1").c_str());

	transform_.scl = { 0.2f, 0.2f, 0.2f };						// 大きさの設定
	transform_.rot = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };	// 角度の設定
	transform_.pos = { 0.0f, 0.0f, 500.0f };					// 位置の設定
	transform_.dir = { 0.0f, 0.0f, 0.0f };						// 右方向に移動する

	isAlive_ = false;

	baseY_ = transform_.pos.y; // 初期位置を保存

	collisionRadius_ = 50.0f;							// 衝突判定用の球体半径
	collisionLocalPos_ = { 0.0f, 100.0f, 0.0f };		// 衝突判定用の球体中心の調整座標

	Update();
}

void Item::Update(void)
{
	if (!isAlive_) {
		return;
	}

	// アニメーションタイマー更新
	floatTimer_ += floatSpeed_ * DX_PI_F / 180.0f;
	if (floatTimer_ > DX_PI_F * 2) floatTimer_ -= DX_PI_F * 2;

	// Y座標を振幅ぶん上下させる（baseY_ からの相対位置）
	transform_.pos.y = baseY_ + sinf(floatTimer_) * floatHeight_;

	MV1SetScale(transform_.modelId, transform_.scl);		// ３Ｄモデルの大きさを設定(引数は、x, y, zの倍率)
	MV1SetRotationXYZ(transform_.modelId, transform_.rot);	// ３Ｄモデルの向き(引数は、x, y, zの回転量。単位はラジアン。)
	MV1SetPosition(transform_.modelId, transform_.pos);		// ３Ｄモデルの位置(引数は、３Ｄ座標)

	VECTOR diff = VSub(player_.GetCapsule().GetPosDown(), transform_.pos);
	float dis = AsoUtility::SqrMagnitudeF(diff);
	if (dis < collisionRadius_ * collisionRadius_ && player_.GetWater() < 10)
	{
		//範囲に入った
		player_.wHit();
		isAlive_ = false;
		return;
	}

	/*VECTOR diff1 = VSub(enemy_.GetCollisionPos(), pos_);
	float dis1 = AsoUtility::SqrMagnitudeF(diff1);
	if (dis1 < collisionRadius_ * collisionRadius_)
	{
		//範囲に入った
		isAlive_ = false;
		return;
	}*/

	Collision();
}

void Item::Draw(void)
{
	if (isAlive_)
	{
		// モデルの描画
		MV1DrawModel(transform_.modelId);
		DrawDebug();
	}
}

void Item::AddCollider(std::weak_ptr<Collider> collider)
{
	colliders_.push_back(collider);
}

void Item::ClearCollider(void)
{
	colliders_.clear();
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

void Item::Collision(void)
{
	collisionLocalPos_ = pos_;
}

void Item::DrawDebug(void)
{
	int white = 0xffffff;
	int black = 0x000000;
	int red = 0xff0000;
	int green = 0x00ff00;
	int blue = 0x0000ff;
	int yellow = 0xffff00;
	int purpl = 0x800080;

	VECTOR v;

	// キャラ基本情報
	//-------------------------------------------------------
	// キャラ座標
	v = transform_.pos;
	DrawFormatString(20, 120, white, "水の座標 ： (%0.2f   , %0.2f   , %0.2f   )",
		v.x, v.y, v.z
	);

	DrawSphere3D(transform_.pos, collisionRadius_, 8, blue, blue, false);
}