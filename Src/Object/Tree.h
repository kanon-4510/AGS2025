//#pragma once
//#include <memory>
//#include "ActorBase.h"
#include"../Common/Vector2.h"

class GameScene;
class SceneManager;
class Player;
//class Capsule;
//class Collider;

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

	Tree(void);							//コンストラクタ
	~Tree(void);						//デストラクタ

	bool Init(void);		//初期化処理
	void Update(void);					//更新処理
	void Draw(void);					//描画処理
	void DrawDebug(void);				//デバッグ用
	void DrawDebugTree2Player(void);

	int GetHp(void);
	int GetLv(void);

	void LvUp(void);
	void ChangeGrow(void);

	// プレイヤーのポインタをセットする関数
	void SetPlayer(Player* player);

	/*//const Capsule& GetCapsule(void) const;	// 衝突用カプセルの取得

	//void SetCollisionPos(const VECTOR collision);//衝突判定用の球体
	//VECTOR GetCollisionPos(void)const;		// 衝突用の中心座標の取得
	//float GetCollisionRadius(void);		// 衝突用の球体半径の取得*/
	float collisionRadius_;		// 衝突判定用の球体半径
	VECTOR collisionLocalPos_;	// 衝突判定用の球体中心の調整座標

	void eHit(void);
	void pHit(void);
private:

	float viewRange_ = 100.0f; // 円の半径（任意の値）
	int circleSegments_ = 120;  // 円の分割数

	Player* player_;
	GameScene* gameScene_;
	SceneManager* sceneManager_;

	int modelIdB_;	//モデルの格納(苗木)
	int modelIdK_;	//モデルの格納(幼木)
	int modelIdA_;	//モデルの格納(成木)
	int modelIdO_;	//モデルの格納(老木)
	VECTOR scl_;	//おおきさ
	VECTOR pos_;	//位置が来る
	VECTOR rot_;	//廻天
	VECTOR dir_;	//移動せんよ

	int lv_;
	int hp_;
	int water_;
	bool isAlive_;
	GROW grow_;

	/*//VECTOR spherePos_;	//スフィアの移動後座標

	//float collisionRadius_;		// 衝突判定用の球体半径
	//VECTOR collisionLocalPos_;	// 衝突判定用の球体中心の調整座標

	//std::vector <std::weak_ptr<Collider>> colliders_;// 衝突判定に用いられるコライダ

	//std::unique_ptr<Capsule> capsule_;//カプセル*/
};