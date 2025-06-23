#include <DxLib.h>
#include <string>
#include <vector>
#include "../Application.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/SceneManager.h"
#include "../Scene/GameScene.h"
#include "../Utility/AsoUtility.h"
#include "Common/AnimationController.h"
#include "Common/Collider.h"
#include "ActorBase.h"
#include "Player.h"
#include "EnemyBase.h"

EnemyBase::EnemyBase() 
	: 
	scene_(nullptr),
	movePow_(AsoUtility::VECTOR_ZERO)
{
	animationController_ = nullptr;

	scene_ = nullptr;
	item_ = nullptr;
	state_ = STATE::NONE;

	// 状態管理
	stateChanges_.emplace(
		STATE::NONE, std::bind(&EnemyBase::ChangeStateNone, this));
	stateChanges_.emplace(
		STATE::PLAY, std::bind(&EnemyBase::ChangeStatePlay, this));
	stateChanges_.emplace(
		STATE::ATTACK, std::bind(&EnemyBase::ChangeStateAttack, this));
	stateChanges_.emplace(
		STATE::DAMAGE, std::bind(&EnemyBase::ChangeStateDamage, this));
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
}

void EnemyBase::SetParam(void)
{	
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

void EnemyBase::UpdatePlay(void)
{
	if (isAlive_)
	{
		// 衝突判定
		Collision();

		ChasePlayer();

		UpdateAttackCollisionPos();
	}
}

void EnemyBase::UpdateAttack(void)
{
	animationController_->Play((int)ANIM_TYPE::ATTACK, false);

	// 攻撃タイミング例：フレーム20あたりでヒット
	if (isAttack_ != true)
	{
		CollisionAttack();
		isAttack_ = true; // 多重ヒット防止用フラグ
	}

	// アニメーション終了で次の状態に遷移
	if (animationController_->IsEnd()) {
		ChangeState(STATE::PLAY);
		isAttack_ = false;
	}
}

void EnemyBase::UpdateDamage(void)
{
	animationController_->Play((int)ANIM_TYPE::DAMAGE, false);
	if (animationController_->IsEnd())
	{
		ChangeState(STATE::PLAY);
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
	if (!player_ ) {
		return;
	}

	animationController_->Play((int)ANIM_TYPE::RUN, true);

	VECTOR playerPos = player_->GetTransform().pos;

	VECTOR toPlayer = VSub(playerPos, transform_.pos);
	toPlayer.y = 0;  // 高さ無視

	float distance = VSize(toPlayer);
	//エネミーの視野内に入ったら追いかける
	if (distance <= VIEW_RANGE && player_->IsPlay())
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

void EnemyBase::Damage(int damage)
{
	hp_ -= damage;
	if (hp_ <= 0 && isAlive_)
	{
		ChangeState(STATE::DEATH);	
	}
	else if (hp_ >= 1 && isAlive_)
	{
		ChangeState(STATE::DAMAGE);
	}
}

#pragma region コリジョン
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
#pragma endregion

void EnemyBase::UpdateAttackCollisionPos(void)
{
	// 前方ベクトルを使って、Zオフセット分だけ回転
	VECTOR forward = transform_.quaRot.GetForward();

	// 前方へオフセット（ローカルZだけ使う）
	VECTOR rotatedOffset = VScale(forward, attackCollisionLocalPos_.z);

	// 敵の位置に加算して、球体のワールド位置を決定
	attackCollisionPos_ = VAdd(transform_.pos, rotatedOffset);
}

void EnemyBase::CollisionAttack(void)
{
	if (isAttack_)
	{

		//プレイヤーとの衝突判定
		// 攻撃の方向（エネミー）
		VECTOR forward = transform_.quaRot.GetForward();
		// 攻撃の開始位置と終了位置
		VECTOR attackStart = VAdd(transform_.pos, VScale(forward, 100.0f));
		attackStart.y += 100.0f;  // 攻撃の高さ調整

	}
}


void EnemyBase::SetGameScene(GameScene* scene)
{
	scene_ = scene;
}

#pragma region Stateの切り替え

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
void EnemyBase::ChangeStatePlay(void)
{
	stateUpdate_ = std::bind(&EnemyBase::UpdatePlay, this);
}

void EnemyBase::ChangeStateAttack(void)
{
	stateUpdate_ = std::bind(&EnemyBase::UpdateAttack, this);
}

void EnemyBase::ChangeStateDamage(void)
{
	stateUpdate_ = std::bind(&EnemyBase::UpdateDamage, this);
}

void EnemyBase::ChangeStateDeath(void)
{
	stateUpdate_ = std::bind(&EnemyBase::UpdateDeath, this);
}

#pragma endregion

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
	VECTOR a;

	// キャラ基本情報
	//-------------------------------------------------------
	//// キャラ座標
	//v = transform_.pos;
	//DrawFormatString(20, 120, white, "キャラ座標 ： (%0.2f, %0.2f, %0.2f)",v.x, v.y, v.z);

	/*s = collisionPos_;
	DrawSphere3D(s, collisionRadius_, 8, red, red, false);
	DrawFormatString(20, 180, white, "スフィア座標 ： (%0.2f, %0.2f, %0.2f)",s.x, s.y, s.z);
	DrawFormatString(20, 210, white, "エネミーの移動速度 ： %0.2f", speed_);*/
	
	a = attackCollisionLocalPos_;
	DrawSphere3D(a, attackCollisionRadius_, 8, purpl, purpl, false);

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