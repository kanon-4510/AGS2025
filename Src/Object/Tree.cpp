#include<DxLib.h>
#include"../Common/Vector2.h"
#include"../Scene/GameScene.h"
#include"../Manager/SceneManager.h"
#include"../Application.h"
#include"Player.h"
#include"Tree.h"

Tree::Tree(void)
{
	lv_ = 0;
	isAlive_ = false;
	hp_ = 0;
	water_ = 0;
	dir_ = {};
	modelId_ = 0;
}
Tree::~Tree(void)
{
}

bool Tree::Init(GameScene* parent)
{
	modelId_ = MV1LoadModel((Application::PATH_MODEL + "wood/1.mv1").c_str());

	scl_ = { 5.0f, 5.0f, 5.0f };						// 大きさの設定
	rot_ = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };		// 角度の設定
	pos_ = { 00.0f, 0.0f, 0.0f };					// 位置の設定
	dir_ = { 0.0f, 90.0f, 0.0f };						// 右方向に移動する

	lv_ = 1;
	isAlive_ = true;
	grow_ = Tree::GROW::BABY;
	hp_ = 50;
	water_ = 0;
	gameScene_ = parent;

	return true;
}
void Tree::Update(void)
{
	MV1SetScale(modelId_, scl_);		// ３Ｄモデルの大きさを設定(引数は、x, y, zの倍率)
	MV1SetRotationXYZ(modelId_, rot_);	// ３Ｄモデルの向き(引数は、x, y, zの回転量。単位はラジアン。)
	MV1SetPosition(modelId_, pos_);		// ３Ｄモデルの位置(引数は、３Ｄ座標)

	if (water_ >= 1)
	{
		lv_ += 1;
		water_ -= 1;
		ChangeGrow();
	}
}
void Tree::Draw(void)
{
	// モデルの描画
	MV1DrawModel(modelId_);
	DrawDebug();
	switch (grow_)
	{
	case Tree::GROW::BABY:
		//DrawRotaGraph(350, 120, 0.1f, 0.0f, img_[0], true, false);
		//pos_ = { 350,130 };
		break;
	case Tree::GROW::KID:
		//DrawRotaGraph(350, 100, 1.25f, 0.0f, img_[1], true, false);
		//pos_ = { 338,138 };
		break;
	case Tree::GROW::ADULT:
		//DrawRotaGraph(350, -17, 2.0f, 0.0f, img_[2], true, false);
		//pos_ = { 338,138 };
		break;
	case Tree::GROW::OLD:
		//DrawRotaGraph(350, -45, 2.0f, 0.0f, img_[3], true, false);
		//pos_ = { 345,135 };
		break;
	}
}
void Tree::DrawDebug(void)
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
	DrawFormatString(20, 210, white, "木の座標 ： (%0.2f   , %0.2f   , %0.2f   )",
		v.x, v.y, v.z
	);
}

int Tree::GetHp(void)
{
	return hp_;
}
int Tree::GetLv(void)
{
	return lv_;
}
void Tree::ChangeGrow(void)
{
	if (lv_ == 75)
	{
		grow_ = Tree::GROW::OLD;
		hp_ = 50;
	}
	else if (lv_ == 50)
	{
		grow_ = Tree::GROW::ADULT;
		hp_ = 50;
	}
	else if (lv_ == 25)
	{
		grow_ = Tree::GROW::KID;
		hp_ = 50;
	}
	else if (lv_ == 1)
	{
		grow_ = Tree::GROW::BABY;
		hp_ = 50;
	}
}

//void Tree::eHit(void)
//{
//	hp_ -= 1;
//}
//void Tree::pHit(void)
//{
//	water_++;
//}