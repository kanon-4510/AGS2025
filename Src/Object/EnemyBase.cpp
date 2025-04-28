#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/GravityManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/ResourceManager.h"
#include "../Utility/AsoUtility.h"
#include "Common/Capsule.h"
#include "Common/Collider.h"
#include "ActorBase.h"
#include "EnemyBase.h"

EnemyBase::EnemyBase(int baseModelId)
{
	baseModelId_[static_cast<int>(TYPE::BIRD)] = baseModelId;
	imgShadow_ = -1;
}

EnemyBase::~EnemyBase(void)
{

}

void EnemyBase::Init(void)
{
	// 丸影画像
	imgShadow_ = resMng_.Load(
		ResourceManager::SRC::PLAYER_SHADOW).handleId_;

	SetParam();
	Update();
	EnemyUpdate();	//置く場所が分からん
}

void EnemyBase::SetParam(void)
{
	// 使用メモリ容量と読み込み時間の削減のため
	// モデルデータをいくつもメモリ上に存在させない
	modelId_ = MV1DuplicateModel(baseModelId_[static_cast<int>(TYPE::BIRD)]);

	transform_.scl = { 0.5f, 0.5f, 0.5f };					// 大きさの設定
	transform_.rot = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };	// 角度の設定
	transform_.pos = { 00.0f, 50.0f, 0.0f };				// 位置の設定
	dir_ = { 0.0f, 0.0f, -1.0f };					// 右方向に移動する


	speed_ = 01.0f;		// 移動スピード

	isAlive_ = true;	// 初期は生存状態

	animAttachNo_ = MV1AttachAnim(modelId_, 10);	// アニメーションをアタッチする
	animTotalTime_ = MV1GetAttachAnimTotalTime(modelId_, animAttachNo_);	// アタッチしているアニメーションの総再生時間を取得する
	stepAnim_ = 0.0f;	// 再生中のアニメーション時間
	speedAnim_ = 30.0f;	// アニメーション速度

	hp_ = hpMax_ = 2;	// HPの設定

	collisionRadius_ = 40.0f;	// 衝突判定用の球体半径
	collisionLocalPos_ = { 0.0f, 0.0f, 0.0f };	// 衝突判定用の球体中心の調整座標

	// カプセルコライダ
	capsule_ = std::make_unique<Capsule>(transform_);
	capsule_->SetLocalPosTop({ 00.0f, 130.0f, 1.0f });
	capsule_->SetLocalPosDown({ 00.0f, 0.0f, 1.0f });
	capsule_->SetRadius(30.0f);

	// 衝突チェック
	gravHitPosDown_ = AsoUtility::VECTOR_ZERO;
	gravHitPosUp_ = AsoUtility::VECTOR_ZERO;
}


void EnemyBase::Update(void)
{
	if (!isAlive_)
	{
		return;
	}
	transform_.pos = VAdd(transform_.pos, VScale(dir_, speed_));

	MV1SetScale(modelId_, transform_.scl);		// ３Ｄモデルの大きさを設定(引数は、x, y, zの倍率)
	MV1SetRotationXYZ(modelId_, transform_.rot);	// ３Ｄモデルの向き(引数は、x, y, zの回転量。単位はラジアン。)
	MV1SetPosition(modelId_, transform_.pos);		// ３Ｄモデルの位置(引数は、３Ｄ座標)

	// アニメーション再生
	// 経過時間の取得
	float deltaTime = 1.0f / SceneManager::DEFAULT_FPS;
	// アニメーション時間の進行
	stepAnim_ += (speedAnim_ * deltaTime);
	if (stepAnim_ > animTotalTime_)
	{
		stepAnim_ = 0.0f;		// ループ再生
	}
	// 再生するアニメーション時間の設定
	MV1SetAttachAnimTime(modelId_, animAttachNo_, stepAnim_);
}

void EnemyBase::EnemyUpdate(void)
{
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
	transform_.quaRot = transform_.quaRot.Mult(enemyRotY_);
}

void EnemyBase::Draw(void)
{
	if (!isAlive_)
	{
		return;
	}

	MV1DrawModel(modelId_);

	DrawDebug();
}

void EnemyBase::Release(void)
{
	MV1DeleteModel(modelId_);
}

VECTOR EnemyBase::GetPos(void)
{
	return VECTOR();
}

void EnemyBase::ProcessMove(void)
{
	auto& ins = InputManager::GetInstance();

	//方向量をゼロ
	movePow_ = AsoUtility::VECTOR_ZERO;

	//X軸回転を除いた、重力方向に垂直なカメラ角度(XZ平面)を取得
	Quaternion cameraRot = mainCamera->GetQuaRotOutX();

	//方向
	VECTOR dir = AsoUtility::VECTOR_ZERO;

	double rotRad = 0;

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
}

void EnemyBase::SetPos(VECTOR pos)
{
	transform_.pos = pos;
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
	}
}

const Capsule& EnemyBase::GetCapsule(void) const
{
	return *capsule_;
}

VECTOR EnemyBase::GetCollisionPos(void)
{
	return VAdd(transform_.pos, collisionLocalPos_);
}

float EnemyBase::GetCollisionRadius(void)
{
	return collisionRadius_;
}


void EnemyBase::Rotate(void)
{
	stepRotTime_ -= scnMng_.GetDeltaTime();

	// 回転の球面補間
	enemyRotY_ = Quaternion::Slerp(
		enemyRotY_, goalQuaRot_, (TIME_ROT - stepRotTime_) / TIME_ROT);
}

void EnemyBase::Collision(void)
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

	float checkPow = 10.0f;

	gravHitPosUp_ = VAdd(movedPos_, VScale(dirUpGravity, gravityPow));

	gravHitPosUp_ = VAdd(gravHitPosUp_, VScale(dirUpGravity, checkPow * 2.0f));

	gravHitPosDown_ = VAdd(movedPos_, VScale(dirGravity, checkPow));

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

			//movedPos_に押し戻し座標を設定
			//押し戻し座標については、dxlib のhit構造体の中にヒントアリ
			//衝突地点情報が格納されている

			movedPos_ = VAdd(hit.HitPosition, VScale(dirUpGravity, 1.0f));

			//jumpPow_の値をゼロにする
			//ジャンプのリセット
			jumpPow_ = AsoUtility::VECTOR_ZERO;

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

	// ライティングを有効にする
	SetUseLighting(TRUE);

	// Ｚバッファを無効にする
	SetUseZBuffer3D(FALSE);

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
		jumpPow_ = gravity;
	}
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
}