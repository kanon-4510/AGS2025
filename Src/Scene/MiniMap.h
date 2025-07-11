#pragma once
#include <memory>
#include <vector>

class EnemyBase;

struct MapVector2
{
    float x, z;
};

class MiniMap
{
public:
    MiniMap(float worldSize, int screenSize, int mapPosX = 1600, int mapPosY = 50);

    void Init(void);

    void Draw(const MapVector2& playerPos, float playerAngleRad, 
        const std::vector<std::shared_ptr<EnemyBase>>& enemies,
        const std::vector<MapVector2>& items);

private:
    float worldHalfSize;      // ゲーム空間の広さ（片側）
    int mapPixelSize;         // ミニマップの表示サイズ（ピクセル）
    int mapPosX, mapPosY;   // ミニマップの画面上の描画位置（左上）
    float scale;              // ワールド座標からミニマップ座標へのスケーリング係数

    int size;
    // 頂点座標
    int topX;
    int topY;
    int rightX;
    int rightY;
    int bottomX;
    int bottomY;
    int leftX;
    int leftY;

    // 画像
    int imgMapTree_;

    //Map背景描画
    void DrawBackground();
    //プレイヤーの位置をミニマップ上に変換して描画
    void DrawPlayer(const MapVector2& playerPos, float playerAngleRad);
    //敵をリストで受け取り、ミニマップ上に変換して描画
    void DrawEnemies(const std::vector<std::shared_ptr<EnemyBase>>& enemies);
    //アイテムをリストで受け取り、ミニマップ上に変換して描画
    void DrawItems(const std::vector<MapVector2>& items);

    // 円の範囲内に制限
    bool IsInsideCircle(int x, int y) const;
};
