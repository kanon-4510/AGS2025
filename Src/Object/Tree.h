#include"../Common/Vector2.h"

class GameScene;
class SceneManager;
class Player;

class Tree
{
public:
	//色
	int white = 0xffffff; //白
	int black = 0x000000; //黒
	int red = 0xff0000;	  //赤
	int green = 0x00ff00; //緑
	int blue = 0x0000ff;  //青
	int yellow = 0xffff00;//黄
	int purpl = 0x800080; //紫
	int gray = 0xaaaaaa;  //灰
	int orange = 0xffdead;//薄黄色

	static constexpr float ZERO = 0.0f;				//初期化の値用
	static constexpr int GROW_UP = 4;				//成長段階
	static constexpr int INVINCIBLE_TIME = 600;		//無敵

	//ステータス関連
	static constexpr int HP_MAX = 50;				//木の最大HP
	static constexpr int HIT_POINT = 12;			//木の現在HP
	static constexpr int LV_BABY = 1;				//木の成長度(苗木)
	static constexpr int LV_KID = 25;				//木の成長度(幼木)
	static constexpr int LV_ADULT = 50;				//木の成長度(成木)
	static constexpr int LV_OLD = 75;				//木の成長度(老木)
	static constexpr int HOLD_WATER_BABY = 1;		//水の所持量(苗木)
	static constexpr int HOLD_WATER_KID = 2;		//水の所持量(幼木)
	static constexpr int HOLD_WATER_ADULT = 2;		//水の所持量(成木)
	static constexpr int HOLD_WATER_OLD = 3;		//水の所持量(老木)

	static constexpr int DOUBLE_WATER = 19;			//水の２倍獲得

	//苗木
	static constexpr VECTOR BABY_SCL = { 3.0f, 2.5f, 3.0f };
	static constexpr VECTOR BABY_ROT = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };
	static constexpr VECTOR BABY_POS = { 0.0f, -3.5f, 0.0f };
	//幼木
	static constexpr VECTOR KID_SCL = { 15.0f, 10.0f, 15.0 };
	static constexpr VECTOR KID_ROT = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };
	static constexpr VECTOR KID_POS = { 0.0f, -2.0f, 0.0f };
	//成木
	static constexpr VECTOR ADULT_SCL = { 30.0f, 25.0f, 30.0f };
	static constexpr VECTOR ADULT_ROT = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };
	static constexpr VECTOR ADULT_POS = { 0.0f, -2.5f, 0.0f };
	//老木
	static constexpr VECTOR OLD_SCL = { 45.0f, 45.0f, 45.0f };
	static constexpr VECTOR OLD_ROT = { 0.0f, 0.0f * DX_PI_F / 180.0f, 0.0f };
	static constexpr VECTOR OLD_POS = { 0.0f, -23.5f, 0.0f };

	//あたり判定
	static constexpr float COL_RAD_BABY = 100.0f;
	static constexpr float COL_RAD_KID = 200.0f;
	static constexpr float COL_RAD_ADULT = 350.0f;
	static constexpr float COL_RAD_OLD = 800.0f;
	static constexpr VECTOR COL_LOCAL_POS = { 0.0f, 60.0f, 0.0f };

	//距離
	static constexpr float DISTANCE_BABY = 70.0f;
	static constexpr float DISTANCE_KID = 100.0f;
	static constexpr float DISTANCE_ADULT = 350.0f;
	static constexpr float DISTANCE_OLD = 700.0f;

	static constexpr int CX = 300;
	static constexpr int CY = 965;
	static constexpr float RADIUS = 32.0f;
	static constexpr int SEGMENTS = 60;
	static constexpr float GUARD_ICON_SIZE = 1.3f;

	//範囲
	static constexpr float RANGE_KID = 200.0f;
	static constexpr float RANGE_ADULT = 450.0f;
	static constexpr float RANGE_OLD = 909.0f;

	static constexpr int ALPHA_GRAY = 180;//影

	//大きさ
	static constexpr float DEFAULT_SCL = 10.0f;
	static constexpr float SCL_TREE_BABY = 50.0f;
	static constexpr float SCL_TREE_KID = 105.0f;
	static constexpr float SCL_TREE_ADULT = 235.0f;
	static constexpr float SCL_TREE_OLD = 480.0f;

	//ステータスバー(画面左下)
	static constexpr int NAME_X = 55;									//名前の位置X
	static constexpr int NAME_Y = Application::SCREEN_SIZE_Y - 220;		//名前の位置Y
	static constexpr int FRAME_START_X = 47;							//枠の最初X
	static constexpr int FRAME_START_Y = Application::SCREEN_SIZE_Y-203;//枠の最初Y
	static constexpr int FRAME_END_X = 653;								//枠の最後X
	static constexpr int FRAME_END_Y = Application::SCREEN_SIZE_Y-162;	//枠の最後Y
	static constexpr int BAR_START_X = 50;								//バーの最初X
	static constexpr int BAR_START_HY = Application::SCREEN_SIZE_Y-200;	//バーの最初体力Y
	static constexpr int BAR_START_WY = Application::SCREEN_SIZE_Y-175;	//バーの最初水Y
	static constexpr int BAR_END_X = 650;								//バーの最後X
	static constexpr int BAR_END_HY = Application::SCREEN_SIZE_Y-180;	//バーの最後体力Y
	static constexpr int BAR_END_WY = Application::SCREEN_SIZE_Y-165;	//バーの最後水Y
	static constexpr int WATER_BAR_BABY = 600;							//木の水分量(苗木)
	static constexpr int WATER_BAR_KID = 300;							//木の水分量(幼木)
	static constexpr int WATER_BAR_ADULT = 300;							//木の水分量(成木)
	static constexpr int WATER_BAR_OLD = 200;							//木の水分量(老木)

	//ヘッダーかファイル冒頭で定数定義（必要に応じて static const や constexpr にしてもOK）
	static constexpr float TREE_GROW_RADIUS_INCREMENT = 10.0f;
	static constexpr float ENEMY_SAFE_MARGIN = 30.0f;
	static constexpr float MIN_DISTANCE_THRESHOLD = 0.001f;

	//成長度合い
	enum class GROW
	{
		BABY,
		KID,
		ADULT,
		OLD,
		MAX
	};

	Tree(void);							//コンストラクタ
	~Tree(void);						//デストラクタ

	bool Init(void);					//初期化処理
	void Update(void);					//更新処理
	void Draw(void);					//描画処理
	void DrawDebug(void);				//デバッグ用
	void DrawDebugTree2Player(void);	//対プレイヤー

	int GetHp(void);//体力
	int GetLv(void);//レベル

	void LvUp(void);		//レベルを上げる
	void ChangeGrow(void);	//成長させる

	void SetPlayer(Player* player);		//プレイヤーのポインタをセットする関数

	VECTOR GetCollisionPos(void)const;	//衝突用の中心座標の取得
	float GetCollisionRadius(void);		//衝突用の球体半径の取得
	float collisionRadius_;				//衝突判定用の球体半径
	VECTOR collisionLocalPos_;			//衝突判定用の球体中心の調整座標
	VECTOR collisionPos_;				//衝突判定の座標		
	
	void Guard(void);		//無敵化
	void EnemyHit(void);	//エネミー達との判定
	void PlayerHit(void);	//プレイヤーとの判定
private:
	float viewRange_ = 100.0f;	//円の半径（任意の値）
	int circleSegments_ = 120;  //円の分割数

	Player* player_;
	GameScene* gameScene_;
	SceneManager* sceneManager_;

	int modelIdB_;	//モデルの格納(苗木)
	int modelIdK_;	//モデルの格納(幼木)
	int modelIdA_;	//モデルの格納(成木)
	int modelIdO_;	//モデルの格納(老木)
	VECTOR scl_;	//大小
	VECTOR pos_;	//位置
	VECTOR rot_;	//回転
	VECTOR dir_;	//移動しない

	int imgGuardIcon_;	//無敵アイコン

	//ステータス
	int lv_;
	int hp_;
	int water_;
	bool isAlive_;
	bool isD_;
	GROW grow_;

	//無敵状態
	bool guard_;
	int guardCnt_;

	//エフェクト
	int effectTreeResId_;
	int effectTreePlayId_;

	void PushEnemy(void);		//木にめり込んだら押し出す
	void GuardTimer(void);		//無敵時間
	void EffectTreeRange(void);	//木の視野範囲
};