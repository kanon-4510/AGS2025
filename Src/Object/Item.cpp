#include <DxLib.h>
#include <vector>
#include "../Application.h"
#include "../Manager/ResourceManager.h"
#include "../Utility/AsoUtility.h"
#include "ActorBase.h"
#include "Item.h"

Item::Item(void)
{
	scl_ = { 0.0f, 0.0f, 0.0f };						// 大きさの設定
	rot_ = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };		// 角度の設定
	pos_ = { 500.0f, -28.0f, 500.0f };					// 位置の設定
	dir_ = { 0.0f, 90.0f, 0.0f };							// 右方向に移動する
}

Item::~Item(void)
{
}

void Item::Init(void)
{
	modelId_ = MV1LoadModel((Application::PATH_MODEL + "Item/bottle.mv1").c_str());

	scl_ = { 0.1f, 0.1f, 0.1f };						// 大きさの設定
	rot_ = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };		// 角度の設定
	pos_ = { 500.0f, -28.0f, 500.0f };					// 位置の設定
	dir_ = { 0.0f, 0.0f, 0.0f };						// 右方向に移動する

	collisionRadius_ = 20.0f;					// 衝突判定用の球体半径
	collisionLocalPos_ = { 0.0f, 100.0f, 0.0f };	// 衝突判定用の球体中心の調整座標

	Update();
}

void Item::Update(void)
{
	MV1SetScale(modelId_, scl_);			// ３Ｄモデルの大きさを設定(引数は、x, y, zの倍率)
	MV1SetRotationXYZ(modelId_, rot_);	// ３Ｄモデルの向き(引数は、x, y, zの回転量。単位はラジアン。)
	MV1SetPosition(modelId_, pos_);		// ３Ｄモデルの位置(引数は、３Ｄ座標)

	Collision();
}

void Item::Draw(void)
{
	// モデルの描画
	MV1DrawModel(modelId_);
	DrawDebug();
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

