#include "Dog.h"

void EnemyDog::SetParam(void)
{
	// 使用メモリ容量と読み込み時間の削減のため
	// モデルデータをいくつもメモリ上に存在させない
	transform_.modelId = MV1DuplicateModel(baseModelId_[static_cast<int>(currentType_)]);

	transform_.scl = { 1.0f, 1.0f, 1.0f };						// 大きさの設定
	transform_.pos = { 00.0f, 50.0f, 2000.0f };					// 位置の設定
	transform_.dir = { 0.0f, 0.0f, 0.0f };						// 右方向に移動する

	speed_ = 3.0f;		// 移動スピード

	isAlive_ = true;	// 初期は生存状態

	hp_ = 2;	// HPの設定

	collisionRadius_ = 100.0f;	// 衝突判定用の球体半径
	collisionLocalPos_ = { 0.0f, 60.0f, 0.0f };	// 衝突判定用の球体中心の調整座標

	// 初期状態
	ChangeState(STATE::PLAY);
}
