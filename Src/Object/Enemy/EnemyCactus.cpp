#include "EnemyCactus.h"

void EnemyCactus::SetParam(void)
{
	// 使用メモリ容量と読み込み時間の削減のため
	// モデルデータをいくつもメモリ上に存在させない
	transform_.modelId = MV1DuplicateModel(baseModelId_[static_cast<int>(TYPE::SABO)]);

	transform_.scl = { 0.5f, 0.5f, 0.5f };						// 大きさの設定
	transform_.rot = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };	// 角度の設定
	transform_.pos = { 100.0f, -28.0f, 1000.0f };				// 位置の設定
	transform_.dir = { 0.0f, 0.0f, -1.0f };								// 右方向に移動する


	speed_ = 01.0f;		// 移動スピード

	isAlive_ = true;	// 初期は生存状態

	currentAnimType_ = ANIM_TYPE::RUN;	//アニメーションタイプ

	attachNo_ = MV1AttachAnim(transform_.modelId, 0);
	animAttachNos_[static_cast<int>(currentAnimType_)] = attachNo_;
	animTotalTimes_[static_cast<int>(currentAnimType_)] = MV1GetAttachAnimTotalTime(transform_.modelId, attachNo_);
	stepAnims_[static_cast<int>(currentAnimType_)] = 0.0f;
	speedAnim_ = 30.0f;

	hp_ = 2;	// HPの設定

	collisionRadius_ = 40.0f;	// 衝突判定用の球体半径
	collisionLocalPos_ = { 0.0f, 0.0f, 0.0f };	// 衝突判定用の球体中心の調整座標

}