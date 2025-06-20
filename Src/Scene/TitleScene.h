#pragma once
#include <memory>
#include "SceneBase.h"
#include "../Object/Common/Transform.h"
class SceneManager;
class SkyDome;
class AnimationController;

class TitleScene : public SceneBase
{

public:

	// コンストラクタ
	TitleScene(void);

	// デストラクタ
	~TitleScene(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void);

private:

	// 画像
	int imgTitle_;
	int imgBackTitle_;
	int imgPush_;
	int imgGame_;
	int imgRule_;
	int imgCursor_;
	int imgUDCursor_;

	int selectedIndex_;

	int blinkFrameCount_ = 0;

	int enemyDirection_ = 1;  // 敵の向き（1:右, -1:左）
	int playerDirection_ = 1; // プレイヤーの向き（1:右, -1:左）
	float enemySpeed_ = 5.0f; // 敵の移動速度
	float playerSpeed_ = 4.0f; // プレイヤーの移動速度

	// 画面の移動制限範囲（例）
	const float leftLimit_ = -1000.0f;
	const float rightLimit_ = 1000.0f;

	// キャラクター
	Transform charactor_;

	// 敵キャラクター
	Transform enemy_;

	// アニメーション
	std::unique_ptr<AnimationController> animationControllerPlayer_;
	std::unique_ptr<AnimationController> animationControllerEnemy_;

};