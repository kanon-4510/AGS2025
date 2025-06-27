#pragma once
#include <memory>
#include <vector>
#include "SceneBase.h"

class Stage;
class EnemyBase;
class SkyDome;
class Tree;
class Player;
class Item;
class MiniMap;

class GameScene : public SceneBase
{
public:
	static constexpr int ENCOUNT = 300;	//エンカウンタ
	static constexpr int ENEMY_MAX = 30;//最大出現数

	GameScene(void);	// コンストラクタ
	~GameScene(void);	// デストラクタ

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

	void DrawMiniMap(void);

	void AddItem(std::shared_ptr<Item> item);
private:
	void EnemyCreate(void);

	std::unique_ptr<Stage> stage_;		// ステージ
	std::shared_ptr<Tree>tree_;			// ツリー
	//EnemyBase& enemy_;				// エネミー
	std::unique_ptr<SkyDome> skyDome_;	// スカイドーム
	std::shared_ptr<Player> player_;	// プレイヤー
	std::vector<std::shared_ptr<Item>> items_;		//アイテム
	std::unique_ptr<MiniMap> map_;		//ミニマップ

	int enemyModelId_;
	int imgGameUi1_;

	std::vector<std::shared_ptr<EnemyBase>> enemys_;
	int enCounter;//敵の出現頻度
};