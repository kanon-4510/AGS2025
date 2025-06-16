#include "EnemyMimic.h"
#include "../../Application.h"
#include "../Common/AnimationController.h"

EnemyMimic::EnemyMimic(int baseModelId)
{
	EnemyBase(baseModelId);
}

void EnemyMimic::InitAnimation(void)
{
	speedAnim_ = 0.5f;

	std::string path = Application::PATH_MODEL + "Enemy/";

	animationController_ = std::make_unique<AnimationController>(transform_.modelId);

	animationController_->Add((int)ANIM_TYPE::RUN, path + "mimic/mimic.mv1", 20.0f, 1);
	animationController_->Add((int)ANIM_TYPE::ATTACK, path + "mimic/mimic.mv1", 20.0f, 2);
	animationController_->Add((int)ANIM_TYPE::DAMAGE, path + "mimic/mimic.mv1", 20.0f, 3);
	animationController_->Add((int)ANIM_TYPE::DEATH, path + "mimic/mimic.mv1", 20.0f, 4);

	animationController_->Play((int)ANIM_TYPE::RUN);
}

void EnemyMimic::SetParam(void)
{
	EnemyBase::SetParam();
	// 使用メモリ容量と読み込み時間の削減のため
	// モデルデータをいくつもメモリ上に存在させない
	transform_.modelId = MV1DuplicateModel(baseModelId_[static_cast<int>(TYPE::MIMIC)]);

	transform_.scl = { 1.0f, 1.0f, 1.0f };						// 大きさの設定
	transform_.pos = { 00.0f, 50.0f, 2000.0f };					// 位置の設定
	transform_.dir = { 0.0f, 0.0f, 0.0f };						// 右方向に移動する

	speed_ = 3.0f;		// 移動スピード

	isAlive_ = true;	// 初期は生存状態

	hp_ = 2;	// HPの設定

	collisionRadius_ = 40.0f;	// 衝突判定用の球体半径
	collisionLocalPos_ = { 0.0f, 0.0f, 0.0f };	// 衝突判定用の球体中心の調整座標
}