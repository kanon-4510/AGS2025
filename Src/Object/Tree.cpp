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
}
Tree::~Tree(void)
{
}

bool Tree::Init(GameScene* parent)
{
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
	if (water_ >= 1)
	{
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
bool Tree::Release(void)
{
	return false;
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