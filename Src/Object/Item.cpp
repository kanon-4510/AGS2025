#include <DxLib.h>
#include <vector>
#include "../Application.h"
#include "../Manager/ResourceManager.h"
#include "../Utility/AsoUtility.h"
#include "ActorBase.h"
#include "Item.h"

Item::Item(void)
{
	dir_ = {};
	modelId_ = 0;
}

Item::~Item(void)
{
}

void Item::Init(void)
{
	modelId_ = MV1LoadModel((Application::PATH_MODEL + "Item/bottle.mv1").c_str());

	transform_.scl = { 0.1f, 0.1f, 0.1f };						// 大きさの設定
	transform_.rot = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };	// 角度の設定
	transform_.pos = { 00.0f, 0.0f, 1000.0f };					// 位置の設定
	dir_ = { 0.0f, 0.0f, 0.0f };								// 右方向に移動する
}

void Item::Update(void)
{
}

void Item::Draw(void)
{
	// モデルの描画
	MV1DrawModel(modelId_);
	DrawDebug();
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
	DrawFormatString(20, 180, white, "水の座標 ： (%0.2f, %0.2f, %0.2f)",
		v.x, v.y, v.z
	);
}
