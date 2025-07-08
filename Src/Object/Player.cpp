#include <string>
#include <vector>
#include <algorithm>
#include <EffekseerForDXLib.h>
#include "../Application.h"
#include "../Utility/AsoUtility.h"
#include "../Manager/SceneManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/GravityManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/Camera.h"
#include "Common/AnimationController.h"
#include "Common/Capsule.h"
#include "Common/Collider.h"
#include "Common/SpeechBalloon.h"
#include "Planet.h"
#include "Tree.h"
#include "Player.h"

//担当髙野

Player::Player(void)
{
	animationController_ = nullptr;
	enemy_ = nullptr;
	tree_ = nullptr;

	state_ = STATE::NONE;

	effectSmokeResId_ = -1;
	effectSmokePleyId_ = -1;


	jumpPow_ = AsoUtility::VECTOR_ZERO;
	// 衝突チェック
	gravHitPosDown_ = AsoUtility::VECTOR_ZERO;
	gravHitPosUp_ = AsoUtility::VECTOR_ZERO;

	//ジャンプの初期化
	isJump_ = false;
	imgShadow_ = -1;
	stepJump_ = 0.0f; //初期化しなかったら遷移時にジャンプを押してる間ジャンプし続ける

	//攻撃の初期化
	isAttack_ = false;
	isAttack2_ = false;
	exAttack_ = false;
	exTimer_ = 10000;
	lastExTime_ = -exTimer_;

	//ステ関連
	hp_ = 30;
	water_ = 0;

	// 無敵状態
	invincible_ = false;
	// 移動が可能かどうか
	canMove_ = true;
	// 所持上限かどうか
	isMax_ = false;

	//ワープの初期化
	reserveStartPos_ = AsoUtility::VECTOR_ZERO;

	// 状態管理
	stateChanges_.emplace(
		STATE::NONE, std::bind(&Player::ChangeStateNone, this));
	stateChanges_.emplace(
		STATE::PLAY, std::bind(&Player::ChangeStatePlay, this));
}

Player::~Player(void)
{
}

void Player::Init(void)
{
	// モデルの基本設定
	transform_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::PLAYER));
	transform_.scl = AsoUtility::VECTOR_ONE;
	transform_.pos = { 300.0f, 0.0f, 0.0f };
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal =
		Quaternion::Euler({ 0.0f, AsoUtility::Deg2RadF(180.0f), 0.0f });
	transform_.Update();

	// 丸影画像
	imgShadow_ = resMng_.Load(
		ResourceManager::SRC::PLAYER_SHADOW).handleId_;

	//足煙エフェクト
	effectSmokeResId_ = ResourceManager::GetInstance().Load(
		ResourceManager::SRC::FOOT_SMOKE).handleId_;

	//モデルのフレーム番号
	fremLeHandl_ = MV1SearchFrame(transform_.modelId, "mixamorig:LeftHand");
	fremReHandl_ = MV1SearchFrame(transform_.modelId, "mixamorig:RightHand");

	// アニメーションの設定
	InitAnimation();


	// カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 0.0f, 110.0f, 0.0f });
	capsule_->SetLocalPosDown({ 0.0f, 30.0f, 0.0f });
	capsule_->SetRadius(20.0f);

	collisionRadius_ = 100.0f;	// 衝突判定用の球体半径
	collisionLocalPos_ = { 0.0f, capsule_->GetCenter().y, 0.0f};	// 衝突判定用の球体中心の調整座標

	//enemy_ = new EnemyBase(); // OK
	//enemy_->SetCollisionPos({ 0.0f, 0.0f, 0.0f });

	// 初期状態
	ChangeState(STATE::PLAY);
}

void Player::Update(void)
{
	// 更新ステップ
	stateUpdate_();

	transform_.Update();

	// アニメーション再生
	animationController_->Update();

	UpdateDown(1.0f);

	auto& ins = InputManager::GetInstance();
	//if (ins.IsNew(KEY_INPUT_U)) wHit();
}

void Player::UpdateDown(float deltaTime)
{
	auto& ins = InputManager::GetInstance();
	if (ins.IsNew(KEY_INPUT_I)) Damage(1);

	if (pstate_ == PlayerState::DOWN) {
		isAttack_ = false;
		isAttack2_ = false;
		exAttack_ = false;
		revivalTimer_ += deltaTime;
		if (revivalTimer_ >= D_COUNT) {
			Revival();
		}
		return;
	}
}

void Player::Draw(void)
{
	MV1DrawModel(transform_.modelId);	// モデルの描画
	DrawShadow();						// 丸影描画
	DrawDebug();						// デバッグ用描画

#pragma region ステータス
	DrawFormatString(55, Application::SCREEN_SIZE_Y - 95, 0x0, "PLAYER");
	DrawBox(50, Application::SCREEN_SIZE_Y - 75, 650, Application::SCREEN_SIZE_Y - 55, 0x0, true);
	if (hp_ != 0)DrawBox(50, Application::SCREEN_SIZE_Y - 75, hp_ * 20 + 50, Application::SCREEN_SIZE_Y - 55, 0x00ff00, true);
	if (hp_ == 0)DrawBox(50, Application::SCREEN_SIZE_Y - 75, revivalTimer_ + 50, Application::SCREEN_SIZE_Y - 55, 0xff0000, true);
	DrawBox(50, Application::SCREEN_SIZE_Y - 50, 650, Application::SCREEN_SIZE_Y - 40, 0x0, true);
	DrawBox(50, Application::SCREEN_SIZE_Y - 50, water_ * 60 + 50, Application::SCREEN_SIZE_Y - 40, 0x0000ff, true);
#pragma endregion
}

void Player::AddCollider(std::weak_ptr<Collider> collider)
{
	colliders_.push_back(collider);
}

void Player::ClearCollider(void)
{
	colliders_.clear();
}

void Player::SetEnemy(const std::vector<std::shared_ptr<EnemyBase>>* enemys)
{
	enemy_ = enemys;
}

VECTOR Player::GetPos() const
{
	return transform_.pos;
}

void Player::SetPos(const VECTOR& pos)
{
	transform_.pos = pos;
}

const Capsule& Player::GetCapsule(void) const
{
	return *capsule_;
}

VECTOR Player::GetCollisionPos(void) const
{
	return VAdd(collisionLocalPos_, transform_.pos);
}

float Player::GetCollisionRadius(void)
{
	return collisionRadius_;
}

const std::vector<std::shared_ptr<EnemyBase>>& Player::GetEnemyCollision(void) const
{
	return *enemy_;
}

bool Player::IsPlay(void) const
{
	return state_ == STATE::PLAY;
}

void Player::InitAnimation(void)
{

	std::string path = Application::PATH_MODEL + "NPlayer/";

	animationController_ = std::make_unique<AnimationController>(transform_.modelId);


	animationController_->Add((int)ANIM_TYPE::IDLE, path + "Player.mv1", 60.0f, 1);
	animationController_->Add((int)ANIM_TYPE::RUN, path + "Player.mv1", 17.0f,2);
	animationController_->Add((int)ANIM_TYPE::FAST_RUN, path + "Player.mv1", 13.0f, 3);
	animationController_->Add((int)ANIM_TYPE::JUMP, path + "Player.mv1", 60.0f);
	animationController_->Add((int)ANIM_TYPE::ATTACK2, path + "Player.mv1", 17.0f, 4);
	animationController_->Add((int)ANIM_TYPE::ATTACK1, path + "Player.mv1", 17.0f, 5);
	animationController_->Add((int)ANIM_TYPE::DOWN, path + "Player.mv1", 15.0f, 7);
	animationController_->Add((int)ANIM_TYPE::EXATTACK, path + "Player.mv1", 15.0f, 8);

	animationController_->Play((int)ANIM_TYPE::IDLE);

}

void Player::ChangeState(STATE state)
{

	// 状態変更
	state_ = state;

	// 各状態遷移の初期処理
	stateChanges_[state_]();

}

void Player::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&Player::UpdateNone, this);
}

void Player::ChangeStatePlay(void)
{
	stateUpdate_ = std::bind(&Player::UpdatePlay, this);
}

void Player::UpdateNone(void)
{
}

void Player::UpdatePlay(void)
{
	if (canMove_)
	{
		//移動処理
		ProcessMove();

		// 移動方向に応じた回転
		Rotate();

		// ジャンプ処理
		ProcessJump();

		// 攻撃処理
		ProcessAttack();

		// 重力による移動量
		CalcGravityPow();

		// 衝突判定
		Collision();

		//現在座標を起点に移動後座標を決める
		movedPos_ = VAdd(transform_.pos, movePow_);

		//移動
		transform_.pos = movedPos_;

		// 重力方向に沿って回転させる
		transform_.quaRot = grvMng_.GetTransform().quaRot;
		transform_.quaRot = transform_.quaRot.Mult(playerRotY_);

		// 歩きエフェクト
		EffectFootSmoke();

	}
}

void Player::DrawShadow(void)
{
	SetUseLighting(FALSE);
	SetUseZBuffer3D(TRUE);
	SetTextureAddressMode(DX_TEXADDRESS_CLAMP);

	constexpr int MAX_VERTS = 300;
	verts_.clear(); // メンバ変数の std::vector<VERTEX3D> verts_;
	verts_.reserve(MAX_VERTS);

	const VECTOR start = transform_.pos;
	const VECTOR end = VAdd(transform_.pos, VGet(0.0f, -PLAYER_SHADOW_HEIGHT, 0.0f));

	VERTEX3D baseVertex = {};
	baseVertex.dif = GetColorU8(255, 255, 255, 255);
	baseVertex.spc = GetColorU8(0, 0, 0, 0);
	baseVertex.su = baseVertex.sv = 0.0f;

	for (const auto& c : colliders_)
	{
		auto collider = c.lock();
		if (!collider) continue;

		auto modelId = collider->modelId_;

		MV1_COLL_RESULT_POLY_DIM hitRes = MV1CollCheck_Capsule(modelId, -1, start, end, PLAYER_SHADOW_SIZE);

		for (int i = 0; i < hitRes.HitNum && verts_.size() + 3 <= MAX_VERTS; ++i)
		{
			const MV1_COLL_RESULT_POLY& poly = hitRes.Dim[i];
			const VECTOR slide = VScale(poly.Normal, 0.5f);

			for (int j = 0; j < 3; ++j)
			{
				VERTEX3D v = baseVertex;
				VECTOR p = VAdd(poly.Position[j], slide);
				v.pos = p;

				float yDiff = fabsf(p.y - transform_.pos.y);
				v.dif.a = (p.y > transform_.pos.y - PLAYER_SHADOW_HEIGHT) ?
					static_cast<BYTE>(128 * (1.0f - yDiff / PLAYER_SHADOW_HEIGHT)) : 0;

				v.u = (p.x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
				v.v = (p.z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;

				verts_.push_back(v);
			}
		}

		MV1CollResultPolyDimTerminate(hitRes);
	}

	if (!verts_.empty())
		DrawPolygon3D(verts_.data(), static_cast<int>(verts_.size()) / 3, imgShadow_, TRUE);

	SetUseLighting(TRUE);
	SetUseZBuffer3D(FALSE);
}

void Player::DrawDebug(void)
{

	int white = 0xffffff;
	int black = 0x000000;
	int red = 0xff0000;
	int green = 0x00ff00;
	int blue = 0x0000ff;
	int yellow = 0xffff00;
	int purpl = 0x800080;

	VECTOR v;

	// キャラ基本情報
	//-------------------------------------------------------
	// キャラ座標
	v = transform_.pos;
	DrawFormatString(20, 60, white, "Player座標 ： (%0.2f, %0.2f, %0.2f)%d", v.x, v.y, v.z, hp_);
	//-------------------------------------------------------

	// 衝突
	DrawLine3D(gravHitPosUp_, gravHitPosDown_, 0x000000);

	
	if (isAttack_ || isAttack2_) {

		VECTOR forward = transform_.quaRot.GetForward();
		VECTOR attackCollisionPos = VAdd(transform_.pos, VScale(forward, 100.0f));
		attackCollisionPos.y += 100.0f;
		float attackCollisionRadius = 100.0f;
		// カプセルの描画確認用	
		DrawSphere3D(attackCollisionPos, attackCollisionRadius, 8, GetColor(255, 0, 0), GetColor(255, 255, 255), FALSE);
	}
	if (exAttack_) {
		VECTOR attackCollisionPos = transform_.pos;
		attackCollisionPos.y += 100.0f;
		float attackCollisionRadius = 150.0f;
		// カプセルの描画確認用	
		DrawSphere3D(attackCollisionPos, attackCollisionRadius, 8, GetColor(255, 0, 0), GetColor(255, 255, 255), FALSE);
	}
	if (!IsExAttackReady())
	{
		int remaining = (exTimer_ - (GetNowCount() - lastExTime_)) / 1000;
		DrawFormatString(50, 40, GetColor(255, 0, 0), "回転斬り使用不可: 残り%d秒", remaining);
	}


	//capsule_->Draw();

	VECTOR s;
	s = collisionPos_;
	DrawSphere3D(s, collisionRadius_, 8, red, red, false);
}

void Player::ProcessMove(void)
{
	auto& ins = InputManager::GetInstance();

	//方向量をゼロ
	movePow_ = AsoUtility::VECTOR_ZERO;

	//X軸回転を除いた、重力方向に垂直なカメラ角度(XZ平面)を取得
	Quaternion cameraRot = mainCamera->GetQuaRotOutX();

	//方向
	VECTOR dir = AsoUtility::VECTOR_ZERO;

	double rotRad = 0;

	if (!isAttack_ && !isAttack2_ && !exAttack_ && IsEndLandingA())
	{
		if (ins.IsNew(KEY_INPUT_W))
		{
			dir = cameraRot.GetForward();
			rotRad = AsoUtility::Deg2RadF(0.0f);
		}
		if (ins.IsNew(KEY_INPUT_S))
		{
			dir = cameraRot.GetBack();
			rotRad = AsoUtility::Deg2RadF(180.0f);
		}
		if (ins.IsNew(KEY_INPUT_D))
		{
			dir = cameraRot.GetRight();
			rotRad = AsoUtility::Deg2RadF(90.0f);
		}
		if (ins.IsNew(KEY_INPUT_A))
		{
			dir = cameraRot.GetLeft();
			rotRad = AsoUtility::Deg2RadF(-90.0f);
		}

		if ((!AsoUtility::EqualsVZero(dir)) &&
			(isJump_ || IsEndLanding()))
		{
			//移動量
			speed_ = SPEED_MOVE;
			if (ins.IsNew(KEY_INPUT_LSHIFT))
			{
				speed_ = SPEED_RUN;
			}


			moveDir_ = dir;
			//移動量
			movePow_ = VScale(dir, speed_);

			// 回転処理IDLE
			SetGoalRotate(rotRad);

			if ((!isJump_ && IsEndLanding()))
			{
				// アニメーション
				if (ins.IsNew(KEY_INPUT_LSHIFT))
				{
					animationController_->Play((int)ANIM_TYPE::FAST_RUN);
				}
				else
				{
					animationController_->Play((int)ANIM_TYPE::RUN);
				}
			}
		}
		else
		{
			if ((!isJump_ && IsEndLanding()))
			{
				animationController_->Play((int)ANIM_TYPE::IDLE);
			}
		}
	}
}

void Player::SetGoalRotate(double rotRad)
{

	VECTOR cameraRot = mainCamera->GetAngles();

	Quaternion axis =
		Quaternion::AngleAxis(
			(double)cameraRot.y + rotRad, AsoUtility::AXIS_Y);

	// 現在設定されている回転との角度差を取る
	double angleDiff = Quaternion::Angle(axis, goalQuaRot_);

	// しきい値
	if (angleDiff > 0.1)
	{
		stepRotTime_ = TIME_ROT;
	}

	goalQuaRot_ = axis;
}

void Player::Rotate(void)
{

	stepRotTime_ -= scnMng_.GetDeltaTime();

	// 回転の球面補間
	playerRotY_ = Quaternion::Slerp(
		playerRotY_, goalQuaRot_, (TIME_ROT - stepRotTime_) / TIME_ROT);

}

void Player::Collision(void)
{
	// 現在座標を起点に移動後座標を決める
	movedPos_ = VAdd(transform_.pos, movePow_);

	// 衝突(カプセル)
	CollisionCapsule();

	// 衝突(重力)
	CollisionGravity();

	// 移動
	moveDiff_ = VSub(movedPos_, transform_.pos);
	transform_.pos = movedPos_;

	collisionPos_ = VAdd(transform_.pos, collisionLocalPos_);
}

void Player::CollisionGravity(void)
{
	movedPos_ = VAdd(movedPos_, jumpPow_);

	const VECTOR dirGravity = grvMng_.GetDirGravity();
	const VECTOR dirUp = grvMng_.GetDirUpGravity();
	const float gravityPow = grvMng_.GetPower();
	constexpr float checkLength = 10.0f;

	VECTOR rayStart = VAdd(movedPos_, VScale(dirUp, gravityPow + checkLength * 2.0f));
	VECTOR rayEnd = VAdd(movedPos_, VScale(dirGravity, checkLength));

	for (const auto& c : colliders_)
	{
		auto collider = c.lock();
		if (!collider) continue;

		auto hit = MV1CollCheck_Line(collider->modelId_, -1, rayStart, rayEnd);

		if (hit.HitFlag > 0 && VDot(dirGravity, jumpPow_) > 0.9f)
		{
			movedPos_ = VAdd(hit.HitPosition, VScale(dirUp, 2.0f));
			jumpPow_ = AsoUtility::VECTOR_ZERO;
			stepJump_ = 0.0f;

			if (isJump_)
			{
				animationController_->Play((int)ANIM_TYPE::JUMP, false, 29.0f, 45.0f, false, true);
			}

			isJump_ = false;
		}
	}
}

void Player::CollisionCapsule(void)
{
	Transform trans = transform_;
	trans.pos = movedPos_;
	trans.Update();
	Capsule cap(*capsule_, trans);

	for (const auto& c : colliders_)
	{
		auto collider = c.lock();
		if (!collider) continue;

		MV1_COLL_RESULT_POLY_DIM hits = MV1CollCheck_Capsule(
			collider->modelId_, -1,
			cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius());

		for (int i = 0; i < hits.HitNum; ++i)
		{
			const auto& hit = hits.Dim[i];

			for (int tryCnt = 0; tryCnt < 6; ++tryCnt) // 回数制限を厳しく
			{
				if (!HitCheck_Capsule_Triangle(
					cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius(),
					hit.Position[0], hit.Position[1], hit.Position[2]))
					break;

				// 小さな押し戻し
				const float pushBack = 1.0f;
				movedPos_ = VAdd(movedPos_, VScale(hit.Normal, pushBack));

				trans.pos = movedPos_;
				trans.Update();
				std::unique_ptr<Capsule> cap = std::make_unique<Capsule>(*capsule_, trans); // 更新後再構築
			}
		}

		MV1CollResultPolyDimTerminate(hits);
	}
}

void Player::CollisionAttack(void)
{
	if (isAttack_ || enemy_)
	{
		//エネミーとの衝突判定
		
		// 攻撃の球の半径（例: 50.0f）
		float attackRadius = 100.0f;
		// 攻撃の方向（プレイヤーの前方）
		VECTOR forward = transform_.quaRot.GetForward();
		// 攻撃の開始位置と終了位置
		VECTOR attackPos = VAdd(transform_.pos, VScale(forward, 100.0f));
		attackPos.y += 100.0f;  // 攻撃の高さ調整

		for (const auto& enemy : *enemy_)
		{
			if (!enemy || !enemy->IsAlive()) continue;

			//敵の当たり判定とサイズ
			VECTOR enemyPos = enemy->GetCollisionPos();
			float enemyRadius = enemy->GetCollisionRadius();

			//判定の距離の比較
			VECTOR diff = VSub(enemyPos, attackPos);
			float dis = AsoUtility::SqrMagnitudeF(diff);

			// 半径の合計
			float radiusSum = attackRadius + enemyRadius;

			if (dis < radiusSum * radiusSum)
			{
				enemy->Damage(2);
				// 1体のみヒット
				break;
			}
		}
	}
}

void Player::CollisionAttack2(void)
{
	if (isAttack2_ || enemy_)
	{
		//エネミーとの衝突判定

		// 攻撃の球の半径（例: 50.0f）
		float attackRadius = 100.0f;
		// 攻撃の方向（プレイヤーの前方）
		VECTOR forward = transform_.quaRot.GetForward();
		// 攻撃の開始位置と終了位置
		VECTOR attackPos = VAdd(transform_.pos, VScale(forward, 100.0f));
		attackPos.y += 100.0f;  // 攻撃の高さ調整

		for (const auto& enemy : *enemy_)
		{
			if (!enemy || !enemy->IsAlive()) continue;

			//敵の当たり判定とサイズ
			VECTOR enemyPos = enemy->GetCollisionPos();
			float enemyRadius = enemy->GetCollisionRadius();

			//判定の距離の比較
			VECTOR diff = VSub(enemyPos, attackPos);
			float dis = AsoUtility::SqrMagnitudeF(diff);

			// 半径の合計
			float radiusSum = attackRadius + enemyRadius;

			if (dis < radiusSum * radiusSum)
			{
				enemy->Damage(1);
				// 複数ヒット
				continue;
			}
		}
	}
}

void Player::CollisionAttackEx(void)
{
	if (exAttack_ || enemy_)
	{
		//エネミーとの衝突判定

		// 攻撃の球の半径（例: 50.0f）
		float attackRadius = 150.0f;
		// 攻撃の開始位置と終了位置
		VECTOR attackPos = transform_.pos;
		attackPos.y += 100.0f;  // 攻撃の高さ調整

		for (const auto& enemy : *enemy_)
		{
			if (!enemy || !enemy->IsAlive()) continue;

			//敵の当たり判定とサイズ
			VECTOR enemyPos = enemy->GetCollisionPos();
			float enemyRadius = enemy->GetCollisionRadius();

			//判定の距離の比較
			VECTOR diff = VSub(enemyPos, attackPos);
			float dis = AsoUtility::SqrMagnitudeF(diff);

			// 半径の合計
			float radiusSum = attackRadius + enemyRadius;

			if (dis < radiusSum * radiusSum)
			{
				enemy->Damage(2);
				// 複数ヒットさせたいなら
				continue;
			}
		}
	}
}

void Player::CalcGravityPow(void)
{
	// 重力方向
	VECTOR dirGravity = grvMng_.GetDirGravity();

	// 重力の強さ
	float gravityPow = grvMng_.GetPower();

	// 重力
	// 重力を作る
	// メンバ変数 jumpPow_ に重力計算を行う(加速度)
	VECTOR gravity = VScale(dirGravity, gravityPow);
	jumpPow_ = VAdd(jumpPow_, gravity);

	// 内積
	float dot = VDot(dirGravity, jumpPow_);
	if (dot >= 0.0f)
	{
		// 重力方向と反対方向(マイナス)でなければ、ジャンプ力を無くす
		jumpPow_ = gravity;
	}
}

void Player::ProcessJump(void)
{

	bool isHit = CheckHitKey(KEY_INPUT_BACK);

	// ジャンプ
	if (isHit && !isAttack_ && !isAttack2_ && !exAttack_ && (isJump_ || IsEndLanding()))
	{
		if (!isJump_)
		{
			// 制御無しジャンプ
			//animationController_->Play((int)ANIM_TYPE::JUMP);

			// この後、いくつかのジャンプパターンを試します
			//無理やりアニメーション
			animationController_->Play((int)ANIM_TYPE::JUMP, true, 13.0f, 25.0f);
			animationController_->SetEndLoop(23.0f, 25.0f, 5.0f);
		}

		isJump_ = true;

		// ジャンプの入力受付時間をヘラス
		stepJump_ += scnMng_.GetDeltaTime();

		if (stepJump_ < TIME_JUMP_IN)
		{
			jumpPow_ = VScale(grvMng_.GetDirUpGravity(), POW_JUMP);
		}

	}

	// ボタンを離したらジャンプ力に加算しない
	if (!isHit)
	{
		stepJump_ = TIME_JUMP_IN;
	}

}

bool Player::IsEndLanding(void)
{
	bool ret = true;

	// アニメーションがジャンプではない
	if (animationController_->GetPlayType() != (int)ANIM_TYPE::JUMP)
	{
		return ret;
	}
	//アニメーションが終了しているか
	if (animationController_->IsEnd())
	{
		return ret;
	}
	return false;

}

void Player::ProcessAttack(void)
{
	bool isHit = CheckHitKey(KEY_INPUT_E);
	bool isHit_N = CheckHitKey(KEY_INPUT_Q);
	bool isHit_E = CheckHitKey(KEY_INPUT_R);

	// アタック
	if (!isJump_ && (isAttack_ || IsEndLandingA()))
	{
		if (!isAttack_ && !isAttack2_ && !exAttack_ && isHit)
		{
			animationController_->Play((int)ANIM_TYPE::ATTACK1, false);
			isAttack_ = true;

			// 衝突(攻撃)
			CollisionAttack();
		}
		else if (!isAttack2_ && !isAttack_ && !exAttack_ && isHit_N)
		{
			// Treeのレベルが25以上でクールタイム10秒がたっているならATTACK1を許可
			if (tree_ && tree_->GetLv() >= 25 && !isAttack2_)
			{
				animationController_->Play((int)ANIM_TYPE::ATTACK2, false);
				isAttack2_ = true;

				// 衝突(攻撃)
				CollisionAttack2();
			}
		}
		else if (!exAttack_ && !isAttack_ && !isAttack2_ && isHit_E)
		{
			// Treeのレベルが25以上でクールタイム10秒がたっているならATTACK1を許可
			if (tree_ && tree_->GetLv() >= 50 && !exAttack_ && IsExAttackReady())
			{
				animationController_->Play((int)ANIM_TYPE::EXATTACK, false);
				exAttack_ = true;
				lastExTime_ = GetNowCount(); // ← クールタイム開始

				// 衝突(攻撃)
				CollisionAttackEx();
			}
		}
	}

	// アニメーションが終わったらフラグをリセット
	if (animationController_->IsEnd())
	{
		if (isAttack_)
		{
			isAttack_ = false;
		}
		if (isAttack2_)
		{
			isAttack2_ = false;
		}
		if (exAttack_)
		{
			exAttack_ = false;
		}
	}
}

bool Player::IsEndLandingA(void)
{
	bool ret = true;
	int animType = animationController_->GetPlayType();

	// 現在のアニメーションが ATTACK1,2 または EXATTACK のいずれかで、まだ終了していない場合
	if ((animType != (int)ANIM_TYPE::ATTACK1 || animType == (int)ANIM_TYPE::ATTACK2 
		|| animType == (int)ANIM_TYPE::EXATTACK) 
		&& !animationController_->IsEnd())
	{
		return ret;
	}
	// アニメーションが終了しているか
	if (animationController_->IsEnd())
	{
		return ret;
	}
	return false;
}

bool Player::IsExAttackReady() const
{
	int now = GetNowCount(); // DxLib の現在時刻（ミリ秒）
	return (now - lastExTime_) >= exTimer_;
}

void Player::Damage(int damage)
{
	if (pstate_ == PlayerState::DOWN) return;  // ダウン中は無敵
	hp_ -= damage;

	if (hp_ <= 0) {
		hp_ = 0;
		StartRevival();  // 死亡ではなく復活待機
	}
}

void Player::StartRevival()
{
	invincible_ = true;   // 無敵状態にする
	canMove_ = false;     // 移動停止

	pstate_ = PlayerState::DOWN;
	revivalTimer_ = 0.0f;

	animationController_->Play((int)ANIM_TYPE::DOWN, false);
	// 必要なら移動や入力を停止させる
}

void Player::Revival()
{
	hp_ = HP;
	pstate_ = PlayerState::NORMAL;

	// 復活後の無敵状態を解除
	invincible_ = false;   // 無敵解除
	// プレイヤーが移動可能になる
	canMove_ = true;   // 移動再開

	animationController_->Play((int)ANIM_TYPE::IDLE, true);
	// 他の再開処理（無敵終了、移動可能など）をここで
}

void Player::EffectFootSmoke(void)
{
	float len = AsoUtility::MagnitudeF(moveDiff_);

	stepFootSmoke_ -= scnMng_.GetDeltaTime();

	//if (!AsoUtility::EqualsVZero(moveDiff_))
	if (stepFootSmoke_ < 0.0f && len >= 1.0f && !isJump_)
	{

		stepFootSmoke_ = TERM_FOOT_SMOKE;

		// エフェクト再生
		effectSmokePleyId_ = PlayEffekseer3DEffect(effectSmokeResId_);

		//エフェクトの大きさ
		SetScalePlayingEffekseer3DEffect(effectSmokePleyId_, 5.0f, 5.0f, 5.0f);

		//エフェクトの位置
		SetPosPlayingEffekseer3DEffect(effectSmokePleyId_,
			transform_.pos.x, transform_.pos.y, transform_.pos.z);
	}
}

int Player::GetWater(void) const
{
	return water_;
}

bool Player::IsMax(void)
{
	return isMax_;
}
void Player::SetIsMax(void)
{
	isMax_ = false;
}

void Player::SetTree(Tree* tree)
{
	tree_ = tree;
}

void Player::eHit(void)
{

}
void Player::wHit(float scale)
{
	// 増加量
	int add = 1;

	// スケールに応じて増加量を変える
	if (scale >= 0.2f) {
		add = 3;
	}
	else if (scale >= 0.15f) {
		add = 2;
	}
	// それ未満は1
	water_+= add;
	if (water_ > WATER_MAX)water_ = WATER_MAX;
}
void Player::tHit()
{
	if (water_ == WATER_MAX)
	{
		isMax_ = true;
		water_ = 0;
		hp_ = 45;
	}
	else
	{
		water_--;
		hp_ += 3;
	}
	if (hp_ > HP)hp_ = HP;
}