#include<DxLib.h>
#include<EffekseerForDXLib.h>
#include"../Common/Vector2.h"
#include"../Scene/GameScene.h"
#include"../Utility/AsoUtility.h"
#include "../Manager/InputManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/SoundManager.h"
#include"../Application.h"
#include"Player.h"
#include"Tree.h"

Tree::Tree(void)
{
	lv_ = ZERO;
	isAlive_ = false;
	hp_ = ZERO;
	water_ = ZERO;
	dir_ = {};
	modelIdB_ = ZERO;
	modelIdK_ = ZERO;
	modelIdA_ = ZERO;
	modelIdO_ = ZERO;

	//無敵状態
	imgMutekiIcon_ = 0;
	invincible_ = false;
	mutekiCnt_ = INVINCIBLE_TIME;

	//エフェクト
	effectTreeResId_ = 0;
	effectTreePlayId_ = 0;
}
Tree::~Tree(void)
{
}

bool Tree::Init(void)
{
	//モデルの読込
	modelIdB_ = MV1LoadModel((Application::PATH_MODEL + "wood/Baby.mv1").c_str());
	modelIdK_ = MV1LoadModel((Application::PATH_MODEL + "wood/Kid_ver2.mv1").c_str());
	modelIdA_ = MV1LoadModel((Application::PATH_MODEL + "wood/Adult_ver2.mv1").c_str());
	modelIdO_ = MV1LoadModel((Application::PATH_MODEL + "wood/Old.mv1").c_str());

	//無敵アイコン画像
	imgMutekiIcon_ = LoadGraph("Data/Image/Icon/MUTEKIIcon.png");

	scl_ = BABY_SCL;			//大きさ
	rot_ = BABY_ROT;			//回転
	pos_ = BABY_POS;			//位置

	lv_ = 1;

	isAlive_ = true;
	isD_ = false;
	grow_ = Tree::GROW::BABY;
	hp_ = HP;
	water_ = 0;

	//衝突判定
	collisionRadius_ = COL_RAD_1;
	collisionLocalPos_ = COL_LOCAL_POS;

	//エフェクト
	effectTreeResId_ = ResourceManager::GetInstance().Load(ResourceManager::SRC::TREE_RANGE).handleId_;

	return true;
}
void Tree::Update(void)
{
	if (!player_) return;

	//プレイヤーとの距離をXZ平面だけで測る
	VECTOR playerPos = player_->GetPos();
	VECTOR treePos = pos_;

	float dx = playerPos.x - treePos.x;
	float dz = playerPos.z - treePos.z;
	float distance = sqrtf(dx * dx + dz * dz);

	//成長段階に応じた最小距離を設定
	float minDistance = 0.0f;

	switch (grow_) 
	{
	case GROW::BABY:
		minDistance = DISTANCE1;
		break;
	case GROW::KID:
		minDistance = DISTANCE2;
		collisionRadius_ = COL_RAD_2;
		break;
	case GROW::ADULT:
		minDistance = DISTANCE3;
		collisionRadius_ = COL_RAD_3;
		break;
	case GROW::OLD:
		minDistance = DISTANCE4;
		collisionRadius_ = COL_RAD_4;
		break;
	}

	//プレイヤーが円の内側にいたら押し出す
	if (distance < minDistance) 
	{
		float len = sqrtf(dx * dx + dz * dz);
		if (len > 0.001f) 
		{
			dx /= len;
			dz /= len;

			VECTOR newPos = {
				treePos.x + dx * minDistance,
				playerPos.y,
				treePos.z + dz * minDistance};

			player_->SetPos(newPos);//←PlayerにSetPosが必要
		}
	}

	if (distance < viewRange_ && player_->GetWater() > 0)
	{
		player_->tHit();
		pHit();//プレイヤーが近くて水を持ってたら水を貯める
	}

	switch (grow_)
	{
	case Tree::GROW::BABY:
		scl_ = BABY_SCL;				//大きさの設定
		rot_ = BABY_ROT;				//角度の設定
		pos_ = BABY_POS;				//位置の設定
		MV1SetScale(modelIdB_, scl_);						//３Ｄモデルの大きさを設定(引数は、x, y, zの倍率)
		MV1SetRotationXYZ(modelIdB_, rot_);					//３Ｄモデルの向き(引数は、x, y, zの回転量。単位はラジアン。)
		MV1SetPosition(modelIdB_, pos_);					//３Ｄモデルの位置(引数は、３Ｄ座標)
		break;
	case Tree::GROW::KID:
		scl_ = KID_SCL;
		rot_ = KID_ROT;		//角度の設定
		pos_ = KID_POS;
		MV1SetScale(modelIdK_, scl_);						//３Ｄモデルの大きさを設定(引数は、x, y, zの倍率)
		MV1SetRotationXYZ(modelIdK_, rot_);					//３Ｄモデルの向き(引数は、x, y, zの回転量。単位はラジアン。)
		MV1SetPosition(modelIdK_, pos_);					//３Ｄモデルの位置(引数は、３Ｄ座標)
		break;
	case Tree::GROW::ADULT:
		scl_ = ADULT_SCL;
		rot_ = ADULT_ROT;		//角度の設定
		pos_ = ADULT_POS;
		MV1SetScale(modelIdA_, scl_);						//３Ｄモデルの大きさを設定(引数は、x, y, zの倍率)
		MV1SetRotationXYZ(modelIdA_, rot_);					//３Ｄモデルの向き(引数は、x, y, zの回転量。単位はラジアン。)
		MV1SetPosition(modelIdA_, pos_);					//３Ｄモデルの位置(引数は、３Ｄ座標)
		break;
	case Tree::GROW::OLD:
		scl_ = OLD_SCL;
		rot_ = OLD_ROT;		//角度の設定
		pos_ = OLD_POS;
		MV1SetScale(modelIdO_, scl_);						//３Ｄモデルの大きさを設定(引数は、x, y, zの倍率)
		MV1SetRotationXYZ(modelIdO_, rot_);					//３Ｄモデルの向き(引数は、x, y, zの回転量。単位はラジアン。)
		MV1SetPosition(modelIdO_, pos_);					//３Ｄモデルの位置(引数は、３Ｄ座標)
		break;
	}

	collisionPos_ = VAdd(pos_, collisionLocalPos_);
	DrawDebugTree2Player();
	EffectTreeRange();	//エフェクト
	MutekiTimer();		//無敵時間
}
void Tree::Draw(void)
{
	//モデルの描画
	switch (grow_)
	{
	case Tree::GROW::BABY:
		MV1SetScale(modelIdB_, scl_);
		MV1SetRotationXYZ(modelIdB_, rot_);
		MV1SetPosition(modelIdB_, pos_);
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

#pragma region ステータス
	DrawFormatString(NAME_X,NAME_Y,black,"YGGDRASILL : Lv%d",lv_);
	//枠線（白）
	DrawBox(FRAME_START_X,FRAME_START_Y,FRAME_END_X,FRAME_END_Y,gray,true);

	DrawBox(BAR_START_X,BAR_START_HY,BAR_END_X,BAR_END_HY,black,true);
	if(isD_ == true)
	{
		DrawBox(BAR_START_X,BAR_START_HY,hp_*HIT_POINT+BAR_START_X,BAR_END_HY,red,true);
		isD_ = false;
	}
	else DrawBox(BAR_START_X,BAR_START_HY,hp_*HIT_POINT+BAR_START_X,BAR_END_HY,green,true);

								DrawBox(BAR_START_X,BAR_START_WY,BAR_END_X ,BAR_END_WY,black,true);
	     if(grow_==GROW::OLD)	DrawBox(BAR_START_X,BAR_START_WY,water_*WATER_BAR_OLD  +BAR_START_X,BAR_END_WY,blue ,true);
	else if(grow_==GROW::ADULT)	DrawBox(BAR_START_X,BAR_START_WY,water_*WATER_BAR_ADULT+BAR_START_X,BAR_END_WY,blue ,true);
	else if(grow_==GROW::KID)	DrawBox(BAR_START_X,BAR_START_WY,water_*WATER_BAR_KID  +BAR_START_X,BAR_END_WY,blue ,true);
	else if(grow_==GROW::BABY)	DrawBox(BAR_START_X,BAR_START_WY,water_*WATER_BAR_BABY +BAR_START_X,BAR_END_WY,blue ,true);

	if (invincible_)
	{
		// 無敵アイコン画像
		DrawRotaGraph(CX, CY, MUTEKI_ICON_SIZE, 0.0, imgMutekiIcon_, true);

		float ratio = static_cast<float>(mutekiCnt_) / INVINCIBLE_TIME;
		int filledSegments = static_cast<int>(SEGMENTS * ratio);

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, ALPHA_GRAY);

		for (int i = filledSegments; i < SEGMENTS; ++i)
		{
			float angle1 = - (DX_PI_F / 2) - DX_TWO_PI * i / SEGMENTS;
			float angle2 = - (DX_PI_F / 2) - DX_TWO_PI * (i + 1) / SEGMENTS;

			float x1 = CX + RADIUS * cosf(angle1);
			float y1 = CY + RADIUS * sinf(angle1);
			float x2 = CX + RADIUS * cosf(angle2);
			float y2 = CY + RADIUS * sinf(angle2);

			DrawTriangle(CX, CY, (int)x1, (int)y1, (int)x2, (int)y2, GetColor(0, 0, 0), true);
		}

		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
#pragma endregion
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
	VECTOR c;
}
void Tree::DrawDebugTree2Player(void)
{
	// プレイヤーとの距離判定して円を描く処理を追加
	if (player_ != nullptr)
	{
		VECTOR centerPos = pos_;  // 木の中心座標
		centerPos.y = 0.0f;		  // 円を下げる
		VECTOR playerPos = player_->GetTransform().pos;

		float dx = playerPos.x - centerPos.x;
		float dz = playerPos.z - centerPos.z;
		float distance = sqrtf(dx * dx + dz * dz);
		bool inRange = (distance <= viewRange_);

		unsigned int color = inRange ? red : orange;  /// 赤 or うすきいろ

		float angleStep = (DX_PI_F * 2.0f) / circleSegments_;

		if (grow_ == GROW::BABY) 
		{
			
		}
		else if (grow_ == GROW::KID)
		{
			centerPos.y = 0.0f;
			viewRange_ = RANGE_1;
			
		}
		else if(grow_ == GROW::ADULT)
		{
			centerPos.y = 0.0f;
			viewRange_ = RANGE_2;
			
		}
		else if(grow_ == GROW::OLD)
		{
			centerPos.y = 0.0f;
			viewRange_ = RANGE_3;
			
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

void Tree::LvUp(void)
{
	if (grow_==GROW::OLD && water_>= HOLD_WATER_OLD)
	{
		lv_+=1;
		water_-=HOLD_WATER_OLD;
		ChangeGrow();
	}
	if (grow_ == GROW::ADULT && water_>= HOLD_WATER_ADULT)
	{
		lv_+=1;
		water_ -= HOLD_WATER_ADULT;
		ChangeGrow();
	}
	if (grow_==GROW::KID && water_>=HOLD_WATER_KID)
	{
		lv_ += 1;
		water_ -= HOLD_WATER_KID;
		ChangeGrow();
	}
	if (grow_==GROW::BABY && water_>=HOLD_WATER_BABY)
	{
		lv_+=1;
		water_-=HOLD_WATER_BABY;
		ChangeGrow();
	}

	// 水を与えたときの音
	SoundManager::GetInstance().Play(SoundManager::SRC::LEVEL_UP_SE, Sound::TIMES::ONCE);
}
void Tree::ChangeGrow(void)
{
	if (lv_ == LV_OLD)
	{
		grow_ = Tree::GROW::OLD;
		hp_ = HP_MAX;
	}
	else if (lv_ == LV_ADULT)
	{
		grow_ = Tree::GROW::ADULT;
		hp_ = HP_MAX;
	}
	else if (lv_ == LV_KID)
	{
		grow_ = Tree::GROW::KID;
		hp_ = HP_MAX;
	}
	else if (lv_ == LV_BABY)
	{
		grow_ = GROW::BABY;
		hp_ = HP_MAX;
	}
}

void Tree::SetPlayer(Player* player)
{
	player_ = player;
}

VECTOR Tree::GetCollisionPos(void) const
{
	return VAdd(collisionLocalPos_, pos_);
}
float Tree::GetCollisionRadius(void)
{
	return collisionRadius_;
}

void Tree::MutekiTimer(void)
{
	//無敵
	if (invincible_)
	{
		mutekiCnt_--;

		if (mutekiCnt_ <= 0)
		{
			invincible_ = false;
			mutekiCnt_ = INVINCIBLE_TIME;
		}
	}
}
void Tree::Muteki(void)
{
	invincible_ = true;

	// 木が無敵になった時の音
	SoundManager::GetInstance().Play(SoundManager::SRC::MUTEKI_SE, Sound::TIMES::ONCE);
}
void Tree::PushEnemy(void)
{
	collisionRadius_ += TREE_GROW_RADIUS_INCREMENT;
	if (!gameScene_) return;

	float pushOutRadius = collisionRadius_ + ENEMY_SAFE_MARGIN;

	for (const auto& enemyPtr : gameScene_->GetEnemies())
	{
		if (!enemyPtr) continue;

		VECTOR enemyPos = enemyPtr->GetTransform().pos;
		float dx = enemyPos.x - pos_.x;
		float dz = enemyPos.z - pos_.z;
		float distance = sqrtf(dx * dx + dz * dz);

		if (distance < pushOutRadius && distance > MIN_DISTANCE_THRESHOLD)
		{
			float scale = pushOutRadius / distance;
			enemyPtr->SetPos({
				pos_.x + dx * scale,
				enemyPos.y,
				pos_.z + dz * scale
				});
		}
	}
}
void Tree::eHit(void)//エネミーとのあたり判定
{
	if (!invincible_)
	{
		hp_ -= 1;
		isD_ = true;

		// 木がダメージを食らった音
		SoundManager::GetInstance().Play(SoundManager::SRC::T_DAMAGE_SE, Sound::TIMES::FORCE_ONCE);
	}
}
void Tree::pHit(void)//プレイヤーとのあたり判定
{
	if (player_->IsMax() == true)
	{
		player_->SetIsMax();
		for (int l = 0; l <= DOUBLE_WATER; l++)
		{
			water_++;
			LvUp();
			ChangeGrow();
		}
	}
	else
	{
		water_++;
		LvUp();
		ChangeGrow();
	}
}

void Tree::EffectTreeRange(void)
{

	if (effectTreePlayId_ >= 0)
	{
		StopEffekseer3DEffect(effectTreePlayId_);
	}

	float scale = DEFAULT_SCL;  // デフォルト値

	switch (grow_)
	{
	case GROW::BABY:
		scale = BABY_TREE_SCL;
		break;
	case GROW::KID:
		scale = BABY_TREE_KID;
		break;
	case GROW::ADULT:
		scale = BABY_TREE_ADULT;
		break;
	case GROW::OLD:
		scale = BABY_TREE_OLD;
		break;
	default:
		break;
	}

	effectTreePlayId_ = PlayEffekseer3DEffect(effectTreeResId_);
	SetScalePlayingEffekseer3DEffect(effectTreePlayId_, scale, scale, scale);
	SetPosPlayingEffekseer3DEffect(effectTreePlayId_, pos_.x, pos_.y, pos_.z);
}