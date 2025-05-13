#pragma once
#include <memory>
#include <vector>
#include "SceneBase.h"

class Stage;
class EnemyBase;
class SkyDome;
class Tree;
class Player;

class GameScene : public SceneBase
{
public:
	GameScene(void);	// コンストラクタ
	~GameScene(void);	// デストラクタ

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
private:
	std::unique_ptr<Stage> stage_;		// ステージ
	std::shared_ptr<Tree>tree_;			//ツリー
	//EnemyBase& enemy_;				// エネミー
	std::unique_ptr<SkyDome> skyDome_;	// スカイドーム
	std::shared_ptr<Player> player_;	// プレイヤー

	int enemyModelId_;
	std::vector<EnemyBase*> enemys_;
};