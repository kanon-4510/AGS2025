#pragma once
#include "../EnemyBase.h"


class EnemyVirus : public EnemyBase
{
public:
	EnemyVirus(int baseModelId);

	//アニメーションロード用
	void InitAnimation(void) override;

	// パラメータ設定(純粋仮想関数)
	void SetParam(void) override;
};
