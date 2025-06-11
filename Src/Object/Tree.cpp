#include<DxLib.h>
#include"../Common/Vector2.h"
#include"../Scene/GameScene.h"
#include"../Utility/AsoUtility.h"
#include"../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
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
	modelIdB_ = 0;
	modelIdK_ = 0;
	modelIdA_ = 0;
	modelIdO_ = 0;

	// カプセルコライダ
	/*capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 00.0f, 130.0f, 1.0f });
	capsule_->SetLocalPosDown({ 00.0f, 0.0f, 1.0f });
	capsule_->SetRadius(30.0f);*/
}

Tree::~Tree(void)
{
}

bool Tree::Init(void)
{
	modelIdB_ = MV1LoadModel((Application::PATH_MODEL + "wood/Baby.mv1").c_str());
	modelIdK_ = MV1LoadModel((Application::PATH_MODEL + "wood/Kid.mv1").c_str());
	modelIdA_ = MV1LoadModel((Application::PATH_MODEL + "wood/Adult.mv1").c_str());
	modelIdO_ = MV1LoadModel((Application::PATH_MODEL + "wood/Old.mv1").c_str());

	scl_ = { 2.0f, 2.0f, 2.0f };							// 大きさの設定
	rot_ = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };			// 角度の設定
	pos_ = { 0.0f, 60.0f, 0.0f };							// 位置の設定

	lv_ = 1;
	isAlive_ = true;
	grow_ = Tree::GROW::BABY;
	hp_ = 50;
	water_ = 0;
	//gameScene_ = parent;

	//collisionRadius_ = 100.0f;								// 衝突判定用の球体半径
	//collisionLocalPos_ = { 0.0f, 60.0f, 0.0f };				// 衝突判定用の球体中心の調整座標

	return true;
}
void Tree::Update(void)
{
	if (!player_) return;

	// プレイヤーとの距離をXZ平面だけで測る
	VECTOR playerPos = player_->GetPos();
	VECTOR treePos = pos_;

	float dx = playerPos.x - treePos.x;
	float dz = playerPos.z - treePos.z;
	float distance = sqrtf(dx * dx + dz * dz);

	// 2. 成長段階に応じた最小距離を設定
	float minDistance = 0.0f;
	switch (grow_) {
	case GROW::BABY:
		minDistance = 70.0f;
		break;
	case GROW::KID:
		minDistance = 200.0f;
		break;
	case GROW::ADULT:
		minDistance = 400.0f;
		break;
	case GROW::OLD:
		minDistance = 600.0f;
		break;
	}

	// 3. プレイヤーが円の内側にいたら押し戻す
	if (distance < minDistance) {
		float len = sqrtf(dx * dx + dz * dz);
		if (len > 0.001f) {
			dx /= len;
			dz /= len;

			VECTOR newPos = {
				treePos.x + dx * minDistance,
				playerPos.y,
				treePos.z + dz * minDistance
			};

			player_->SetPos(newPos); // ←PlayerにSetPosが必要
		}
	}

	if (distance < viewRange_ && player_->GetWater() > 0)
	{
		water_++;          // プレイヤーが近くて水を持っていたら木に水を貯める（または別の処理に応じて）
		player_->UseWater(1);
	}

	// 水の量に応じて成長処理
	if (grow_ == GROW::OLD && water_ >= 4)
	{
		lv_ += 1;
		water_ -= 4;
		ChangeGrow();
	}
	else if (grow_ == GROW::ADULT && water_ >= 3)
	{
		lv_ += 1;
		water_ -= 3;
		ChangeGrow();
	}
	else if (grow_ == GROW::KID && water_ >= 2)
	{
		lv_ += 1;
		water_ -= 2;
		ChangeGrow();
	}
	else if (grow_ == GROW::BABY && water_ >= 1)
	{
		lv_ += 1;
		water_ -= 1;
		ChangeGrow();
	}

	switch (grow_)
	{
	case Tree::GROW::BABY:
		scl_ = { 3.0f, 2.5f, 3.0f };						// 大きさの設定
		rot_ = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };		// 角度の設定
		pos_ = { 0.0f, 100.0f, 0.0f };						// 位置の設定
		MV1SetScale(modelIdB_, scl_);						//３Ｄモデルの大きさを設定(引数は、x, y, zの倍率)
		MV1SetRotationXYZ(modelIdB_, rot_);					//３Ｄモデルの向き(引数は、x, y, zの回転量。単位はラジアン。)
		MV1SetPosition(modelIdB_, pos_);					//３Ｄモデルの位置(引数は、３Ｄ座標)
		break;
	case Tree::GROW::KID:
		rot_ = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };		// 角度の設定
		scl_ = { 15.0f, 10.0f, 15.0f };
		pos_ = { 0.0f, 450.0f, 0.0f };
		MV1SetScale(modelIdK_, scl_);						//３Ｄモデルの大きさを設定(引数は、x, y, zの倍率)
		MV1SetRotationXYZ(modelIdK_, rot_);					//３Ｄモデルの向き(引数は、x, y, zの回転量。単位はラジアン。)
		MV1SetPosition(modelIdK_, pos_);					//３Ｄモデルの位置(引数は、３Ｄ座標)
		break;
	case Tree::GROW::ADULT:
		rot_ = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };		// 角度の設定
		scl_ = { 30.0f, 25.0f, 30.0f };
		pos_ = { 0.0f, 805.0f, 0.0f };
		MV1SetScale(modelIdA_, scl_);						//３Ｄモデルの大きさを設定(引数は、x, y, zの倍率)
		MV1SetRotationXYZ(modelIdA_, rot_);					//３Ｄモデルの向き(引数は、x, y, zの回転量。単位はラジアン。)
		MV1SetPosition(modelIdA_, pos_);					//３Ｄモデルの位置(引数は、３Ｄ座標)
		break;
	case Tree::GROW::OLD:
		rot_ = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };		// 角度の設定
		scl_ = { 50.0f, 45.0f, 50.0f };
		pos_ = { 0.0f, 1470.0f, 0.0f };
		MV1SetScale(modelIdO_, scl_);						//３Ｄモデルの大きさを設定(引数は、x, y, zの倍率)
		MV1SetRotationXYZ(modelIdO_, rot_);					//３Ｄモデルの向き(引数は、x, y, zの回転量。単位はラジアン。)
		MV1SetPosition(modelIdO_, pos_);					//３Ｄモデルの位置(引数は、３Ｄ座標)
		break;
	}

	if (grow_ == GROW::OLD   && water_ >= 4)
	{
		lv_ += 1;
		water_ -= 4;
		ChangeGrow();
	}
	if (grow_ == GROW::ADULT && water_ >= 3)
	{
		lv_ += 1;
		water_ -= 3;
		ChangeGrow();
	}
	if (grow_ == GROW::KID   && water_ >= 2)
	{
		lv_ += 1;
		water_ -= 2;
		ChangeGrow();
	}
	if (grow_ == GROW::BABY  && water_ >= 1)
	{
		lv_ += 1;
		water_ -= 1;
		ChangeGrow();
	}

	/*VECTOR diff = VSub(player_.GetCapsule().GetPosDown(), pos_);
	float dis = AsoUtility::SqrMagnitudeF(diff);
	if (dis < collisionRadius_ * collisionRadius_ && player_.GetWater() < 10)
	{
		//範囲に入った
		player_.wHit();
		isAlive_ = false;
		return;
	}
	isAlive_ = true;*/

	auto& ins = InputManager::GetInstance();
	if (ins.IsNew(KEY_INPUT_P))	lv_ -= 1;
	if (ins.IsNew(KEY_INPUT_O)) pHit();
	if (ins.IsNew(KEY_INPUT_L)) hp_-=1;
}

void Tree::Draw(void)
{
	// モデルの描画
	switch (grow_)
	{
	case Tree::GROW::BABY:
		MV1DrawModel(modelIdB_);
		break;
	case Tree::GROW::KID:
		MV1DrawModel(modelIdK_);
		break;
	case Tree::GROW::ADULT:
		MV1DrawModel(modelIdA_);
		break;
	case Tree::GROW::OLD:
		MV1DrawModel(modelIdO_);
		break;
	}

#pragma region ステータス表示
	DrawFormatString(55, Application::SCREEN_SIZE_Y - 160, 0x0, "YGGDRASILL : Lv%d", lv_);
	DrawBox(50, Application::SCREEN_SIZE_Y - 140, 650, Application::SCREEN_SIZE_Y - 120, 0x0, true);
	DrawBox(50, Application::SCREEN_SIZE_Y - 140, hp_ * 12 + 50, Application::SCREEN_SIZE_Y - 120, 0x00ff00, true);
	DrawBox(50, Application::SCREEN_SIZE_Y - 115, 650, Application::SCREEN_SIZE_Y - 105, 0x0, true);
	if(grow_ == GROW::BABY) DrawBox(50,Application::SCREEN_SIZE_Y-115,water_*600+50,Application::SCREEN_SIZE_Y-105,0x0000ff,true);
	if(grow_ == GROW::KID)  DrawBox(50,Application::SCREEN_SIZE_Y-115,water_*300+50,Application::SCREEN_SIZE_Y-105,0x0000ff,true);
	if(grow_ == GROW::ADULT)DrawBox(50,Application::SCREEN_SIZE_Y-115,water_*200+50,Application::SCREEN_SIZE_Y-105,0x0000ff,true);
	if(grow_ == GROW::OLD)  DrawBox(50,Application::SCREEN_SIZE_Y-115,water_*150+50,Application::SCREEN_SIZE_Y-105,0x0000ff,true);
#pragma endregion

	DrawDebugTree2Player();

	DrawDebug();
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

	v = pos_;
	DrawFormatString(20, 30, white, "木の座標：(%0.2f, %0.2f, %0.2f)", v.x, v.y, v.z);
}

void Tree::DrawDebugTree2Player(void)
{
	// プレイヤーとの距離判定して円を描く処理を追加
	if (player_ != nullptr)
	{
		VECTOR centerPos = pos_;  // 木の中心座標
		centerPos.y -= 100.0f;  // 円を下げる
		VECTOR playerPos = player_->GetTransform().pos;

		float dx = playerPos.x - centerPos.x;
		float dz = playerPos.z - centerPos.z;
		float distance = sqrtf(dx * dx + dz * dz);
		bool inRange = (distance <= viewRange_);

		unsigned int color = inRange ? 0xff0000 : 0xffdead;  // 赤 or 薄黄色

		float angleStep = DX_PI_F * 2.0f / circleSegments_;

		if (grow_ == GROW::BABY) 
		{
			
		}
		else if (grow_ == GROW::KID)
		{
			centerPos.y -= 350.0f;
			viewRange_ = 200.0f;
			
		}
		else if(grow_ == GROW::ADULT)
		{
			centerPos.y -= 705.0f;
			viewRange_ = 650.0f;
			
		}
		else if(grow_ == GROW::OLD)
		{
			centerPos.y -= 1370.0f;
			viewRange_ = 909.0f;
			
		}

		for (int i = 0; i < circleSegments_; ++i)
		{
			float angle1 = angleStep * i;
			float angle2 = angleStep * (i + 1);

			VECTOR p1 = {
				centerPos.x + viewRange_ * sinf(angle1),
				centerPos.y,
				centerPos.z + viewRange_ * cosf(angle1)
			};

			VECTOR p2 = {
				centerPos.x + viewRange_ * sinf(angle2),
				centerPos.y,
				centerPos.z + viewRange_ * cosf(angle2)
			};

			DrawTriangle3D(centerPos, p1, p2, color, false);
		}
	}
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
	if (lv_ >= 75)
	{
		grow_ = Tree::GROW::OLD;
		hp_ = 50;
	}
	else if (lv_ >= 50)
	{
		grow_ = Tree::GROW::ADULT;
		hp_ = 50;
	}
	else if (lv_ >= 25)
	{
		grow_ = Tree::GROW::KID;
		hp_ = 50;
	}
	else if (lv_ >= 1)
	{
		grow_ = GROW::BABY;
		hp_ = 50;
	}
}

void Tree::SetPlayer(Player* player)
{
	player_ = player;
}

/*const Capsule& Tree::GetCapsule(void) const
{
	return *capsule_;
}

//void Tree::SetCollisionPos(const VECTOR collision)
//{
//	spherePos_ = collision;
//}

//VECTOR Tree::GetCollisionPos(void) const
//{
//	return VAdd(collisionLocalPos_,pos_);
//}

float Tree::GetCollisionRadius(void)
{
	return collisionRadius_;
}*/

void Tree::eHit(void)//エネミーとのあたり判定
{
	hp_ -= 1;
}
void Tree::pHit(void)//プレイヤーとのあたり判定
{
	water_++;
}