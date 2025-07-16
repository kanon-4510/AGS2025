#pragma once
#include <memory>
#include <array>
#include<map>
#include <functional>
#include <vector>
#include "ActorBase.h"
#include "Item.h" 

class AnimationController;
class GameScene;
//class Collider;
//class Capsule;
class Player;
class Tree;

class EnemyBase : public ActorBase
{
public:
	static constexpr float VIEW_RANGE = 500.0f;	// 視野の広さ
	static constexpr float VIEW_ANGLE = 15.0f;	// 視野角

	//敵の種類
	enum class TYPE
	{
		DOG,	//犬
		SABO,	//サボテン
		MIMIC,	//ミミック
		MUSH,	//キノコ
		ONION,	//玉ねぎ
		TOGE,	//トゲゾー
		VIRUS,	//ウィルス
		BOSS,	//ボス
		MAX
	};

	//状態管理
	enum class STATE
	{
		NONE,
		IDLE,
		PLAY,
		ATTACK,
		DAMAGE,
		DEATH,
		MAX
	};

	// アニメーション種別
	enum class ANIM_TYPE
	{
		NONE,
		IDLE,
		RUN,
		ATTACK,
		DAMAGE,
		DEATH,
		MAX
	};

	EnemyBase();	// コンストラクタ
	virtual ~EnemyBase(void);	// デストラクタ

	virtual void Init(void);			// 初期処理(最初の１回のみ実行)
	virtual void InitAnimation(void) {};//アニメーションロード用
	virtual void SetParam(void){};		// パラメータ設定(純粋仮想関数)
	virtual void Update(void);			 // 更新処理(毎フレーム実行)
	virtual void Draw(void);			 // 描画処理(毎フレーム実行)
	virtual void Release(void);			 // 解放処理(最後の１回のみ実行)

	void SetPos(VECTOR pos);	// 座標の設定
	STATE GetState(void);		// 状態獲得

	bool IsAlive(void);			// 生存判定
	void SetAlive(bool alive);	// 生存判定

	void Damage(int damage);	// ダメージを受ける

	TYPE GetEnemyType(void) const;
	TYPE enemyType_;

	// 死亡時のドロップアイテムを決める関数（デフォルトはNONE）
	virtual Item::TYPE GetDropItemType() const;
	
	void SetCollisionPos(const VECTOR collision);//衝突判定用の球体
	VECTOR GetCollisionPos(void)const;	// 衝突用の中心座標の取得
	float GetCollisionRadius(void);		// 衝突用の球体半径の取得

	void SetGameScene(GameScene* scene);

	void DrawDebug(void);	//デバッグ用
	void DrawDebugSearchRange(void);

	void SetPlayer(std::shared_ptr<Player> player);
	void SetTree(std::shared_ptr<Tree> tree);
protected:

	std::shared_ptr<Player> player_;
	std::shared_ptr<Tree> tree_;
	std::shared_ptr<Item>item_;
	GameScene* scene_;

	// アニメーション
	std::unique_ptr<AnimationController> animationController_;
	
	std::map<STATE, std::function<void(void)>> stateChanges_;// 状態管理(状態遷移時初期処理)
	std::function<void(void)> stateUpdate_;					 // 状態管理(更新ステップ)
	
	float speed_;	// 移動速度
	
	VECTOR movePow_;	// 移動量
	VECTOR movedPos_;	// 移動後の座標
	VECTOR moveDiff_;	// フレームごとの移動値

	VECTOR collisionPos_;	//赤い球体の移動後座標
	VECTOR attackCollisionPos_; //紫の球体の移動後座標

	int hp_;	// 体力
	int attackPow_; //攻撃力

	bool isAlive_;	// 生存判定
	bool isAttack_ = false;		//攻撃判定
	bool isAttack_P = false;	//攻撃判定
	bool isAttack_T = false;	//攻撃判定

	//ダメージに関する変数と定数
	COLOR_F originalColor_;  // デフォルトは白
	COLOR_F blinkColor_;     // 点滅中は赤
	bool isDamageBlinking_ = false;
	int damageBlinkTimer_ = 0;	//点滅する時間
	int blinkInterval_ = 6;		// 6フレームごとに色切替

	STATE state_;	//状態管理

	float speedAnim_; // 再生速度（共通）

	float collisionRadius_;		// 衝突判定用の球体半径
	VECTOR collisionLocalPos_;	// 衝突判定用の球体中心の調整座標

	float attackCollisionRadius_;	 // 攻撃判定用と攻撃範囲の球体半径
	VECTOR attackCollisionLocalPos_; // 攻撃判定用と攻撃範囲の調整座標

	VECTOR playerCenter_;	//プレイヤーの球体の座標を取得
	float playerRadius_;	//プレイヤーの球体の半径を取得
	VECTOR p_Diff_;			//プレイヤーの位置差分
	float p_Dis_;			//プレイヤーまでの距離
	float p_RadiusSum_;		//プレイヤーとの衝突半径の合計

	VECTOR treeCenter_;		//木の球体の座標を取得
	float treeRadius_;		//木の球体の半径を取得
	VECTOR t_Diff_;			//木の位置差分
	float t_Dis_;			//木までの距離
	float t_RadiusSum_;		//木との衝突半径の合計

	void UpdateNone(void){};		// 更新ステップ
	virtual void UpdateIdle(void);	// 更新ステップ
	virtual void UpdatePlay(void);	// 更新処理(毎フレーム実行)
	virtual void UpdateAttack(void);// 更新処理(毎フレーム実行)
	virtual void UpdateDamage(void);// 更新処理(毎フレーム実行)
	virtual void UpdateDeath(void);	// 死んだ歳の更新処理
	
	void ChasePlayer(void);			//プレイヤーを追いかける

	//攻撃関係
	void AttackCollisionPos(void);	//攻撃用関数
	void EnemyToPlayer(void);		//敵がプレイヤー攻撃
	void EnemyToTree(void);			//敵が木を攻撃
	void CheckHitAttackHit(void);	//攻撃が当たったか確認する

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateNone(void);
	void ChangeStateIdle(void);
	void ChangeStatePlay(void);
	void ChangeStateAttack(void);
	void ChangeStateDamage(void);
	void ChangeStateDeath(void);

	void Collision(void);	// 衝突判定
};