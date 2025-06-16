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
#include "../Object/Enemy/EnemyCactus.h"
#include "../Object/Enemy/EnemyDog.h"
#include "../Object/Enemy/EnemyMimic.h"
#include "../Object/Enemy/EnemyMushroom.h"
#include "../Object/Enemy/EnemyOnion.h"
#include "../Object/Enemy/EnemyThorn.h"
#include "../Object/Enemy/EnemyVirus.h"
#include "../Object/Tree.h"
#include "../Object/Planet.h"
#include "../Object/Item.h"
#include "MiniMap.h"
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
	//auto enemy = new EnemyMimic(enemyModelId_);
	auto enemy = new EnemyVirus();
	enemy->Init();
	enemys_.push_back(enemy);

	player_->SetEnemy(enemys_[0]);
	enemy->SetPlayer(player_);

	//木
	tree_ = std::make_shared<Tree>();
	tree_->Init();

	tree_->SetPlayer(player_.get());

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

	map_ = std::make_unique<MiniMap>(16000.0f, 300);

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
	//敵のエンカウント
	enCounter++;
	if (enCounter > ENCOUNT)
	{
		enCounter = 0;//エンカウントのリセット

		for (int i = 0; i < ENEMY_MAX; i++)
		{
			if (enemys_[i]->GetState() == EnemyBase::STATE::NONE)
			{
				EnemyCreate(i);
				break;
			}
		}
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

	DrawMiniMap();
	
	// ヘルプ
	DrawFormatString(30, 500, 0x000000, "移動　　：WASD");
	DrawFormatString(30, 520, 0x000000, "カメラ　：矢印キー");
	DrawFormatString(30, 540, 0x000000, "ダッシュ：左Shift");
	DrawFormatString(30, 560, 0x000000, "ジャンプ：Space");
	DrawFormatString(30, 580, 0x000000, "攻撃　　：Eキー");
}

void GameScene::DrawMiniMap(void)
{
	if (!map_) return;

	// プレイヤーの座標
	MapVector2 playerPos;
	playerPos.x = player_->GetTransform().pos.x;
	playerPos.z = player_->GetTransform().pos.z;

	// 敵の座標リストを作成
	std::vector<MapVector2> enemyPositions;
	for (const auto& enemy : enemys_)
	{
		if (enemy->IsAlive())
		{
			MapVector2 e;
			e.x = enemy->GetTransform().pos.x;
			e.z = enemy->GetTransform().pos.z;
			enemyPositions.push_back(e);
		}
	}

	// アイテムの座標リストを作成
	std::vector<MapVector2> itemPositions;
	for (const auto& item : items_)
	{
		if (item->GetIsAlive())
		{
			MapVector2 i;
			i.x = item->GetTransform().pos.x;
			i.z = item->GetTransform().pos.z;
			itemPositions.push_back(i);
		}
	}

	// ミニマップ描画呼び出し
	map_->Draw(playerPos, enemyPositions, itemPositions);

	/*if (!map_) return;

	map_->SetPlayerPosition(player_->GetTransform().pos);

	map_->BeginRender();

	// ミニマップに表示したいオブジェクトだけ描画
	stage_->Draw();             // 地形
	tree_->Draw();              // 木
	player_->Draw();            // プレイヤー
	for (auto enemy : enemys_) {
		enemy->Draw();
	}

	map_->EndRender();

	// ミニマップを画面右上に表示
	map_->Draw(1024, 0);*/
}

void GameScene::AddItem(std::shared_ptr<Item> item)
{
	items_.push_back(item);
}

void GameScene::EnemyCreate(int i)
{
	int randDir = GetRand(3);
	VECTOR randPos;
	switch (randDir)//位置
	{
	case 0://前
		randPos.x = GetRand(29000) - 14500;
		randPos.z = 14500;
		break;
	case 1://後
		randPos.x = GetRand(29000) - 14500;
		randPos.z = -14500;
		break;
	case 2://左
		randPos.x = -14500;
		randPos.z = GetRand(29000) - 14500;
		break;
	case 3://右
		randPos.x = 14500;
		randPos.z = GetRand(29000) - 14500;
		break;
	default:
		break;
	}
	//敵のtype
	enemys_[i]->Release();
	delete enemys_[i];
	EnemyBase::TYPE type_;
	type_ = static_cast<EnemyBase::TYPE>(GetRand(static_cast<int>(EnemyBase::TYPE::MAX)-1));
	switch (type_)
	{
	case EnemyBase::TYPE::SABO:
		enemys_[i] = new EnemyCactus(0);
		break;
	case EnemyBase::TYPE::DOG:
		enemys_[i] = new EnemyDog();
		break;
	case EnemyBase::TYPE::MIMIC:
		enemys_[i] = new EnemyMimic(0);
		break;
	case EnemyBase::TYPE::MUSH:
		enemys_[i] = new EnemyMushroom(0);
		break;
	case EnemyBase::TYPE::ONION:
		enemys_[i] = new EnemyOnion(0);
		break;
	case EnemyBase::TYPE::TOGE:
		enemys_[i] = new EnemyThorn(0);
		break;
	case EnemyBase::TYPE::VIRUS:
		enemys_[i] = new EnemyVirus(0);
		break;
	case EnemyBase::TYPE::MAX:
		break;
	}
}