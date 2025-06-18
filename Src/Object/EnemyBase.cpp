#include <DxLib.h>
#include <string>
#include <vector>
#include "../Application.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/SceneManager.h"
#include "../Scene/GameScene.h"
#include "../Utility/AsoUtility.h"
#include "Common/AnimationController.h"
//#include "Common/Capsule.h"
#include "Common/Collider.h"
#include "ActorBase.h"
#include "Player.h"
#include "EnemyBase.h"

EnemyBase::EnemyBase() 
	: 
	scene_(nullptr),
	gravHitPosDown_(AsoUtility::VECTOR_ZERO),
	gravHitPosUp_(AsoUtility::VECTOR_ZERO),
	movePow_(AsoUtility::VECTOR_ZERO)
{
	animationController_ = nullptr;

	// 敵のモデル
	baseModelId_[static_cast<int>(TYPE::DOG)];
	scene_ = nullptr;
	item_ = nullptr;
	state_ = STATE::NONE;

	// 状態管理
	stateChanges_.emplace(
		STATE::NONE, std::bind(&EnemyBase::ChangeStateNone, this));
	stateChanges_.emplace(
		STATE::PLAY, std::bind(&EnemyBase::ChangeStatePlay, this));
	stateChanges_.emplace(
		STATE::DEATH, std::bind(&EnemyBase::ChangeStateDeath, this));
}

EnemyBase::~EnemyBase(void)
{

}

void EnemyBase::Init(void)
{
 	SetParam();
	InitAnimation();
}

void EnemyBase::InitAnimation(void)
{
	/*speedAnim_ = 0.5f;

	std::string path = Application::PATH_MODEL + "Enemy/";

	animationController_ = std::make_unique<AnimationController>(transform_.modelId);

	animationController_->Add((int)ANIM_TYPE::RUN, path + "Yellow/Yellow.mv1", 20.0f,1);
	animationController_->Add((int)ANIM_TYPE::ATTACK, path + "Yellow/Yellow.mv1", 20.0f,2);
	animationController_->Add((int)ANIM_TYPE::DAMAGE, path + "Yellow/Yellow.mv1", 20.0f,3);
	animationController_->Add((int)ANIM_TYPE::DEATH, path + "Yellow/Yellow.mv1", 20.0f,4);

	animationController_->Play((int)ANIM_TYPE::RUN);*/
}

void EnemyBase::SetParam(void)
{
	//// 使用メモリ容量と読み込み時間の削減のため
	//// モデルデータをいくつもメモリ上に存在させない
	//transform_.modelId = MV1DuplicateModel(baseModelId_[static_cast<int>(currentType_)]);

	//transform_.scl = { 1.0f, 1.0f, 1.0f };						// 大きさの設定
	//transform_.quaRotLocal = Quaternion::Euler(AsoUtility::Deg2RadF(0.0f),AsoUtility::Deg2RadF(180.0f) , 0.0f);//クォータニオンをいじると向きが変わる
	//transform_.pos = { 00.0f, 50.0f, 2000.0f };					// 位置の設定
	//transform_.dir = { 0.0f, 0.0f, 0.0f };						// 右方向に移動する

	//speed_ = 3.0f;		// 移動スピード

	//isAlive_ = true;	// 初期は生存状態
	//
	//hp_ = 2;	// HPの設定

	//collisionRadius_ = 100.0f;	// 衝突判定用の球体半径
	//collisionLocalPos_ = { 0.0f, 60.0f, 0.0f };	// 衝突判定用の球体中心の調整座標

	//// カプセルコライダ
	//capsule_ = std::make_unique<Capsule>(transform_);
	//capsule_->SetLocalPosTop({ 00.0f, 130.0f, 1.0f });
	//capsule_->SetLocalPosDown({ 00.0f, 0.0f, 1.0f });
	//capsule_->SetRadius(30.0f);

	//// 初期状態
	//ChangeState(STATE::PLAY);
}

void EnemyBase::Update(void)
{
	if (!isAlive_)
	{
		return;
	}

	transform_.Update();

	//アニメーション再生
	animationController_->Update();


	// 更新ステップ
	if (stateUpdate_)
	{
		stateUpdate_();
	}
}

#pragma region StateごとのUpdate

void EnemyBase::UpdateNone(void)
{
}

void EnemyBase::UpdateAllive(void)
{
	if (isAlive_)
	{
		// 衝突判定
		Collision();

		ChasePlayer();
	}
}
void EnemyBase::UpdateDeath(void)
{

	animationController_->Play((int)ANIM_TYPE::DEATH, false);

	if (animationController_->IsEnd())
	{
		isAlive_ = false;
		//アイテムドロップ
		
			auto newItem = std::make_shared<Item>(*player_, Transform{});
			newItem->Init();
			newItem->SetPos(transform_.pos);
			newItem->SetIsAlive(true);
			scene_->AddItem(newItem);
	}
}

#pragma endregion


void EnemyBase::ChasePlayer(void)
{
	if (!player_ /*|| currentAnimType_ != ANIM_TYPE::RUN*/) {
		return;
	}
	VECTOR playerPos = player_->GetTransform().pos;

	VECTOR toPlayer = VSub(playerPos, transform_.pos);
	toPlayer.y = 0;  // 高さ無視

	float distance = VSize(toPlayer);
	//エネミーの視野内に入ったら追いかける
	if (distance <= VIEW_RANGE)
	{
		VECTOR dirToPlayer = VNorm(toPlayer);
		VECTOR moveVec = VScale(dirToPlayer, speed_);

		transform_.pos = VAdd(transform_.pos, moveVec);

		// 方向からクォータニオンに変換
		transform_.quaRot = Quaternion::LookRotation(dirToPlayer);
		
	}
	else
	{
		// 原点に向かう
		VECTOR toOrigin = VSub(VGet(0.0f, 0.0f, 0.0f), transform_.pos);
		toOrigin.y = 0;  // 高さ無視

		float distToOrigin = VSize(toOrigin);
		if (distToOrigin > 0.01f) // 近すぎる場合は動かない
		{
			VECTOR dirToOrigin = VNorm(toOrigin);
			VECTOR moveVec = VScale(dirToOrigin, speed_);
			transform_.pos = VAdd(transform_.pos, moveVec);

			// 方向からクォータニオンに変換
			transform_.quaRot = Quaternion::LookRotation(dirToOrigin);
		}
	}
}

void EnemyBase::Draw(void)
{
	if (!isAlive_)
	{
		return;
	}

	Collision();

	// モデル反映
	MV1SetScale(transform_.modelId, transform_.scl);
	MV1SetPosition(transform_.modelId, transform_.pos);

	MV1DrawModel(transform_.modelId);

	//デッバグ
	DrawDebug();

	// 視野範囲の描画
	DrawDebugSearchRange();
}

void EnemyBase::Release(void)
{
	MV1DeleteModel(transform_.modelId);

	//capsule_.reset();
}

VECTOR EnemyBase::GetPos(void)
{
	return transform_.pos;
}

void EnemyBase::SetPos(VECTOR pos)
{
	transform_.pos = pos;
}

EnemyBase::STATE EnemyBase::GetState(void)
{
	return state_;
}

bool EnemyBase::IsAlive(void)
{
	return isAlive_;
}

void EnemyBase::SetAlive(bool alive)
{
	isAlive_ = alive;
}

void EnemyBase::Attack(void)
{

}


void EnemyBase::Damage(int damage)
{
	hp_ -= damage;
	if (hp_ <= 0 && isAlive_)
	{
		ChangeState(STATE::DEATH);	
	}
}

//const Capsule& EnemyBase::GetCapsule(void) const
//{
//	return *capsule_;
//}

const Item& EnemyBase::GetItem(void) const
{
	return *item_;
}

void EnemyBase::Collision(void)
{
	// 現在座標を起点に移動後座標を決める
	movedPos_ = VAdd(transform_.pos, movePow_);

	// 移動
	moveDiff_ = VSub(movedPos_, transform_.pos);
	transform_.pos = movedPos_;

	collisionPos_ = VAdd(transform_.pos, collisionLocalPos_);
}

void EnemyBase::SetCollisionPos(const VECTOR collision)
{
	collisionPos_ = collision;
}

VECTOR EnemyBase::GetCollisionPos(void)const
{
	return VAdd(collisionLocalPos_, transform_.pos);
}

float EnemyBase::GetCollisionRadius(void)
{
	return collisionRadius_;
}

void EnemyBase::SetGameScene(GameScene* scene)
{
	scene_ = scene;
}

void EnemyBase::ChangeState(STATE state)
{
	// 状態変更
	state_ = state;

	// 各状態遷移の初期処理
	stateChanges_[state_]();
}

void EnemyBase::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&EnemyBase::UpdateNone, this);
}
void EnemyBase::ChangeStateAlive(void)
{
	stateUpdate_ = std::bind(&EnemyBase::UpdateAllive, this);
}

void EnemyBase::ChangeStateDeath(void)
{
	stateUpdate_ = std::bind(&EnemyBase::UpdateDeath, this);
}

void EnemyBase::SetPlayer(std::shared_ptr<Player> player)
{
	player_ = player;
}

void EnemyBase::DrawDebug(void)
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
	VECTOR s;

	// キャラ基本情報
	//-------------------------------------------------------
	// キャラ座標
	v = transform_.pos;
	DrawFormatString(20, 120, white, "キャラ座標 ： (%0.2f, %0.2f, %0.2f)",v.x, v.y, v.z);

	/*capsule_->Draw();
	c = capsule_->GetPosDown();
	DrawFormatString(20, 150, white, "コリジョン座標 ： (%0.2f, %0.2f, %0.2f)",c.x, c.y, c.z);*/

	s = collisionPos_;
	DrawSphere3D(s, collisionRadius_, 8, red, red, false);
	DrawFormatString(20, 180, white, "スフィア座標 ： (%0.2f, %0.2f, %0.2f)",s.x, s.y, s.z);
	DrawFormatString(20, 210, white, "エネミーの移動速度 ： %0.2f", speed_);
	DrawFormatString(20, 330, 0xffffff, "アタッチNo.%2d",currentAnimType_);
}

void EnemyBase::DrawDebugSearchRange(void)
{
	VECTOR centerPos = transform_.pos;
	float radius = VIEW_RANGE;
	int segments = 60;

	// プレイヤーの座標
	VECTOR playerPos = player_->GetTransform().pos; // プレイヤーオブジェクトの参照を持っている想定

	// プレイヤーと敵の距離（XZ平面）
	float dx = playerPos.x - centerPos.x;
	float dz = playerPos.z - centerPos.z;
	float distance = sqrtf(dx * dx + dz * dz);

	// 範囲内か判定
	bool inRange = (distance <= radius);

	// 色を決定（範囲内なら赤、範囲外は元の色）
	unsigned int color = inRange ? 0xff0000 : 0xffdead;

	float angleStep = DX_PI * 2.0f / segments;

	for (int i = 0; i < segments; ++i)
	{
		float angle1 = angleStep * i;
		float angle2 = angleStep * (i + 1);

		VECTOR p1 = {
			centerPos.x + radius * sinf(angle1),
			centerPos.y,
			centerPos.z + radius * cosf(angle1)
		};
		VECTOR p2 = {
			centerPos.x + radius * sinf(angle2),
			centerPos.y,
			centerPos.z + radius * cosf(angle2)
		};
		DrawTriangle3D(centerPos, p1, p2, color, false);
	}
	DrawSphere3D(centerPos, 20.0f, 10, 0x00ff00, 0x00ff00, true);
}