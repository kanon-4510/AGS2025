#include <DxLib.h>
#include "../Utility/AsoUtility.h"
#include "../Application.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/InputManager.h"
#include "../Manager/SoundManager.h"
#include "../Manager/GravityManager.h"
#include "../Manager/ResourceManager.h"
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
	imgGameUi1_ = -1;
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

	//木
	tree_ = std::make_shared<Tree>();
	tree_->Init();
	tree_->SetPlayer(player_.get());

	// プレイヤーにTreeを渡す(条件付き攻撃用)
	player_->SetTree(tree_.get());

	// 敵のモデル
	EnemyCreate();

	player_->SetEnemy(&enemys_);

	//アイテム
	/*item_ = std::make_shared<Item>(*player_, Transform{});
	item_->Init();*/

	// ステージ
	stage_ = std::make_unique<Stage>(*player_);
	stage_->Init();

	// ステージの初期設定
	stage_->ChangeStage(Stage::NAME::MAIN_PLANET);

	// スカイドーム
	skyDome_ = std::make_unique<SkyDome>(player_->GetTransform());
	skyDome_->Init();

	map_ = std::make_unique<MiniMap>(30000.0f, 300);
	map_->Init();

	// 画像
	imgGameUi1_ = resMng_.Load(ResourceManager::SRC::GAMEUI_1).handleId_;

	pauseImgs_[0] = resMng_.Load(ResourceManager::SRC::GOGAME).handleId_;
	pauseImgs_[1] = resMng_.Load(ResourceManager::SRC::OPERATION).handleId_;
	pauseImgs_[2] = resMng_.Load(ResourceManager::SRC::ITEMTEACH).handleId_;
	pauseImgs_[3] = resMng_.Load(ResourceManager::SRC::GOTITLE).handleId_;

	pauseExplainImgs_[0] = resMng_.Load(ResourceManager::SRC::PAUSEOPE).handleId_; // 操作説明
	pauseExplainImgs_[1] = resMng_.Load(ResourceManager::SRC::PAUSEITEM).handleId_;   // アイテム概要

	// カウンタ
	uiFadeStart_ = false;
	uiFadeFrame_ = 0;
	uiDisplayFrame_ = 0;

	// ポーズ
	isPaused_ = false;
	pauseSelectIndex_ = 0;

	// カメラのポーズ解除
	auto cam = SceneManager::GetInstance().GetCamera().lock();
	if (cam) {
		cam->SetPaused(false); // ← ここが重要！
	}

	// 音楽
	SoundManager::GetInstance().Play(SoundManager::SRC::GAME_BGM, Sound::TIMES::LOOP);

	mainCamera->SetFollow(&player_->GetTransform());
	mainCamera->ChangeMode(Camera::MODE::FOLLOW);
}

void GameScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();

	// TABキーでポーズのON/OFF切り替え（Menu中のみ）
	if (ins.IsTrgDown(KEY_INPUT_TAB)) {
		if (pauseState_ == PauseState::Menu) {
			isPaused_ = !isPaused_;
			pauseSelectIndex_ = 0;

			// カメラのポーズ切り替え
			mainCamera->SetPaused(isPaused_);
		}
		return; // TABを押したら他の処理はしない
	}

	// -------------------------
	// ポーズ中：ゲームロジック停止、メニューだけ操作可
	// -------------------------
	if (isPaused_) {
		if (pauseState_ == PauseState::Menu) {
			if (ins.IsTrgDown(KEY_INPUT_DOWN)) {
				pauseSelectIndex_ = (pauseSelectIndex_ + 1) % 4;
			}
			if (ins.IsTrgDown(KEY_INPUT_UP)) {
				pauseSelectIndex_ = (pauseSelectIndex_ + 3) % 4;
			}
			if (ins.IsTrgDown(KEY_INPUT_RETURN)) {
				switch (pauseSelectIndex_) {
				case 0: // ゲームに戻る
					isPaused_ = false;
					pauseState_ = PauseState::Menu;  // ← 必須
					mainCamera->SetPaused(false);    // カメラ復帰も忘れずに
					break;
				case 1:
					pauseState_ = PauseState::ShowControls;
					break;
				case 2:
					pauseState_ = PauseState::ShowItems;
					break;
				case 3:
					SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
					break;
				}
			}
		}
		else {
			// 操作説明 or アイテム概要画面中 → Enterで戻る
			if (ins.IsTrgDown(KEY_INPUT_RETURN)) {
				pauseState_ = PauseState::Menu;
			}
		}
		return;
	}

	// -------------------------
	// 通常時のゲーム進行（ポーズされてないときだけ）
	// -------------------------

	uiDisplayFrame_++;

	if (tree_->GetLv() >= 100) {
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::CLEAR);
	}
	if (tree_->GetHp() <= 0) {
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::OVER);
	}

	skyDome_->Update();
	stage_->Update();
	tree_->Update();
	player_->Update();
	for (auto& item : items_) {
		item->Update();
	}
	for (auto enemy : enemys_) {
		enemy->Update();
	}

	// 敵のエンカウント処理
	enCounter++;
	if (enCounter > ENCOUNT) {
		enCounter = 0;
		if (ENEMY_MAX >= enemys_.size()) {
			EnemyCreate();
		}
	}
}

void GameScene::Draw(void)
{
	skyDome_->Draw();
	stage_->Draw();
	for (auto& item : items_)
	{
		item->Draw();
	}
	for (auto enemy : enemys_)
	{
		enemy->Draw();
	}
	tree_->Draw();
	player_->Draw();

	DrawMiniMap();

	// 入力チェック or 時間経過でフェード開始
	if (!uiFadeStart_) {
		if ((CheckHitKey(KEY_INPUT_W)
			|| CheckHitKey(KEY_INPUT_A)
			|| CheckHitKey(KEY_INPUT_S)
			|| CheckHitKey(KEY_INPUT_D))
			|| uiDisplayFrame_ >= 240)  // 時間経過による自動フェード
		{
			uiFadeStart_ = true;
			uiFadeFrame_ = 0;
		}
	}

	if (!uiFadeStart_) {
		// フェード前（通常表示）
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
		DrawGraph(400, 40, imgGameUi1_, true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
	else if (uiFadeFrame_ < 60) {
		// フェード中（60フレームで徐々に消す）
		int alpha = static_cast<int>(255 * (60 - uiFadeFrame_) / 60.0f);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawGraph(400, 40, imgGameUi1_, true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		uiFadeFrame_++;
	}

	if (isPaused_) {
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
		DrawBox(0, 0, 1920, 1080, GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		if (pauseState_ == PauseState::Menu) {
			// 通常のポーズメニュー
			const int startX = 700;
			const int startY = 300;
			const int spacing = 90;

			for (int i = 0; i < 4; ++i) {
				int y = startY + i * spacing;
				DrawGraph(startX, y, pauseImgs_[i], true);
			}
		}
		else if (pauseState_ == PauseState::ShowControls) {
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
			DrawBox(0, 0, 1920, 1080, GetColor(255, 255, 255), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			DrawGraph(0, 0, pauseExplainImgs_[0], true);
			DrawFormatString(30, 950, GetColor(255, 255, 255), "Enterキーで戻る");
		}
		else if (pauseState_ == PauseState::ShowItems) {
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
			DrawBox(0, 0, 1920, 1080, GetColor(255, 255, 255), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			DrawGraph(0, 0, pauseExplainImgs_[1], true);
			DrawFormatString(30, 950, GetColor(255, 255, 255), "Enterキーで戻る");
		}

		return;
	}
}

void GameScene::Release(void)
{
	SoundManager::GetInstance().Stop(SoundManager::SRC::GAME_BGM);
}

void GameScene::DrawMiniMap(void)
{
	if (!map_) return;

	// プレイヤーの座標
	MapVector2 playerPos;
	playerPos.x = player_->GetTransform().pos.x;
	playerPos.z = player_->GetTransform().pos.z;
	// Y軸回転角を使用(ラジアン or 度数)
	float playerAngle = player_->GetTransform().rot.y;

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
	map_->Draw(playerPos, playerAngle, enemyPositions, itemPositions);

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

std::shared_ptr<Item> GameScene::CreateItem(const VECTOR& spawnPos, float scale, Item::TYPE itemType)
{
	// 現在のアクティブ（生きている）アイテム数を数える
	int aliveCount = 0;
	for (const auto& item : items_) {
		if (item->GetIsAlive()) {
			aliveCount++;
		}
	}

	// 上限に達していたら生成しない
	/*if (aliveCount >= MAX_ITEMS) {
		return nullptr;
	}*/

	// 再利用可能なアイテムを探す
	if (itemType == Item::TYPE::WATER)
	{
		for (auto& item : items_) {
			if (!item->GetIsAlive() && item->GetItemType() == Item::TYPE::WATER) {
				OutputDebugStringA("再利用アイテムを使用\n");
				item->Respawn(spawnPos);
				item->SetScale(scale);
				return item;
			}
		}
	}

	// 再利用できなければ新しく作成
	OutputDebugStringA("新規アイテムを作成\n");
	auto newItem = std::make_shared<Item>(*player_, Transform{}, itemType);
	newItem->Init(); // 初期化（モデル読み込み等）
	newItem->Respawn(spawnPos);
	newItem->SetScale(scale);
	items_.push_back(newItem);
	return newItem;
}

void GameScene::EnemyCreate(void)
{

	int randDir = GetRand(3);
	VECTOR randPos = VGet(0.0f, 0.0f, 0.0f);
	switch (randDir)//位置
	{
	case 0://前
		randPos.x = GetRand(20000) - 10000;
		randPos.z = 10000;
		break;
	case 1://後
		randPos.x = GetRand(20000) - 10000;
		randPos.z = -10000;
		break;
	case 2://左
		randPos.x = -10000;
		randPos.z = GetRand(20000) - 10000;
		break;
	case 3://右
		randPos.x = 10000;
		randPos.z = GetRand(29000) - 10000;
		break;
	default:
		break;
	}

	std::shared_ptr<EnemyBase> enemy;

	//敵のtype
	EnemyBase::TYPE type_ = static_cast<EnemyBase::TYPE>(GetRand(static_cast<int>(EnemyBase::TYPE::MAX) - 1));
	switch (type_)
	{
	case EnemyBase::TYPE::SABO:
		enemy = std::make_shared<EnemyCactus>();
		break;
	case EnemyBase::TYPE::DOG:
		enemy = std::make_shared<EnemyDog>();
		break;
	case EnemyBase::TYPE::MIMIC:
		enemy = std::make_shared<EnemyMimic>();
		break;
	case EnemyBase::TYPE::MUSH:
		enemy = std::make_shared<EnemyMushroom>();
		break;
	case EnemyBase::TYPE::ONION:
		enemy = std::make_shared<EnemyOnion>();
		break;
	case EnemyBase::TYPE::TOGE:
		enemy = std::make_shared<EnemyThorn>();
		break;
	case EnemyBase::TYPE::VIRUS:
		enemy = std::make_shared<EnemyVirus>();
		break;
	default:
		enemy = std::make_shared<EnemyCactus>();
		break;
	}
	// 生成された敵の初期化
	enemy->SetGameScene(this);
	enemy->SetPos(randPos);
	enemy->SetPlayer(player_);
	enemy->SetTree(tree_);
	enemy->Init();

	enemys_.emplace_back(std::move(enemy));
}