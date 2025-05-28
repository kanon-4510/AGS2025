#include <string>
#include <vector>
#include "../Application.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/GravityManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/ResourceManager.h"
#include "../Utility/AsoUtility.h"
#include "Common/Capsule.h"
#include "Common/Collider.h"
#include "Common/AnimationController.h"
#include "ActorBase.h"
#include "Player.h"
#include "EnemyGhost.h"
#include "EnemyBase.h"

EnemyBase::EnemyBase(int baseModelId)
{
	// 敵のモデル
	baseModelId_[static_cast<int>(TYPE::BIRD)] = baseModelId;

	animationController_ = nullptr;
	state_ = STATE::NONE;
	isGround_ = false;

	jumpPow_ = AsoUtility::VECTOR_ZERO;

	// 衝突チェック
	gravHitPosDown_ = AsoUtility::VECTOR_ZERO;
	gravHitPosUp_ = AsoUtility::VECTOR_ZERO;

	// 状態管理
	stateChanges_.emplace(
		STATE::NONE, std::bind(&EnemyBase::ChangeStateNone, this));
	stateChanges_.emplace(
		STATE::PLAY, std::bind(&EnemyBase::ChangeStatePlay, this));

	// 初期状態関数を必ず設定する！
	auto it = stateChanges_.find(state_);
	if (it != stateChanges_.end()) {
		stateUpdate_ = it->second;
	}
	else {
		stateUpdate_ = []() {};  // デフォルト空関数でクラッシュ回避
	}
}

EnemyBase::~EnemyBase(void)
{

}

void EnemyBase::Init(void)
{
	SetParam();
	InitLoad();
	//Update();
}

void EnemyBase::InitLoad(void)
{
	std::string path = Application::PATH_MODEL + "Enemy/";

	imgShadow_ = resMng_.Load(
		ResourceManager::SRC::PLAYER_SHADOW).handleId_;

	//modelId_ = MV1LoadModel((Application::PATH_MODEL + "Enemy/Yellow.mv1").c_str());

	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	animationController_->Add((int)ANIM_TYPE::RUN, path + "Run.mv1", 20.0f);
	animationController_->Add((int)ANIM_TYPE::ATTACK, path + "Attack.mv1", 60.0f);
	animationController_->Add((int)ANIM_TYPE::DAMAGE, path + "Dgame.mv1", 60.0f);
	animationController_->Add((int)ANIM_TYPE::DEATH, path + "Death.mv1", 60.0f);

	animationController_->Play((int)ANIM_TYPE::RUN);
}

void EnemyBase::SetParam(void)
{
	// 使用メモリ容量と読み込み時間の削減のため
	// モデルデータをいくつもメモリ上に存在させない
	transform_.modelId = MV1DuplicateModel(baseModelId_[static_cast<int>(TYPE::BIRD)]);

	transform_.scl = { 1.0f, 1.0f, 1.0f };						// 大きさの設定
	transform_.rot = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };	// 角度の設定
	transform_.pos = { 00.0f, 0.0f, 1000.0f };				// 位置の設定
	dir_ = { 0.0f, 0.0f, -1.0f };								// 右方向に移動する

	speed_ = 01.0f;		// 移動スピード

	isAlive_ = true;	// 初期は生存状態

	//animAttachNo_ = MV1AttachAnim(modelId_, 0);	// アニメーションをアタッチする
	//animTotalTime_ = MV1GetAttachAnimTotalTime(modelId_, animAttachNo_);	// アタッチしているアニメーションの総再生時間を取得する
	stepAnim_ = 0.0f;	// 再生中のアニメーション時間
	speedAnim_ = 30.0f;	// アニメーション速度

	hp_ = hpMax_ = 2;	// HPの設定

	collisionRadius_ = 100.0f;	// 衝突判定用の球体半径
	collisionLocalPos_ = { 0.0f, 60.0f, 0.0f };	// 衝突判定用の球体中心の調整座標

	// カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 00.0f, 130.0f, 1.0f });
	capsule_->SetLocalPosDown({ 00.0f, 0.0f, 1.0f });
	capsule_->SetRadius(30.0f);
}

void EnemyBase::Update(void)
{

	// 更新ステップ
	stateUpdate_();

	transform_.Update();

	// アニメーション再生
	animationController_->Update();

	//UpdateD(1.0f);

}

void EnemyBase::EnemyUpdate(void)
{
	if (isAlive_)
	{
		// 重力の加算（jumpPow_の更新）
		if (!isGround_)
		{
			CalcGravityPow();
		}
		
		// 移動ベクトルの計算
		movePow_ = VScale(dir_, speed_);
		//movePow_ = VAdd(movePow_, jumpPow_);

		// 仮の移動先座標を計算（ここで pos を直接変更しない！）
		movedPos_ = VAdd(transform_.pos, movePow_);
		

		// 衝突処理（movedPos_ を調整）
		Collision();

		// 移動を反映
		transform_.pos = movedPos_;

		// モデルに反映
		MV1SetScale(transform_.modelId, transform_.scl);
		MV1SetRotationXYZ(transform_.modelId, transform_.rot);
		MV1SetPosition(transform_.modelId, transform_.pos);
	}
}

void EnemyBase::UpdateNone(void)
{
}

void EnemyBase::Draw(void)
{
	if (!isAlive_)
	{
		return;
	}

	// モデルの描画
	MV1DrawModel(transform_.modelId);

	// 丸影描画
	DrawShadow();

	// デバッグ用描画
	DrawDebug();
}

void EnemyBase::DrawShadow(void)
{

	int i, j;
	MV1_COLL_RESULT_POLY_DIM HitResDim;
	MV1_COLL_RESULT_POLY* HitRes;
	VERTEX3D Vertex[3];
	VECTOR SlideVec;
	int ModelHandle;

	// ライティングを無効にする
	SetUseLighting(FALSE);

	// Ｚバッファを有効にする
	SetUseZBuffer3D(TRUE);

	// テクスチャアドレスモードを CLAMP にする( テクスチャの端より先は端のドットが延々続く )
	SetTextureAddressMode(DX_TEXADDRESS_CLAMP);

	// 影を落とすモデルの数だけ繰り返し
	for (const auto c : colliders_)
	{
		// 地面との衝突
		auto hit = MV1CollCheck_Line(
			c.lock()->modelId_, -1, gravHitPosUp_, gravHitPosDown_);

		// プレイヤーの直下に存在する地面のポリゴンを取得
		HitResDim = MV1CollCheck_Capsule(
			c.lock()->modelId_,
			-1,
			transform_.pos,
			VAdd(transform_.pos, VGet(0.0f, -ENEMY_SHADOW_HEIGHT, 0.0f)), ENEMY_SHADOW_SIZE);

		// 頂点データで変化が無い部分をセット
		Vertex[0].dif = GetColorU8(255, 255, 255, 255);
		Vertex[0].spc = GetColorU8(0, 0, 0, 0);
		Vertex[0].su = 0.0f;
		Vertex[0].sv = 0.0f;
		Vertex[1] = Vertex[0];
		Vertex[2] = Vertex[0];

		// 球の直下に存在するポリゴンの数だけ繰り返し
		HitRes = HitResDim.Dim;
		for (i = 0; i < HitResDim.HitNum; i++, HitRes++)
		{
			// ポリゴンの座標は地面ポリゴンの座標
			Vertex[0].pos = HitRes->Position[0];
			Vertex[1].pos = HitRes->Position[1];
			Vertex[2].pos = HitRes->Position[2];

			// ちょっと持ち上げて重ならないようにする
			SlideVec = VScale(HitRes->Normal, 0.5f);
			Vertex[0].pos = VAdd(Vertex[0].pos, SlideVec);
			Vertex[1].pos = VAdd(Vertex[1].pos, SlideVec);
			Vertex[2].pos = VAdd(Vertex[2].pos, SlideVec);

			// ポリゴンの不透明度を設定する
			Vertex[0].dif.a = 0;
			Vertex[1].dif.a = 0;
			Vertex[2].dif.a = 0;
			if (HitRes->Position[0].y > transform_.pos.y - ENEMY_SHADOW_HEIGHT)
				Vertex[0].dif.a = 128 * (1.0f - fabs(HitRes->Position[0].y - transform_.pos.y) / ENEMY_SHADOW_HEIGHT);

			if (HitRes->Position[1].y > transform_.pos.y - ENEMY_SHADOW_HEIGHT)
				Vertex[1].dif.a = 128 * (1.0f - fabs(HitRes->Position[1].y - transform_.pos.y) / ENEMY_SHADOW_HEIGHT);

			if (HitRes->Position[2].y > transform_.pos.y - ENEMY_SHADOW_HEIGHT)
				Vertex[2].dif.a = 128 * (1.0f - fabs(HitRes->Position[2].y - transform_.pos.y) / ENEMY_SHADOW_HEIGHT);

			// ＵＶ値は地面ポリゴンとプレイヤーの相対座標から割り出す
			Vertex[0].u = (HitRes->Position[0].x - transform_.pos.x) / (ENEMY_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[0].v = (HitRes->Position[0].z - transform_.pos.z) / (ENEMY_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[1].u = (HitRes->Position[1].x - transform_.pos.x) / (ENEMY_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[1].v = (HitRes->Position[1].z - transform_.pos.z) / (ENEMY_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[2].u = (HitRes->Position[2].x - transform_.pos.x) / (ENEMY_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[2].v = (HitRes->Position[2].z - transform_.pos.z) / (ENEMY_SHADOW_SIZE * 2.0f) + 0.5f;

			// 影ポリゴンを描画
			DrawPolygon3D(Vertex, 1, imgShadow_, TRUE);
		}

		// 検出した地面ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(HitResDim);
	}

	//デバッグ
	DrawDebug();

	// 視野範囲の描画
	DrawDebugSearchRange();
}

void EnemyBase::DrawDebug(void)
{

	int white = 0xffffff;
	int black = 0x000000;
	int red = 0xff0000;
	int green = 0x00ff00;
	int blue = 0x0000ff;
	int yellow = 0xffff00;
	int purpl = 0x800080;

	VECTOR v;
	VECTOR c;
	VECTOR s;

	// キャラ基本情報
	//-------------------------------------------------------
	// キャラ座標
	v = transform_.pos;
	DrawFormatString(20, 120, white, "キャラ座標 ： (%0.2f, %0.2f, %0.2f)",
		v.x, v.y, v.z
	);

	capsule_->Draw();
	c = capsule_->GetPosDown();
	DrawFormatString(20, 150, white, "コリジョン座標 ： (%0.2f, %0.2f, %0.2f)",
		c.x, c.y, c.z
	);

	s = spherePos_;
	DrawSphere3D(s, collisionRadius_, 8, red, red, false);
	DrawFormatString(20, 180, white, "スフィア座標 ： (%0.2f, %0.2f, %0.2f)",
		s.x, s.y, s.z
	);

	int animNum = MV1GetAnimNum(transform_.modelId);
	if (animNum == 0) {
		DrawFormatString(20, 260, 0xff0000, "このモデルにはアニメーションがありません");
	}

	DrawFormatString(20, 290, 0xffffff, "ジャンプ力：(%0.2f, %0.2f, %0.2f)", jumpPow_.x, jumpPow_.y, jumpPow_.z);
	DrawFormatString(20, 320, GetColor(255, 255, 255), "colliders_ size: %d", colliders_.size());

}

void EnemyBase::Release(void)
{
	MV1DeleteModel(transform_.modelId);
}

void EnemyBase::ChangeState(STATE state)
{
	// 状態変更
	state_ = state;

	// 各状態遷移の初期処理
	stateChanges_[state_]();
}

void EnemyBase::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&EnemyBase::UpdateNone, this);
}

void EnemyBase::ChangeStatePlay(void)
{
	stateUpdate_ = std::bind(&EnemyBase::EnemyUpdate, this);
}

bool EnemyBase::IsPlay(void) const
{
	return state_ == STATE::PLAY;
}

void EnemyBase::Rotate(void)
{
	stepRotTime_ -= scnMng_.GetDeltaTime();

	// 回転の球面補間
	enemyRotY_ = Quaternion::Slerp(
		enemyRotY_, goalQuaRot_, (TIME_ROT - stepRotTime_) / TIME_ROT);
}

void EnemyBase::CalcGravityPow(void)
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
		jumpPow_ = AsoUtility::VECTOR_ZERO;
	}
}

void EnemyBase::Collision(void)
{
	// 現在座標を起点に移動後座標を決める
	movedPos_ = VAdd(transform_.pos, movePow_);
	
	//衝突
	CollisionCapsule();
	//衝突(重力)
	CollisionGravity();
	
	// 移動
	moveDiff_ = VSub(movedPos_, transform_.pos);
	transform_.pos = movedPos_;
	spherePos_ = VAdd(transform_.pos, collisionLocalPos_);
}

void EnemyBase::CollisionGravity(void)
{
	// ジャンプ量を加算
	movedPos_ = VAdd(movedPos_, jumpPow_);

	// 重力方向
	VECTOR dirGravity = grvMng_.GetDirGravity();
	// 重力方向の反対
	VECTOR dirUpGravity = grvMng_.GetDirUpGravity();
	// 重力の強さ
	float gravityPow = grvMng_.GetPower();
	float checkPow = 30.0f; // ←拡大

	gravHitPosUp_ = VAdd(movedPos_, VScale(dirUpGravity, gravityPow));
	gravHitPosUp_ = VAdd(gravHitPosUp_, VScale(dirUpGravity, checkPow * 2.0f));
	gravHitPosDown_ = VAdd(movedPos_, VScale(dirGravity, checkPow));

	isGround_ = false;

	for (const auto c : colliders_)
	{
		// 地面との衝突
		auto hit = MV1CollCheck_Line(
			c.lock()->modelId_, -1, gravHitPosUp_, gravHitPosDown_);

		if (hit.HitFlag > 0 && VDot(dirGravity, jumpPow_) > 0.9f)
		{
			// 衝突地点から、少し上に移動

			// 地面と衝突している
			// 押し戻し処理とジャンプ力の打ち消しを実装しましょう

	animationController_ = std::make_unique<AnimationController>(transform_.modelId);
	animationController_->Add((int)ANIM_TYPE::RUN, path + "Run.mv1", 20.0f);
	animationController_->Add((int)ANIM_TYPE::ATTACK, path + "Attack.mv1", 60.0f);
	animationController_->Add((int)ANIM_TYPE::DAMAGE, path + "Dgame.mv1", 60.0f);
	animationController_->Add((int)ANIM_TYPE::DEATH, path + "Death.mv1", 60.0f);

			movedPos_ = VAdd(hit.HitPosition, VScale(dirUpGravity, 0.9f));
			jumpPow_ = AsoUtility::VECTOR_ZERO;
			isGround_ = true;
			break;
		}
	}
}

void EnemyBase::CollisionCapsule(void)
{
	// カプセルを移動させる
	Transform trans = Transform(transform_);
	trans.pos = movedPos_;
	trans.Update();
	Capsule cap = Capsule(*capsule_, trans);
	// カプセルとの衝突判定
	for (const auto c : colliders_)
	{
		auto hits = MV1CollCheck_Capsule(
			c.lock()->modelId_, -1,
			cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius());
		// 衝突した複数のポリゴンと衝突回避するまで、
		// プレイヤーの位置を移動させる
		for (int i = 0; i < hits.HitNum; i++)
		{
			auto hit = hits.Dim[i];
			// 地面と異なり、衝突回避位置が不明なため、何度か移動させる
			// この時、移動させる方向は、移動前座標に向いた方向であったり、
			// 衝突したポリゴンの法線方向だったりする
			for (int tryCnt = 0; tryCnt < 10; tryCnt++)
			{
				// 再度、モデル全体と衝突検出するには、効率が悪過ぎるので、
				// 最初の衝突判定で検出した衝突ポリゴン1枚と衝突判定を取る
				int pHit = HitCheck_Capsule_Triangle(
					cap.GetPosTop(), cap.GetPosDown(), cap.GetRadius(),
					hit.Position[0], hit.Position[1], hit.Position[2]);
				if (pHit)
				{
					// 法線の方向にちょっとだけ移動させる
					movedPos_ = VAdd(movedPos_, VScale(hit.Normal, 1.0f));
					// カプセルも一緒に移動させる
					trans.pos = movedPos_;
					trans.Update();
					continue;
				}
				break;
			}
		}
		// 検出した地面ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(hits);
	}
}

void EnemyBase::AddCollider(std::weak_ptr<Collider> collider)
{
	colliders_.push_back(collider);
}

void EnemyBase::ClearCollider(void)
{
	colliders_.clear();
}

const Capsule& EnemyBase::GetCapsule(void) const
{
	return *capsule_;
}

VECTOR EnemyBase::GetPos(void)
{
	return transform_.pos;
}

void EnemyBase::SetPos(VECTOR pos)
{
	transform_.pos = pos;
}

void EnemyBase::SetCollisionPos(const VECTOR collision)
{
	spherePos_ = collision;
}

VECTOR EnemyBase::GetCollisionPos(void)const
{
	return VAdd(collisionLocalPos_, transform_.pos);
}

float EnemyBase::GetCollisionRadius(void)
{
	return collisionRadius_;
}

bool EnemyBase::IsAlive(void)
{
	return isAlive_;
}

void EnemyBase::SetAlive(bool alive)
{
	isAlive_ = alive;
}

void EnemyBase::Damage(int damage)
{
	hp_ -= damage;
	if (hp_ <= 0)
	{
		hp_ = 0;
		isAlive_ = false;
	int animNum = MV1GetAnimNum(transform_.modelId);
	if (animNum == 0) {
		DrawFormatString(20, 260, 0xff0000, "このモデルにはアニメーションがありません");
	}
}

void EnemyBase::DrawDebugSearchRange(void)
{
	//VECTOR centerPos = transform_.pos;
	//float radius = VIEW_RANGE;
	//int segments = 60; // 分割数（多いほど滑らか）

	//float angleStep = DX_PI * 2.0f / segments; // 360度分割の角度ステップ

	//for (int i = 0; i < segments; ++i)
	//{
	//	float angle1 = angleStep * i;
	//	float angle2 = angleStep * (i + 1);

	//	VECTOR p1 = {
	//		centerPos.x + radius * sinf(angle1),
	//		centerPos.y,
	//		centerPos.z + radius * cosf(angle1)
	//	};

	//	VECTOR p2 = {
	//		centerPos.x + radius * sinf(angle2),
	//		centerPos.y,
	//		centerPos.z + radius * cosf(angle2)
	//	};

	//	DrawTriangle3D(centerPos, p1, p2, 0xffffff, false);
	//}

	//DrawSphere3D(centerPos, 20.0f, 10, 0x00ff00, 0x00ff00, true);

	VECTOR centerPos = transform_.pos;
	float radius = VIEW_RANGE;
	int segments = 60;

	// プレイヤーの座標
	VECTOR playerPos = player_->GetTransform().pos; // プレイヤーオブジェクトの参照を持っている想定

	// プレイヤーと敵の距離（XZ平面）
	float dx = playerPos.x - centerPos.x;
	float dz = playerPos.z - centerPos.z;
	float distance = sqrtf(dx * dx + dz * dz);

	// 範囲内か判定
	bool inRange = (distance <= radius);

	// 色を決定（範囲内なら赤、範囲外は元の色）
	unsigned int color = inRange ? 0xff0000 : 0xffdead;

	float angleStep = DX_PI * 2.0f / segments;

	for (int i = 0; i < segments; ++i)
	{
		float angle1 = angleStep * i;
		float angle2 = angleStep * (i + 1);

		VECTOR p1 = {
			centerPos.x + radius * sinf(angle1),
			centerPos.y,
			centerPos.z + radius * cosf(angle1)
		};

		VECTOR p2 = {
			centerPos.x + radius * sinf(angle2),
			centerPos.y,
			centerPos.z + radius * cosf(angle2)
		};

		DrawTriangle3D(centerPos, p1, p2, color, false);
	}

	DrawSphere3D(centerPos, 20.0f, 10, 0x00ff00, 0x00ff00, true);
}

void EnemyBase::SetPlayer(std::shared_ptr<Player> player)
{
	player_ = player;
}
