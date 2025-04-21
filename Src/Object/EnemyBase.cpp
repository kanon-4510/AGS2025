#include "../Manager/SceneManager.h"
#include "ActorBase.h"
#include "EnemyBase.h"

EnemyBase::EnemyBase(int baseModelId)
{
	baseModelId_[static_cast<int>(TYPE::BIRD)] = baseModelId;
}

EnemyBase::~EnemyBase(void)
{

}

void EnemyBase::Init(void)
{
	SetParam();
	Update();
}

void EnemyBase::SetParam(void)
{
	// 使用メモリ容量と読み込み時間の削減のため
	// モデルデータをいくつもメモリ上に存在させない
	modelId_ = MV1DuplicateModel(baseModelId_[static_cast<int>(TYPE::BIRD)]);

	transform_.scl = { 0.5f, 0.5f, 0.5f };						// 大きさの設定
	transform_.rot = { 0.0f, -90.0f * DX_PI_F / 180.0f, 0.0f };	// 角度の設定
	transform_.pos = { 0.0f, -2780.0f, 0.0f };						// 位置の設定
	transform_.dir = { 1.0f, 0.0f, 0.0f };						// 右方向に移動する

	speed_ = 1.5f;		// 移動スピード

	isAlive_ = true;	// 初期は生存状態

	animAttachNo_ = MV1AttachAnim(modelId_, 10);	// アニメーションをアタッチする
	animTotalTime_ = MV1GetAttachAnimTotalTime(modelId_, animAttachNo_);	// アタッチしているアニメーションの総再生時間を取得する
	stepAnim_ = 0.0f;	// 再生中のアニメーション時間
	speedAnim_ = 30.0f;	// アニメーション速度

	hp_ = hpMax_ = 2;	// HPの設定

	collisionRadius_ = 35.0f;	// 衝突判定用の球体半径
	collisionLocalPos_ = { 0.0f, 50.0f, 0.0f };	// 衝突判定用の球体中心の調整座標
}


void EnemyBase::Update(void)
{
	if (!isAlive_)
	{
		return;
	}
	pos_ = VAdd(pos_, VScale(dir_, speed_));

	MV1SetScale(modelId_, scl_);		// ３Ｄモデルの大きさを設定(引数は、x, y, zの倍率)
	MV1SetRotationXYZ(modelId_, rot_);	// ３Ｄモデルの向き(引数は、x, y, zの回転量。単位はラジアン。)
	MV1SetPosition(modelId_, pos_);		// ３Ｄモデルの位置(引数は、３Ｄ座標)

	// アニメーション再生
	// 経過時間の取得
	float deltaTime = 1.0f / SceneManager::DEFAULT_FPS;
	// アニメーション時間の進行
	stepAnim_ += (speedAnim_ * deltaTime);
	if (stepAnim_ > animTotalTime_)
	{
		stepAnim_ = 0.0f;		// ループ再生
	}
	// 再生するアニメーション時間の設定
	MV1SetAttachAnimTime(modelId_, animAttachNo_, stepAnim_);
}

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
	pos_ = pos;
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

VECTOR EnemyBase::GetCollisionPos(void)
{
	return VAdd(pos_, collisionLocalPos_);
}

float EnemyBase::GetCollisionRadius(void)
{
	return collisionRadius_;
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

	// キャラ基本情報
	//-------------------------------------------------------
	// キャラ座標
	v = transform_.pos;
	DrawFormatString(20, 120, white, "キャラ座標 ： (%0.2f, %0.2f, %0.2f)",
		v.x, v.y, v.z
	);
	
	//capsule_->Draw();
}