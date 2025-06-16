#include "EnemyThorn.h"
#include "../../Application.h"
#include "../Common/AnimationController.h"
#include "../../Manager/ResourceManager.h"
#include "../../Utility/AsoUtility.h"
#include "../Common/Capsule.h"

EnemyThorn::EnemyThorn(int baseModelId):EnemyBase(baseModelId)
{
}

void EnemyThorn::InitAnimation(void)
{
	speedAnim_ = 0.5f;

	std::string path = Application::PATH_MODEL + "Enemy/";

	animationController_ = std::make_unique<AnimationController>(transform_.modelId);

	animationController_->Add((int)ANIM_TYPE::RUN, path + "thorn/thorn.mv1", 20.0f, 1);
	animationController_->Add((int)ANIM_TYPE::ATTACK, path + "thorn/thorn.mv1", 20.0f, 2);
	animationController_->Add((int)ANIM_TYPE::DAMAGE, path + "thorn/thorn.mv1", 20.0f, 3);
	animationController_->Add((int)ANIM_TYPE::DEATH, path + "thorn/thorn.mv1", 20.0f, 4);

	animationController_->Play((int)ANIM_TYPE::RUN);
}

void EnemyThorn::SetParam(void)
{
	// 使用メモリ容量と読み込み時間の削減のため
	// モデルデータをいくつもメモリ上に存在させない
	transform_.SetModel(ResourceManager::GetInstance().LoadModelDuplicate(ResourceManager::SRC::TOGE));

	transform_.scl = { 1.0f, 1.0f, 1.0f };						// 大きさの設定
	transform_.pos = { 00.0f, 50.0f, 2000.0f };					// 位置の設定
	transform_.dir = { 0.0f, 0.0f, 0.0f };						// 右方向に移動する

	speed_ = 3.0f;		// 移動スピード

	isAlive_ = true;	// 初期は生存状態

	hp_ = 2;	// HPの設定

	collisionRadius_ = 40.0f;	// 衝突判定用の球体半径
	collisionLocalPos_ = { 0.0f, 0.0f, 0.0f };	// 衝突判定用の球体中心の調整座標

	// カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 00.0f, 130.0f, 1.0f });
	capsule_->SetLocalPosDown({ 00.0f, 0.0f, 1.0f });
	capsule_->SetRadius(30.0f);

	// 初期状態
	ChangeState(STATE::PLAY);
}