#include <string>
#include <DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/GravityManager.h"
#include "../Manager/SoundManager.h"
#include "../Manager/Camera.h"
#include "../Object/Common/AnimationController.h"
#include "../Object/SkyDome.h" 
#include "TitleScene.h"

TitleScene::TitleScene(void)
{
	imgPush_ = -1;
	imgTitle_ = -1;
	imgBackTitle_ = -1;
	imgCursor_ = -1;
	imgGame_ = -1;
	imgRule_ = -1;
	imgUDCursor_ = -1;

	animationControllerPlayer_ = nullptr;
	animationControllerEnemy_ = nullptr;
}

TitleScene::~TitleScene(void)
{
}

void TitleScene::Init(void)
{

	// 重力制御の初期化
	GravityManager::GetInstance().Init();

	// 画像読み込み
	imgTitle_ = resMng_.Load(ResourceManager::SRC::TITLE).handleId_;
	imgBackTitle_ = resMng_.Load(ResourceManager::SRC::BACK_TITLE).handleId_;
	imgPush_ = resMng_.Load(ResourceManager::SRC::PUSH).handleId_;
	imgGame_ = resMng_.Load(ResourceManager::SRC::PLAY).handleId_;
	imgRule_ = resMng_.Load(ResourceManager::SRC::RULE).handleId_;
	imgCursor_ = resMng_.Load(ResourceManager::SRC::CURSOR).handleId_; // ← カーソル画像

	// 音楽
	SoundManager::GetInstance().Play(SoundManager::SRC::TITLE_BGM, Sound::TIMES::LOOP);

	selectedIndex_ = 0;

	float size;

	// 初期位置は左端付近にプレイヤー、その左側に敵
	charactor_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::PLAYER));
	charactor_.pos = { -900.0f, -400.0f, 100.0f };
	charactor_.scl = { 1.4f, 1.4f, 1.4f };
	charactor_.quaRot = Quaternion::Euler(0.0f, AsoUtility::Deg2RadF(0.0f), 0.0f);
	charactor_.Update();

	enemy_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::DOG));
	enemy_.pos = { -1100.0f, -400.0f, 100.0f };
	enemy_.scl = { 1.3f, 1.3f, 1.3f };
	enemy_.quaRot = Quaternion::Euler(0.0f, AsoUtility::Deg2RadF(0.0f), 0.0f);
	enemy_.Update();

	// 右向きスタート（1:右向き、-1:左向き）
	enemyDirection_ = 1;

	// プレイヤーのアニメーション
	std::string path = Application::PATH_MODEL + "Player/";
	animationControllerPlayer_ = std::make_unique<AnimationController>(charactor_.modelId);
	animationControllerPlayer_->Add(0, path + "Run.mv1", 20.0f);
	animationControllerPlayer_->Play(0);

	// 敵のアニメーション
	animationControllerEnemy_ = std::make_unique<AnimationController>(enemy_.modelId);
	animationControllerEnemy_->Add(0, "Data/Model/Enemy/Yellow/Run.mv1", 20.0f);
	animationControllerEnemy_->Play(0);

	// 定点カメラ
	mainCamera->ChangeMode(Camera::MODE::FIXED_POINT);

}

void TitleScene::Update(void)
{
	// 点滅フレーム更新（1秒周期）
	blinkFrameCount_++;
	if (blinkFrameCount_ > 60) {
		blinkFrameCount_ = 0;
	}

	// 入力処理
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_DOWN) || ins.IsTrgDown(KEY_INPUT_UP)) {
		selectedIndex_ = (selectedIndex_ + 1) % 2;
	}
	if (ins.IsTrgDown(KEY_INPUT_RETURN)) {
		if (selectedIndex_ == 0) {
			SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
		}
		else if (selectedIndex_ == 1) {
			SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::DEMO);
		}
	}

	const float playerSpeed = 5.0f;
	const float enemySpeed = 5.0f;
	const float leftBound = -1150.0f;
	const float rightBound = 1150.0f;
	const float safeDistance = 250.0f;

	// プレイヤーは向き(enemyDirection_)に従い移動
	charactor_.pos.x += playerSpeed * enemyDirection_;

	// 敵はプレイヤーを追いかける
	float diffX = charactor_.pos.x - enemy_.pos.x;
	if (enemyDirection_ == 1) {
		// 右向きの時、敵はプレイヤーの後ろを追いかける（safeDistanceを保つ）
		if (diffX > safeDistance) {
			enemy_.pos.x += enemySpeed;
			if (enemy_.pos.x > charactor_.pos.x - safeDistance)
				enemy_.pos.x = charactor_.pos.x - safeDistance;
		}
	}
	else {
		// 左向きの時、敵はプレイヤーの後ろを追いかける
		if (diffX < -safeDistance) {
			enemy_.pos.x -= enemySpeed;
			if (enemy_.pos.x < charactor_.pos.x + safeDistance)
				enemy_.pos.x = charactor_.pos.x + safeDistance;
		}
	}

	// 端に来たら向きを反転
	if (charactor_.pos.x > rightBound) {
		enemyDirection_ = -1; // 左向きへ
	}
	else if (charactor_.pos.x < leftBound) {
		enemyDirection_ = 1; // 右向きへ
	}

	// 向き更新（右向きは-90°、左向きは90°）
	float yRotDeg = (enemyDirection_ == 1) ? -90.0f : 90.0f;
	enemy_.quaRot = Quaternion::Euler(0.0f, AsoUtility::Deg2RadF(yRotDeg), 0.0f);
	charactor_.quaRot = Quaternion::Euler(0.0f, AsoUtility::Deg2RadF(yRotDeg), 0.0f);

	enemy_.Update();
	charactor_.Update();

	animationControllerPlayer_->Update();
	animationControllerEnemy_->Update();
}

void TitleScene::Draw(void)
{
	int centerX = Application::SCREEN_SIZE_X / 2;

	DrawGraph(0, 0, imgBackTitle_, true);

	// タイトルロゴ（中央上に表示、等倍）
	int titleW, titleH;
	GetGraphSize(imgTitle_, &titleW, &titleH);
	DrawGraph(centerX - titleW / 2, -150, imgTitle_, true);

	int buttonW = 400;
	int buttonH = 100;

	int yGame = 600;
	int yRule = 720;

	// フェード用の透明度計算
	int fadeCycle = 60; // 60フレーム（1秒）で1周期
	int phase = blinkFrameCount_ % fadeCycle;
	int alpha = 0;
	if (phase < fadeCycle / 2) {
		alpha = (255 * phase) / (fadeCycle / 2);
	}
	else {
		alpha = 255 - (255 * (phase - fadeCycle / 2)) / (fadeCycle / 2);
	}

	// 通常のボタン描画
	// 選択中ボタンだけ透明度付きで点滅させる
	if (selectedIndex_ == 0) {
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawExtendGraph(centerX - buttonW / 2, yGame - buttonH / 2,
			centerX + buttonW / 2, yGame + buttonH / 2,
			imgGame_, true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		DrawExtendGraph(centerX - buttonW / 2, yRule - buttonH / 2,
			centerX + buttonW / 2, yRule + buttonH / 2,
			imgRule_, true);
	}
	else if (selectedIndex_ == 1) {
		DrawExtendGraph(centerX - buttonW / 2, yGame - buttonH / 2,
			centerX + buttonW / 2, yGame + buttonH / 2,
			imgGame_, true);

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawExtendGraph(centerX - buttonW / 2, yRule - buttonH / 2,
			centerX + buttonW / 2, yRule + buttonH / 2,
			imgRule_, true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// カーソル（矢印）表示（選択ボタンの左に）
	int indicatorX = centerX - buttonW / 2 - 50;
	int indicatorY = (selectedIndex_ == 0) ? yGame : yRule;
	indicatorY += 8; 

	DrawRotaGraph(indicatorX, indicatorY, 0.5, 0.0, imgCursor_, true);

	//十字キー
	DrawGraph(1400, 500, imgPush_, true);

	// キャラモデル描画
	MV1DrawModel(charactor_.modelId);

	MV1DrawModel(enemy_.modelId);
}

void TitleScene::Release(void)
{
	SoundManager::GetInstance().Stop(SoundManager::SRC::TITLE_BGM);
}
