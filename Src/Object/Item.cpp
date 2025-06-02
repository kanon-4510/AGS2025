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
	dir_ = {};
	modelId_ = 0;
}

Item::~Item(void)
{
}

void Item::Init(void)
{
	// モデルの基本設定
	modelId_ = MV1LoadModel((Application::PATH_MODEL + "Item/bottle.mv1").c_str());

	scl_ = { 0.1f, 0.1f, 0.1f };						// 大きさの設定
	rot_ = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };		// 角度の設定
	pos_ = { 0.0f, -28.0f, 500.0f };					// 位置の設定
	dir_ = { 0.0f, 0.0f, 0.0f };						// 右方向に移動する

	isAlive_ = false;

	collisionRadius_ = 70.0f;							// 衝突判定用の球体半径
	collisionLocalPos_ = { 0.0f, 100.0f, 0.0f };		// 衝突判定用の球体中心の調整座標

	Update();
}

void Item::Update(void)
{
	if (!isAlive_) {
		return;
	}

	MV1SetScale(modelId_, scl_);		// ３Ｄモデルの大きさを設定(引数は、x, y, zの倍率)
	MV1SetRotationXYZ(modelId_, rot_);	// ３Ｄモデルの向き(引数は、x, y, zの回転量。単位はラジアン。)
	MV1SetPosition(modelId_, pos_);		// ３Ｄモデルの位置(引数は、３Ｄ座標)

	VECTOR diff = VSub(player_.GetCapsule().GetPosDown(), pos_);
	float dis = AsoUtility::SqrMagnitudeF(diff);
	if (dis < collisionRadius_ * collisionRadius_ && player_.GetWater() < 10)
	{
		//範囲に入った
		player_.wHit();
		isAlive_ = false;
		return;
	}

	Collision();
}

void Item::Draw(void)
{
	if (isAlive_)
	{
		// モデルの描画
		MV1DrawModel(modelId_);
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
	return VECTOR();
}

void Item::SetPos(VECTOR pos)
{
	pos_ = pos;
}

void Item::SetCollisionPos(const VECTOR collision)
{
	collisionLocalPos_ = collision;
}

VECTOR Item::GetCollisionPos(void) const
{
	return VECTOR();
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
	v = pos_;
	DrawFormatString(20, 210, white, "水の座標 ： (%0.2f   , %0.2f   , %0.2f   )",
		v.x, v.y, v.z
	);

	DrawSphere3D(collisionLocalPos_, collisionRadius_, 8, blue, blue, false);
}