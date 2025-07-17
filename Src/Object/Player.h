#pragma once
#include <memory>
#include <map>
#include <functional>
#include <vector>
#include <DxLib.h>
#include <algorithm>
#include "ActorBase.h"
#include "EnemyBase.h"
#include "Stage.h"

class AnimationController;
class Collider;
class Capsule;
class Tree;

class Player : public ActorBase
{

public:

	// スピード
	static constexpr float SPEED_MOVE = 5.0f;
	static constexpr float SPEED_RUN = 10.0f;
	
	//ステータス変更用の値
	static constexpr float STATUS_UP = 2.0f;

	// 回転完了までの時間
	static constexpr float TIME_ROT = 1.0f;

	// ジャンプ力
	static constexpr float POW_JUMP = 35.0f;
	// ジャンプ受付時間
	static constexpr float TIME_JUMP_IN = 0.5f;

	//影の大きさ
	static constexpr float PLAYER_SHADOW_SIZE = 100.0f;
	static constexpr float PLAYER_SHADOW_HEIGHT = 300.0f;

	// 煙エフェクト発生間隔
	static constexpr float TERM_FOOT_SMOKE = 0.3f;

	//ステ関連
	static constexpr int HP = 10;
	static constexpr int D_COUNT = 600;
	static constexpr int WATER_MAX = 10;

	//ステータスアップ
	static constexpr int POWER_UP_TIME = 1200;
	static constexpr int SPEED_UP_TIME = 1200;

	// 状態
	enum class STATE
	{
		NONE,
		PLAY,
		END
	};

	// PLAY中の状態
	enum class PlayerState {
		NORMAL,
		DOWN,
		// 他にも Jumping, Attacking など必要なら追加
	};
	PlayerState pstate_ = PlayerState::NORMAL;

	// アニメーション種別
	enum class ANIM_TYPE
	{
		IDLE,
		RUN,
		FAST_RUN,
		JUMP,
		DOWN,
		ATTACK1,
		ATTACK2,
		EXATTACK,
	};

	// コンストラクタ
	Player(void);

	// デストラクタ
	~Player(void);

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

	// --- ゲームステート処理 ---
	bool IsPlay(void) const;	//状態確認

	// --- 当たり判定 / 衝突 ---
	
	// 衝突判定に用いられるコライダ制御
	void AddCollider(std::weak_ptr<Collider> collider);
	void ClearCollider(void);
	const Capsule& GetCapsule(void) const;// 衝突用カプセルの取得

	VECTOR GetCollisionPos(void)const;	// 衝突用の中心座標の取得
	float GetCollisionRadius(void);		// 衝突用の球体半径の取得
	const std::vector<std::shared_ptr<EnemyBase>>& GetEnemyCollision(void) const;

	// --- 外部インターフェース ---
	void SetEnemy(const std::vector<std::shared_ptr<EnemyBase>>* enemys);//敵の情報を取得
	VECTOR GetPos() const;
	void SetPos(const VECTOR& pos);
	void SetTree(Tree* tree);

	// --- インタラクション（ヒット系） ---
	void wHit(float scale);	//水を取得
	void tHit(void);		//木にヒット

	// --- ステータス操作 ---
	int GetWater(void) const;

	//水獲得量特別増加
	bool IsMax(void);
	void SetIsMax(void);
	
	void Damage(int damage);//ダメージ
	void PowerUp(void);		//パワーアップ
	void SpeedUp(void);		//スピードアップ
	void Heal(void);		//回復
	void Muteki(void);		//無敵

private:

	// --- アニメーション ---
	std::unique_ptr<AnimationController> animationController_;
	void InitAnimation(void);

	// --- 状態管理 ---
	STATE state_;
	std::map<STATE, std::function<void(void)>> stateChanges_;// 状態管理(状態遷移時初期処理)
	std::function<void(void)> stateUpdate_;// 状態管理(更新ステップ)

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStatePlay(void);

	void UpdatePlay(void);//stateがplayの状態のupdate
	void UpdateDown(float deltaTime);//ダウン中の処理

	// --- 衝突 ---
	std::unique_ptr<Capsule> capsule_;
	std::vector <std::weak_ptr<Collider>> colliders_;// 衝突判定に用いられるコライダ

	// 衝突チェック 衝突用線分
	VECTOR gravHitPosDown_;
	VECTOR gravHitPosUp_;

	// 衝突判定
	void Collision(void);
	void CollisionGravity(void);
	void CollisionCapsule(void);

	// 攻撃判定
	void CollisionAttack(void);
	void CollisionAttack2(void);
	void CollisionAttackEx(void);

	// --- モーション・操作 ---
	void ProcessMove(void);		// 移動
	void ProcessJump(void);		//ジャンプモーション
	void ProcessAttack(void);	//攻撃モーション

	// 回転
	void SetGoalRotate(double rotRad);
	void Rotate(void);

	// モーション終了
	bool IsEndLanding(void);		// ジャンプ終了
	bool IsEndLandingA(void);		// アタック終了
	bool IsExAttackReady() const;	// 回転斬りリセット

	// --- ステータス変化管理 ---
	void PowerUpTimer(void);//パワーアップの制限時間
	void SpeedUpTimer(void);//スピードアップの制限時間

	//復活処理
	void StartRevival();
	void Revival();

	// --- エフェクト ---

	Tree* tree_;

	// ジャンプ量
	VECTOR jumpPow_;
	
	// 丸影
	int imgShadow_;

	// 移動スピード
	float speed_;
	bool speedUpFlag_;	//スピードが上がったている間treu
	int speedUpCnt_;	//スピードアップの時間(20秒)

	// 移動方向
	VECTOR moveDir_;
	
	// 移動量
	VECTOR movePow_;
	
	// 移動後の座標
	VECTOR movedPos_;

	VECTOR collisionPos_;		//プレイヤーの当たり判定移動後座標
	float collisionRadius_;		// 衝突判定用の球体半径
	VECTOR collisionLocalPos_;	// 衝突判定用の球体中心の調整座標

	// 回転
	Quaternion playerRotY_;
	Quaternion goalQuaRot_;
	float stepRotTime_;

	// ジャンプ判定
	bool isJump_;
	
	// ジャンプの入力受付時間
	float stepJump_;
	
	//攻撃力
	int normalAttack_;
	int slashAttack_;
	int exrAttack_;

	//攻撃の判定
	bool powerUpFlag_;	//パワーが上がったている間treu
	bool isAttack_;		//縦斬り
	bool isAttack2_;	//横斬り
	bool exAttack_;		//回転斬り
	int exTimer_;		// クールタイム 10秒（ミリ秒）
	int lastExTime_;	// 最初から使えるようにする
	int powerUpCnt_;	//2パワーアップの時間(20秒)
	
	//ステ関連
	int hp_;
	int water_;

	// ステアイコン
	int imgPowerIcon_;			// パワー
	int imgSpeedIcon_;			// スピード
	int imgRotateAttackIcon_;	// 回転切り

	// 無敵状態
	bool invincible_;

	// 移動が可能かどうか
	bool canMove_;

	// 復活処理
	float revivalTimer_;

	// 足煙エフェクト
	int effectSmokeResId_;
	int effectSmokePleyId_;
	float stepFootSmoke_;

	// 回復エフェクト
	int effectHealResId_;
	int effectHealPleyId_;
	float stepHeal_;

	// フレームごとの移動値
	VECTOR moveDiff_;

	// ワープ準備開始時のプレイヤー情報
	Quaternion reserveStartQua_;
	VECTOR reserveStartPos_;

	// 更新ステップ
	void DrawShadow(void);

	// 描画系
	void DrawDebug(void);

	// 移動量の計算
	void CalcGravityPow(void);

	const std::vector<std::shared_ptr<EnemyBase>>* enemy_;

	// 足煙エフェクト
	void EffectFootSmoke(void);

	void EffectHeal(void);

	bool isMax_;//水の所持上限
};
