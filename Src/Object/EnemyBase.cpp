#include <string>
#include <vector>
#include "../Application.h"
#include "../Manager/GravityManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/ResourceManager.h"
#include "../Scene/GameScene.h"
#include "../Utility/AsoUtility.h"
#include "Common/Capsule.h"
#include "Common/Collider.h"
#include "Common/AnimationController.h"
#include "ActorBase.h"
#include "Player.h"
#include "EnemyGhost.h"
#include "EnemyBase.h"

EnemyBase::EnemyBase(int baseModelId)
{
	// 敵のモデル
	baseModelId_[static_cast<int>(TYPE::BIRD)] = baseModelId;

	animationController_ = nullptr;
	//item_ = nullptr;
	state_ = STATE::NONE;

	scene_ = nullptr;

	// 衝突チェック
	gravHitPosDown_ = AsoUtility::VECTOR_ZERO;
	gravHitPosUp_ = AsoUtility::VECTOR_ZERO;

	// 状態管理
	stateChanges_.emplace(
		STATE::NONE, std::bind(&EnemyBase::ChangeStateNone, this));
	stateChanges_.emplace(
		STATE::PLAY, std::bind(&EnemyBase::ChangeStatePlay, this));

	// 初期状態関数を必ず設定する！
	auto it = stateChanges_.find(state_);
	if (it != stateChanges_.end()) {
		stateUpdate_ = it->second;
	}
	else {
		stateUpdate_ = []() {};  // デフォルト空関数でクラッシュ回避
	}
}

EnemyBase::~EnemyBase(void)
{

}

void EnemyBase::Init(void)
{
	SetParam();
	InitLoad();
	//Update();
}

void EnemyBase::SetParam(void)
{
	// 使用メモリ容量と読み込み時間の削減のため
	// モデルデータをいくつもメモリ上に存在させない
	transform_.modelId = MV1DuplicateModel(baseModelId_[static_cast<int>(TYPE::BIRD)]);

	transform_.scl = { 1.0f, 1.0f, 1.0f };						// 大きさの設定
	transform_.rot = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };	// 角度の設定
	transform_.pos = { 00.0f, -28.0f, 2000.0f };				// 位置の設定
	dir_ = { 0.0f, 0.0f, -1.0f };								// 右方向に移動する

	speed_ = 01.0f;		// 移動スピード

	isAlive_ = true;	// 初期は生存状態

	//animAttachNo_ = MV1AttachAnim(modelId_, 0);	// アニメーションをアタッチする
	//animTotalTime_ = MV1GetAttachAnimTotalTime(modelId_, animAttachNo_);	// アタッチしているアニメーションの総再生時間を取得する
	stepAnim_ = 0.0f;	// 再生中のアニメーション時間
	speedAnim_ = 30.0f;	// アニメーション速度

	hp_ = hpMax_ = 2;	// HPの設定

	collisionRadius_ = 100.0f;	// 衝突判定用の球体半径
	collisionLocalPos_ = { 0.0f, 60.0f, 0.0f };	// 衝突判定用の球体中心の調整座標

	// カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 00.0f, 130.0f, 1.0f });
	capsule_->SetLocalPosDown({ 00.0f, 0.0f, 1.0f });
	capsule_->SetRadius(30.0f);


	// 初期状態
	ChangeState(STATE::PLAY);
}

void EnemyBase::Update(void)
{
	if (!isAlive_)
	{
		return;
	}

	transform_.Update();

	// アニメーション再生
	animationController_->Update();

	// 更新ステップ
	if (stateUpdate_)
	{
		stateUpdate_();
	}

}

void EnemyBase::UpdateNone(void)
{
}

void EnemyBase::EnemyUpdate(void)
{
	if (isAlive_)
	{
		if (InputManager::GetInstance().IsTrgDown(KEY_INPUT_Q)) 
		{
			Damage(1);
		}

		ChasePlayer();

		// モデル反映
		MV1SetScale(transform_.modelId, transform_.scl);
		MV1SetRotationXYZ(transform_.modelId, transform_.rot);
		MV1SetPosition(transform_.modelId, transform_.pos);

		// 衝突判定
		Collision();
	}
}

void EnemyBase::ChasePlayer(void)
{
	if (!player_) {
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

		// 角度計算（X,Z軸）
		float targetAngle = atan2f(dirToPlayer.x, dirToPlayer.z);

		//Rotate();
		// モデルの前が +Z軸ならこのままでOK
		//モデルが反対を向いているから180度プラスする
		transform_.rot.y = targetAngle + DX_PI_F;
		
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

			// 向き調整
			float targetAngle = atan2f(dirToOrigin.x, dirToOrigin.z);
			transform_.rot.y = targetAngle + DX_PI_F;
		}
	}
}

void EnemyBase::Draw(void)
{
	if (!isAlive_)
	{
		return;
	}

	if (transform_.modelId == 0)
	{
		DrawFormatString(20, 250, 0xff0000, "Model is not loaded!");
	}

	MV1DrawModel(transform_.modelId);

	DrawDebug();

	// 視野範囲の描画
	DrawDebugSearchRange();
}

void EnemyBase::Release(void)
{
	MV1DeleteModel(transform_.modelId);
}

VECTOR EnemyBase::GetPos(void)
{
	return transform_.pos;
}

void EnemyBase::SetPos(VECTOR pos)
{
	transform_.pos = pos;
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
		hp_ = 0;
		isAlive_ = false;
		printf("Enemy is dead.\n");

		if (!item_) {

			auto newItem = std::make_shared<Item>(*player_, Transform{});
			newItem->Init();
			newItem->SetPos(transform_.pos);
			newItem->SetIsAlive(true);
			scene_->AddItem(newItem);
		}
	}
}

const Capsule& EnemyBase::GetCapsule(void) const
{
	return *capsule_;
}

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

void EnemyBase::InitLoad(void)
{
	std::string path = Application::PATH_MODEL + "Enemy/";

	//modelId_ = MV1LoadModel((Application::PATH_MODEL + "Enemy/Yellow.mv1").c_str());

	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	animationController_->Add((int)ANIM_TYPE::RUN, path + "Run.mv1", 20.0f);
	animationController_->Add((int)ANIM_TYPE::ATTACK, path + "Attack.mv1", 25.0f);
	animationController_->Add((int)ANIM_TYPE::DAMAGE, path + "Dgame.mv1", 60.0f);
	animationController_->Add((int)ANIM_TYPE::DEATH, path + "Death.mv1", 60.0f);

	animationController_->Play((int)ANIM_TYPE::ATTACK);
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

void EnemyBase::ChangeStatePlay(void)
{
	stateUpdate_ = std::bind(&EnemyBase::EnemyUpdate, this);
}

//void EnemyBase::Rotate(void)
//{
//	stepRotTime_ -= scnMng_.GetDeltaTime();
//
//	// 回転の球面補間
//	enemyRotY_ = Quaternion::Slerp(
//		enemyRotY_, goalQuaRot_, (TIME_ROT - stepRotTime_) / TIME_ROT);
//}

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
	DrawFormatString(20, 120, white, "キャラ座標 ： (%0.2f, %0.2f, %0.2f)",
		v.x, v.y, v.z
	);

	capsule_->Draw();
	c = capsule_->GetPosDown();
	DrawFormatString(20, 150, white, "コリジョン座標 ： (%0.2f, %0.2f, %0.2f)",
		c.x, c.y, c.z
	);

	s = collisionPos_;
	DrawSphere3D(s, collisionRadius_, 8, red, red, false);
	DrawFormatString(20, 180, white, "スフィア座標 ： (%0.2f, %0.2f, %0.2f)",
		s.x, s.y, s.z
	);

	int animNum = MV1GetAnimNum(transform_.modelId);
	if (animNum == 0) {
		DrawFormatString(20, 260, 0xff0000, "このモデルにはアニメーションがありません");
	}

	
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

void EnemyBase::Attack(void)
{
	VECTOR diff1 = VSub(player_->GetCapsule().GetCenter(), pos_);
	float dis1 = AsoUtility::SqrMagnitudeF(diff1);
	if (dis1 < collisionRadius_ * collisionRadius_)
	{
		//範囲に入った
		isAlive_ = false;
		return;
	}
}

//bool EnemyBase::IsEndLandingA(void)
//{
//	return false;
//}

void EnemyBase::SetPlayer(std::shared_ptr<Player> player)
{
	player_ = player;
}