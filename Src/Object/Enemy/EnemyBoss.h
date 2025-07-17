#pragma once
#include "../EnemyBase.h"

class EnemyBoss : public EnemyBase
{
public:
	EnemyBoss();

	int maxHp_;

	//アニメーションロード用
	void InitAnimation(void) override;

	// パラメータ設定(純粋仮想関数)
	void SetParam(void) override;

	//ドロップアイテム
	Item::TYPE GetDropItemType() const override;

	void DrawBossHpBar(void) override;
};
