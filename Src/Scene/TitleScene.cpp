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
	imgTitle_ = -1;
	imgBackTitle_ = -1;
	imgPush_ = -1;
	imgGoGame_ = -1;
	imgRule_ = -1;
	imgEndGame_ = -1;
	imgCursor_ = -1;
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
	imgTitle_ = resMng_.Load(ResourceManager::SRC::TITLE).handleId_;			//タイトル名前画像
	imgBackTitle_ = resMng_.Load(ResourceManager::SRC::BACK_TITLE).handleId_;	//タイトル背景
	imgPush_ = resMng_.Load(ResourceManager::SRC::PUSH).handleId_;				//
	imgGoGame_ = resMng_.Load(ResourceManager::SRC::PLAY).handleId_;			//ゲームに進む画像
	imgRule_ = resMng_.Load(ResourceManager::SRC::RULE).handleId_;				//ルールを見る画像
	imgEndGame_ = resMng_.Load(ResourceManager::SRC::ENDGAME).handleId_;		//ゲームを終了する画像
	imgCursor_ = resMng_.Load(ResourceManager::SRC::CURSOR).handleId_;			//カーソル画像
	imgConfirmEnd_ = resMng_.Load(ResourceManager::SRC::CONFIRM_END).handleId_;	//本当に終了しますか？画像
	imgYes_ = resMng_.Load(ResourceManager::SRC::YES).handleId_;				//はい画像
	imgNo_ = resMng_.Load(ResourceManager::SRC::NO).handleId_;					//いいえ画像
	imgYesSel_ = resMng_.Load(ResourceManager::SRC::SELECT_YES).handleId_;		//選択中はい画像
	imgNoSel_ = resMng_.Load(ResourceManager::SRC::SELECT_NO).handleId_;		//選択中いいえ画像

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
	std::string path = Application::PATH_MODEL + "NPlayer/";
	animationControllerPlayer_ = std::make_unique<AnimationController>(charactor_.modelId);
	animationControllerPlayer_->Add(0, path + "Player.mv1", 20.0f, 2);
	animationControllerPlayer_->Play(0);

	// 敵のアニメーション
	animationControllerEnemy_ = std::make_unique<AnimationController>(enemy_.modelId);
	animationControllerEnemy_->Add(0, "Data/Model/Enemy/Yellow/Yellow.mv1", 20.0f, 1);
	animationControllerEnemy_->Play(0);

	// 定点カメラ
	mainCamera->ChangeMode(Camera::MODE::FIXED_POINT);

}

void TitleScene::Update(void)
{
	InputManager& ins = InputManager::GetInstance();

	// === 終了確認中の入力処理 ===
	if (isConfirmingExit_)
	{
		// アニメーション進行（最大まで）
		if (confirmAnimFrame_ < CONFIRM_ANIM_DURATION) {
			confirmAnimFrame_++;
		}

		if (confirmAnimFrame_ >= CONFIRM_ANIM_DURATION) {
			// 入力受付はアニメーション終了後に
			if (ins.IsTrgDown(KEY_INPUT_LEFT) || ins.IsTrgDown(KEY_INPUT_RIGHT)) {
				confirmIndex_ = 1 - confirmIndex_; // 「はい」「いいえ」切替
			}
			if (ins.IsTrgDown(KEY_INPUT_RETURN)) {
				if (confirmIndex_ == 0) {
					Application::isRunning_ = false;
				}
				else {
					isConfirmingExit_ = false;
				}
			}
		}
		return;
	}

	// === 点滅更新 ===
	blinkFrameCount_++;
	if (blinkFrameCount_ > 60) {
		blinkFrameCount_ = 0;
	}

	// === メニュー選択操作 ===
	if (ins.IsTrgDown(KEY_INPUT_DOWN)) {
		selectedIndex_ = (selectedIndex_ + 1) % 3;
	}
	else if (ins.IsTrgDown(KEY_INPUT_UP)) {
		selectedIndex_ = (selectedIndex_ + 2) % 3;
	}

	if (ins.IsTrgDown(KEY_INPUT_RETURN))
	{
		if (selectedIndex_ == 0) {
			SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
		}
		else if (selectedIndex_ == 1) {
			SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::DEMO);
		}
		else if (selectedIndex_ == 2) {
			isConfirmingExit_ = true;
			confirmIndex_ = 1;
			confirmAnimFrame_ = 0;  // ← アニメーション開始
		}
	}

	// === キャラクターの移動・向き制御 ===
	const float playerSpeed = 5.0f;
	const float enemySpeed = 5.0f;
	const float leftBound = -1150.0f;
	const float rightBound = 1150.0f;
	const float safeDistance = 250.0f;

	charactor_.pos.x += playerSpeed * enemyDirection_;

	float diffX = charactor_.pos.x - enemy_.pos.x;
	if (enemyDirection_ == 1) {
		if (diffX > safeDistance) {
			enemy_.pos.x += enemySpeed;
			if (enemy_.pos.x > charactor_.pos.x - safeDistance)
				enemy_.pos.x = charactor_.pos.x - safeDistance;
		}
	}
	else {
		if (diffX < -safeDistance) {
			enemy_.pos.x -= enemySpeed;
			if (enemy_.pos.x < charactor_.pos.x + safeDistance)
				enemy_.pos.x = charactor_.pos.x + safeDistance;
		}
	}

	if (charactor_.pos.x > rightBound) {
		enemyDirection_ = -1;
	}
	else if (charactor_.pos.x < leftBound) {
		enemyDirection_ = 1;
	}

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

	// タイトルロゴ表示
	int titleW, titleH;
	GetGraphSize(imgTitle_, &titleW, &titleH);
	DrawGraph(centerX - titleW / 2, -150, imgTitle_, true);

	// ボタン設定
	const int buttonW = 400;
	const int buttonH = 100;
	const int baseY = 450;
	const int buttonOffset = 120;

	const int yGame = baseY;
	const int yRule = baseY + buttonOffset;
	const int yExit = baseY + buttonOffset * 2;

	const int fadeCycle = 60;
	int phase = blinkFrameCount_ % fadeCycle;
	int alpha = (phase < fadeCycle / 2)
		? (255 * phase) / (fadeCycle / 2)
		: 255 - (255 * (phase - fadeCycle / 2)) / (fadeCycle / 2);

	const int buttonCount = 3;
	int yPositions[buttonCount] = { yGame, yRule, yExit };
	int images[buttonCount] = { imgGoGame_, imgRule_, imgEndGame_ };

	// メニュー描画
	for (int i = 0; i < buttonCount; ++i) {
		if (selectedIndex_ == i) {
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		}
		DrawExtendGraph(centerX - buttonW / 2, yPositions[i] - buttonH / 2,
			centerX + buttonW / 2, yPositions[i] + buttonH / 2,
			images[i], true);
		if (selectedIndex_ == i) {
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}

	// カーソル描画
	int indicatorX = centerX - buttonW / 2 - 50;
	int indicatorY = yPositions[selectedIndex_] + 8;
	DrawRotaGraph(indicatorX, indicatorY, 0.5, 0.0, imgCursor_, true);

	// ヒント表示など
	DrawGraph(1400, 500, imgPush_, true);

	// モデル描画
	MV1DrawModel(charactor_.modelId);
	MV1DrawModel(enemy_.modelId);

	if (isConfirmingExit_) {
		float t = static_cast<float>(confirmAnimFrame_) / CONFIRM_ANIM_DURATION;
		if (t > 1.0f) t = 1.0f;

		int alpha = static_cast<int>(220 * t);  // 背景透明度
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawBox(0, 0, Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y, GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		int centerX = Application::SCREEN_SIZE_X / 2;
		int centerY = Application::SCREEN_SIZE_Y / 2;

		// ウィンドウ背景画像がある場合はここに描画（省略可）

		// 「本当に終了しますか？」画像描画
		int textW, textH;
		GetGraphSize(imgConfirmEnd_, &textW, &textH);
		DrawGraph(centerX - textW / 2, centerY - 300, imgConfirmEnd_, TRUE);

		// ボタン画像
		int btnY = centerY + 100;
		int yesX = centerX - 450;
		int noX = centerX + 80;

		// 選択中で画像を切り替え
		if (confirmIndex_ == 0) {
			DrawGraph(yesX, btnY, imgYesSel_, TRUE);
			DrawGraph(noX, btnY, imgNo_, TRUE);
		}
		else {
			DrawGraph(yesX, btnY, imgYes_, TRUE);
			DrawGraph(noX, btnY, imgNoSel_, TRUE);
		}
	}
}

void TitleScene::Release(void)
{
	if (charactor_.modelId != -1)
	{
		MV1DeleteModel(charactor_.modelId);
		charactor_.modelId = -1;
	}


	SoundManager::GetInstance().Stop(SoundManager::SRC::TITLE_BGM);
}
