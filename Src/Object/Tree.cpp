#include<DxLib.h>
#include"../Common/Vector2.h"
#include"../Scene/GameScene.h"
#include"../Manager/SceneManager.h"
#include"../Application.h"
#include"Player.h"
#include"Tree.h"

Tree::Tree(void)
{
	pos_ = { 0,0 };
	lv_ = 0;
	isAlive_ = false;
	hp_ = 0;
	water_ = 0;
}
Tree::~Tree(void)
{
}

bool Tree::Init(GameScene* parent)
{
	img_[0] = LoadGraph("bmp/Tree/tree1.png");
	img_[1] = LoadGraph("bmp/Tree/tree2.png");
	img_[2] = LoadGraph("bmp/Tree/tree3.png");
	img_[3] = LoadGraph("bmp/Tree/tree4.png");
	dmg_[0] = LoadGraph("bmp/Tree/tree11.png");
	dmg_[1] = LoadGraph("bmp/Tree/tree21.png");
	dmg_[2] = LoadGraph("bmp/Tree/tree31.png");
	dmg_[3] = LoadGraph("bmp/Tree/tree41.png");
	dmgSe_ = LoadMusicMem("sound/SE/dmg.mp3");
	up1Se_ = LoadMusicMem("sound/SE/up1.mp3");
	up2Se_ = LoadMusicMem("sound/SE/up2.mp3");
	lv_ = 1;
	isLvUp_ = false;
	isAlive_ = true;
	isDmg_ = false;
	grow_ = Tree::GROW::BABY;
	pos_ = { 0,0 };
	hp_ = 75;
	water_ = 0;
	gameScene_ = parent;

	return true;
}
void Tree::Update(void)
{
	if (water_ >= 1)
	{
		isLvUp_ = true;
		lv_ += 1;
		water_ -= 1;
		ChangeGrow();
	}
}
void Tree::Draw(void)
{
	switch (grow_)
	{
	case Tree::GROW::BABY:
		DrawRotaGraph(350, 120, 0.1f, 0.0f, img_[0], true, false);
		pos_ = { 350,130 };
		break;
	case Tree::GROW::KID:
		DrawRotaGraph(350, 100, 1.25f, 0.0f, img_[1], true, false);
		pos_ = { 338,138 };
		break;
	case Tree::GROW::ADULT:
		DrawRotaGraph(350, -17, 2.0f, 0.0f, img_[2], true, false);
		pos_ = { 338,138 };
		break;
	case Tree::GROW::OLD:
		DrawRotaGraph(350, -45, 2.0f, 0.0f, img_[3], true, false);
		pos_ = { 345,135 };
		break;
	}
	if (isDmg_ == true)
	{
		for (int d = 0; d <= 60; d++)
		{
			switch (grow_)
			{
			case Tree::GROW::BABY:
				DrawRotaGraph(350, 120, 0.1f, 0.0f, dmg_[0], true, false);
				pos_ = { 350,130 };
				break;
			case Tree::GROW::KID:
				DrawRotaGraph(350, 100, 1.25f, 0.0f, dmg_[1], true, false);
				pos_ = { 338,138 };
				break;
			case Tree::GROW::ADULT:
				DrawRotaGraph(350, -17, 2.0f, 0.0f, dmg_[2], true, false);
				pos_ = { 338,138 };
				break;
			case Tree::GROW::OLD:
				DrawRotaGraph(350, -45, 2.0f, 0.0f, dmg_[3], true, false);
				pos_ = { 345,135 };
				break;
			}
		}
		isDmg_ = false;
	}

#pragma region ステータス画面
	DrawFormatString(735, 20, 0x0, "YGGDRASILL : LV%d", lv_);
	//DrawFormatString(735, 35, 0x0, "HP");
	DrawBox(735, 40, 885, 50, 0, true);
	DrawBox(735, 40, 735 + hp_ * 2, 50, 0x00ef00, true);

	DrawBox(735, 55, 885, 65, 0, true);
	if (isLvUp_ == true)
	{
		for (int c = 0; c <= 150; c++)
		{
			DrawBox(735, 55, 735 + c, 65, 0x0050ff, true);
		}
		isLvUp_ = false;
	}
#pragma endregion
}
bool Tree::Release(void)
{
	return false;
}

//Vector2 Tree::GetPos(void)
//{
//	return pos_;
//}
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
		hp_ = 75;
	}
	else if (lv_ == 50)
	{
		grow_ = Tree::GROW::ADULT;
		hp_ = 75;
	}
	else if (lv_ == 25)
	{
		grow_ = Tree::GROW::KID;
		hp_ = 75;
	}
	else if (lv_ == 1)
	{
		grow_ = Tree::GROW::BABY;
		hp_ = 75;
	}
}

void Tree::eHit(void)
{
	hp_ -= 1;
	isDmg_ = true;
	PlayMusicMem(dmgSe_, DX_PLAYTYPE_BACK);
}
void Tree::pHit(void)
{
	water_++;
	PlayMusicMem(up2Se_, DX_PLAYTYPE_BACK);
}