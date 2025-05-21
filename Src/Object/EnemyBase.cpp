#include <string>
#include <vector>
#include "../Application.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/GravityManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/ResourceManager.h"
#include "../Utility/AsoUtility.h"
#include "Common/Capsule.h"
#include "Common/Collider.h"
#include "Common/AnimationController.h"
#include "ActorBase.h"
#include "EnemyGhost.h"
#include "EnemyBase.h"

EnemyBase::EnemyBase(int baseModelId)
{
	// 敵のモデル
	baseModelId_[static_cast<int>(TYPE::BIRD)] = baseModelId;

	animationController_ = nullptr;
	state_ = STATE::NONE;

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
	Update();
}

void EnemyBase::SetParam(void)
{
	// 使用メモリ容量と読み込み時間の削減のため
	// モデルデータをいくつもメモリ上に存在させない
	modelId_ = MV1DuplicateModel(baseModelId_[static_cast<int>(TYPE::BIRD)]);

	transform_.scl = { 1.0f, 1.0f, 1.0f };						// 大きさの設定
	transform_.rot = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };	// 角度の設定
	transform_.pos = { 00.0f, -28.0f, 1000.0f };				// 位置の設定
	dir_ = { 0.0f, 0.0f, -1.0f };								// 右方向に移動する

	speed_ = 01.0f;		// 移動スピード

	isAlive_ = true;	// 初期は生存状態

	animAttachNo_ = MV1AttachAnim(modelId_, 10);	// アニメーションをアタッチする
	animTotalTime_ = MV1GetAttachAnimTotalTime(modelId_, animAttachNo_);	// アタッチしているアニメーションの総再生時間を取得する
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
}

#pragma region Update
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

	EnemyUpdate();	//置く場所が分からん
}

void EnemyBase::UpdateNone(void)
{
}

void EnemyBase::EnemyUpdate(void)
{
	if (isAlive_)
	{
		transform_.pos = VAdd(transform_.pos, VScale(dir_, speed_));


		MV1SetScale(modelId_, transform_.scl);			// ３Ｄモデルの大きさを設定(引数は、x, y, zの倍率)
		MV1SetRotationXYZ(modelId_, transform_.rot);	// ３Ｄモデルの向き(引数は、x, y, zの回転量。単位はラジアン。)
		MV1SetPosition(modelId_, transform_.pos);		// ３Ｄモデルの位置(引数は、３Ｄ座標)

		// 衝突判定
		Collision();

		//現在座標を起点に移動後座標を決める
		movedPos_ = VAdd(transform_.pos, movePow_);

		//移動
		transform_.pos = movedPos_;
	}
}
#pragma endregion

void EnemyBase::Draw(void)
{
	if (!isAlive_)
	{
		return;
	}

	MV1DrawModel(modelId_);

	DrawDebug();
}

void EnemyBase::Release(void)
{
	MV1DeleteModel(modelId_);
}

VECTOR EnemyBase::GetPos(void)
{
	return VECTOR();
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
	if (hp_ <= 0)
	{
		hp_ = 0;
		isAlive_ = false;
	}
}

const Capsule& EnemyBase::GetCapsule(void) const
{
	return *capsule_;
}

void EnemyBase::Collision(void)
{
	// 現在座標を起点に移動後座標を決める
	movedPos_ = VAdd(transform_.pos, movePow_);

	// 移動
	moveDiff_ = VSub(movedPos_, transform_.pos);
	transform_.pos = movedPos_;

	spherePos_ = VAdd(transform_.pos, collisionLocalPos_);
}

void EnemyBase::SetCollisionPos(const VECTOR collision)
{
	spherePos_ = collision;
}

VECTOR EnemyBase::GetCollisionPos(void)const
{
	return VAdd(collisionLocalPos_, pos_);
}

float EnemyBase::GetCollisionRadius(void)
{
	return collisionRadius_;
}

void EnemyBase::InitLoad(void)
{
	std::string path = Application::PATH_MODEL + "Enemy/";

	modelId_ = MV1LoadModel((Application::PATH_MODEL + "Enemy/Yellow.mv1").c_str());

	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	animationController_->Add((int)ANIM_TYPE::RUN, path + "Run.mv1", 20.0f);
	animationController_->Add((int)ANIM_TYPE::ATTACK, path + "Attack.mv1", 60.0f);
	animationController_->Add((int)ANIM_TYPE::DAMAGE, path + "Attack.mv1", 60.0f);
	animationController_->Add((int)ANIM_TYPE::DEATH, path + "Attack.mv1", 60.0f);

	animationController_->Play((int)ANIM_TYPE::RUN);
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

void EnemyBase::Rotate(void)
{
	stepRotTime_ -= scnMng_.GetDeltaTime();

	// 回転の球面補間
	enemyRotY_ = Quaternion::Slerp(
		enemyRotY_, goalQuaRot_, (TIME_ROT - stepRotTime_) / TIME_ROT);
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
	DrawFormatString(20, 120, white, "キャラ座標 ： (%0.2f, %0.2f, %0.2f)",
		v.x, v.y, v.z
	);

	capsule_->Draw();
	c = capsule_->GetPosDown();
	DrawFormatString(20, 150, white, "コリジョン座標 ： (%0.2f, %0.2f, %0.2f)",
		c.x, c.y, c.z
	);

	s = spherePos_;
	DrawSphere3D(s, collisionRadius_, 8, red, red, false);
	DrawFormatString(20, 180, white, "スフィア座標 ： (%0.2f, %0.2f, %0.2f)",
		s.x, s.y, s.z
	);
}