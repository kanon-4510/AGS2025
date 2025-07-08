#pragma once
#include "../EnemyBase.h"

class EnemyBoss : public EnemyBase
{
public:
	EnemyBoss();

	//アニメーションロード用
	void InitAnimation(void) override;

	// パラメータ設定(純粋仮想関数)
	void SetParam(void) override;
};
