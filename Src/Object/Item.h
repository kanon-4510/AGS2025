#pragma once
#include "ActorBase.h"
#include <vector>
class Item : public ActorBase
{
public:
	// コンストラクタ
	Item(void);

	// デストラクタ
	~Item(void);

	void Init(void);
	void Update(void);
	void Draw(void);

	void DrawDebug(void);	//デバッグ用
private:
	int modelId_;//モデルの格納

	VECTOR scl_;
	VECTOR pos_;
	VECTOR rot_;
	VECTOR dir_;	// 移動方向

};

