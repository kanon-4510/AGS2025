#include <DxLib.h>
#include "../Utility/AsoUtility.h"
#include "../Application.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/InputManager.h"
#include "../Manager/GravityManager.h"
#include "../Object/Common/Capsule.h"
#include "../Object/Common/Collider.h"
#include "../Object/SkyDome.h"
#include "../Object/Stage.h"
#include "../Object/Player.h"
#include "../Object/EnemyBase.h"
#include "../Object/Tree.h"
#include "../Object/Planet.h"
#include "../Object/Item.h"
#include "GameScene.h"

//担当いけだ

GameScene::GameScene(void)
{
	player_ = nullptr;
	tree_ = nullptr;
	skyDome_ = nullptr;
	stage_ = nullptr;
	//item_ = nullptr;
}

GameScene::~GameScene(void)
{
}

void GameScene::Init(void)
{
	// プレイヤー
	player_ = std::make_shared<Player>();
	GravityManager::GetInstance().SetPlayer(player_);
	player_->Init();

	// 敵のモデル
	enemyModelId_ = MV1LoadModel((Application::PATH_MODEL + "Enemy/Run.mv1").c_str());
	auto enemy = new EnemyBase(enemyModelId_);
	enemy->Init();
	enemys_.push_back(enemy);

	player_->SetEnemy(enemys_[0]);
	enemy->SetPlayer(player_);

	//木
	tree_ = std::make_shared<Tree>();
	tree_->Init();

	//アイテム
	/*item_ = std::make_shared<Item>(*player_, Transform{});
	item_->Init();*/
	enemy->SetGameScene(this);

	// ステージ
	stage_ = std::make_unique<Stage>(*player_);
	stage_->Init();
	// ステージの初期設定
	stage_->ChangeStage(Stage::NAME::MAIN_PLANET);

	// スカイドーム
	skyDome_ = std::make_unique<SkyDome>(player_->GetTransform());
	skyDome_->Init();

	mainCamera->SetFollow(&player_->GetTransform());
	mainCamera->ChangeMode(Camera::MODE::FOLLOW);
}

void GameScene::Update(void)
{
	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_TAB))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}
	if (tree_->GetLv() >= 100)
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::CLEAR);
	}
	if (tree_->GetHp() <= 0)
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::OVER);
	}

	skyDome_->Update();
	stage_->Update();
	tree_->Update();
	player_->Update();
	for (auto& item : items_)
	{
		item->Update();
	}
	for (auto enemy : enemys_)
	{
		enemy->Update();
	}


}

void GameScene::Draw(void)
{
	skyDome_->Draw();
	stage_->Draw();
	tree_->Draw();
	player_->Draw();
	for (auto& item : items_)
	{
		item->Draw();
	}
	for (auto enemy : enemys_)
	{
		enemy->Draw();
	}

	// ヘルプ
	DrawFormatString(30, 500, 0x000000, "移動　　：WASD");
	DrawFormatString(30, 520, 0x000000, "カメラ　：矢印キー");
	DrawFormatString(30, 540, 0x000000, "ダッシュ：左Shift");
	DrawFormatString(30, 560, 0x000000, "ジャンプ：Space");
}

void GameScene::AddItem(std::shared_ptr<Item> item)
{
	items_.push_back(item);
}
