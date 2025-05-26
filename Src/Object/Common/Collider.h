#pragma once
class Collider
{

public :

	// 衝突種別
	enum class TYPE
	{
		STAGE,
		PLAYER,
		ENEMY,
		ITEM
	};

	// コンストラクタ
	Collider(TYPE type, int modelId);

	// デストラクタ
	virtual ~Collider(void);

	// 衝突種別
	TYPE type_;

	// モデルのハンドルID
	int modelId_;

};
