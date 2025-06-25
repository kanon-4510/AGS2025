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
#include "ClearScene.h"

ClearScene::ClearScene(void)
{
	imgPush_ = -1;
	imgTitle_ = -1;
	imgBackTitle_ = -1;
	skyDome_ = nullptr;
	animationController_ = nullptr;
}

ClearScene::~ClearScene(void)
{
}

void ClearScene::Init(void)
{

	// 重力制御の初期化
	GravityManager::GetInstance().Init();

	// 画像読み込み
	imgTitle_ = resMng_.Load(ResourceManager::SRC::TITLE).handleId_;
	imgBackTitle_ = resMng_.Load(ResourceManager::SRC::BACK_TITLE).handleId_;
	imgPush_ = resMng_.Load(ResourceManager::SRC::PUSH).handleId_;

	float size;

	// キャラ
	charactor_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::PLAYER));
	charactor_.pos = { 0.0f,-200.0f,0.0f };
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

void ClearScene::Update(void)
{

	// シーン遷移
	InputManager& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_TAB))
	{
		SceneManager::GetInstance().ChangeScene(SceneManager::SCENE_ID::TITLE);
	}

	// 惑星の回転
	movePlanet_.quaRot = movePlanet_.quaRot.Mult(
		Quaternion::Euler(0.0f, 0.0f, AsoUtility::Deg2RadF(-1.0f)));
	movePlanet_.Update();

	// キャラアニメーション
	animationController_->Update();

	//skyDome_->Update();

}

void ClearScene::Draw(void)
{
	MV1DrawModel(charactor_.modelId);
}

void ClearScene::Release(void)
{
}
