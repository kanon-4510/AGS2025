#include <string>
#include <DxLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/GravityManager.h"
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

	skyDome_ = nullptr;
	animationController_ = nullptr;
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

	selectedIndex_ = 0;

	float size;

	// メイン惑星
	planet_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::FALL_PLANET));
	planet_.pos = AsoUtility::VECTOR_ZERO;
	planet_.scl = AsoUtility::VECTOR_ONE;
	planet_.Update();

	// 回転する惑星
	movePlanet_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::LAST_PLANET));
	movePlanet_.pos = { -250.0f, -100.0f, -100.0f };
	size = 0.7f;
	movePlanet_.scl = { size, size, size };
	movePlanet_.quaRotLocal = Quaternion::Euler(
		AsoUtility::Deg2RadF(90.0f), 0.0f, 0.0f);
	movePlanet_.Update();

	// キャラ
	charactor_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::PLAYER));
	charactor_.pos = { 0.0f,-400.0f,100.0f };
	size = 1.4f;
	charactor_.scl = { size, size, size };
	charactor_.quaRot = Quaternion::Euler(
		0.0f, AsoUtility::Deg2RadF(0.0f), 0.0f);
	charactor_.Update();

	// アニメーションの設定
	std::string path = Application::PATH_MODEL + "Player/";
	animationController_ = std::make_unique<AnimationController>(charactor_.modelId);
	animationController_->Add(0, path + "Watering.mv1", 20.0f);
	animationController_->Play(0);

	// 定点カメラ
	mainCamera->ChangeMode(Camera::MODE::FIXED_POINT);

}

void TitleScene::Update(void)
{
	blinkFrameCount_++;
	if (blinkFrameCount_ > 60) {  // 60フレーム（1秒）でリセット
		blinkFrameCount_ = 0;
	}

	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_TAB))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
	}

	if (ins.IsTrgDown(KEY_INPUT_DOWN) || ins.IsTrgDown(KEY_INPUT_UP)) {
		selectedIndex_ = (selectedIndex_ + 1) % 2;
	}

	if (ins.IsTrgDown(KEY_INPUT_RETURN)) {
		if (selectedIndex_ == 0) {
			SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::GAME);
		}
		else if (selectedIndex_ == 1) {
			//SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::RULE); // ← ルールシーンが未実装なら保留でOK
		}
	}


	// 惑星の回転
	movePlanet_.quaRot = movePlanet_.quaRot.Mult(
		Quaternion::Euler(0.0f, 0.0f, AsoUtility::Deg2RadF(-1.0f)));
	movePlanet_.Update();

	// キャラアニメーション
	animationController_->Update();

	//skyDome_->Update();

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
	indicatorY -= 13; // 少し上にずらす

	DrawRotaGraph(indicatorX, indicatorY, 0.5, 0.0, imgCursor_, true);

	// キャラモデル描画
	MV1DrawModel(charactor_.modelId);
}