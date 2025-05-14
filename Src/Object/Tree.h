//#pragma once
#include"../Common/Vector2.h"

class GameScene;
class SceneManager;
class Player;

class Tree
{
public:
	static constexpr int GROW_UP = 4;
	enum class GROW
	{
		BABY,
		KID,
		ADULT,
		OLD,
		MAX
	};

	Tree(void);     //コンストラクタ
	~Tree(void);    //デストラクタ

	bool Init(GameScene* parent);      //初期化処理
	void Update(void);    //更新処理
	void Draw(void);      //描画処理
	bool Release(void);   //解放処理

	int GetHp(void);
	int GetLv(void);
	void ChangeGrow(void);

	//void eHit(void);
	//void pHit(void);
private:
	Player* player_;
	GameScene* gameScene_;
	SceneManager* sceneManager_;

	int lv_;
	int hp_;
	int water_;
	bool isAlive_;
	GROW grow_;
};