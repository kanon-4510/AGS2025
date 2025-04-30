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
#include "../Object/Planet.h"
#include "GameScene.h"

//担当いけだ

GameScene::GameScene(void)
{
	player_ = nullptr;
	skyDome_ = nullptr;
	stage_ = nullptr;
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
	enemyModelId_ = MV1LoadModel((Application::PATH_MODEL + "Enemy/Birb.mv1").c_str());
	auto enemy = new EnemyBase(enemyModelId_);
	enemy->Init();
	enemys_.push_back(enemy);

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

	skyDome_->Update();
	stage_->Update();
	player_->Update();
	for (auto enemy : enemys_)
	{
		enemy->Update();
	}
}

void GameScene::Draw(void)
{
	skyDome_->Draw();
	stage_->Draw();
	player_->Draw();
	for (auto enemy : enemys_)
	{
		enemy->Draw();
	}

	// ヘルプ
	DrawFormatString(840, 20, 0x000000, "移動　　：WASD");
	DrawFormatString(840, 40, 0x000000, "カメラ　：矢印キー");
	DrawFormatString(840, 60, 0x000000, "ダッシュ：右Shift");
	DrawFormatString(840, 80, 0x000000, "ジャンプ：＼(バクスラ)");
}