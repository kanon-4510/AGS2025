#pragma once
#include <memory>
#include <vector>
#include "SceneBase.h"
#include "../Object/Item.h"

class Stage;
class EnemyBase;
class SkyDome;
class Tree;
class Player;
class Item;
class MiniMap;
class Camera;

class GameScene : public SceneBase
{
public:
	static constexpr int ENCOUNT = 300;		//エンカウンタ
	static constexpr int ENEMY_MAX = 200;	//最大出現数
	static constexpr int ENE_ENC = 30;		//最大許容量

	GameScene(void);	// コンストラクタ
	~GameScene(void);	// デストラクタ

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

	void DrawMiniMap(void);

	void AddItem(std::shared_ptr<Item> item);
	std::shared_ptr<Item>CreateItem(const VECTOR& spawnPos, float scale,Item::TYPE itemType);
private:
	void EnemyCreate(void);

	std::unique_ptr<Stage> stage_;		// ステージ
	std::shared_ptr<Tree>tree_;			// ツリー
	//std::shared_ptr<EnemyBase>enemys_;// エネミー
	std::unique_ptr<SkyDome> skyDome_;	// スカイドーム
	std::shared_ptr<Player> player_;	// プレイヤー
	std::vector<std::shared_ptr<Item>> items_;		//アイテム
	std::unique_ptr<MiniMap> map_;		//ミニマップ
	std::shared_ptr<Camera> camera_;	//カメラ

	int enemyModelId_;
	int imgGameUi1_;
	int uiDisplayFrame_;	//カウンタ

	bool uiFadeStart_ = false;
	int uiFadeFrame_ = 0;

	std::vector<std::shared_ptr<EnemyBase>> enemys_;
	int enCounter;//敵の出現頻度

	int isB_;

	// ポーズ
	bool isPaused_;           // ポーズ中かどうか
	int pauseSelectIndex_;    // ポーズメニューの選択項目（上下選択）
	int pauseImgs_[4];        // メニュー画像
	int pauseExplainImgs_[2];

	enum class PauseState {
		Menu,        // 通常のポーズメニュー
		ShowControls,// 操作説明画面
		ShowItems    // アイテム概要画面
	};

	PauseState pauseState_ = PauseState::Menu;
};