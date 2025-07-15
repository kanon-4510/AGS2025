#include "MiniMap.h"
#include "DxLib.h"
#include "../Manager/ResourceManager.h"
#include "../Object/EnemyBase.h"
#include <algorithm> // std::clamp
#include <cmath>     // std::atan2, std::cos, std::sin

MiniMap::MiniMap(float worldSize, int screenSize, int mapPosX, int mapPosY)
    : mapPixelSize(screenSize), mapPosX(mapPosX), mapPosY(mapPosY)
{
    worldHalfSize = worldSize / 2.0f;
    scale = static_cast<float>(mapPixelSize) / worldSize;
}

void MiniMap::Init(void)
{
    // 画像読み込み
    imgMapTree_ = LoadGraph("Data/Image/MapTree.png");
}

void MiniMap::Draw(const MapVector2& playerPos, float playerAngleRad,
    float cameraAngleRad,
    const std::vector<std::shared_ptr<EnemyBase>>& enemies,
    const std::vector<MapVector2>& items)
{
    DrawBackground();
    DrawEnemies(enemies);
    DrawItems(items);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);  //透過ON
    DrawCameraViewCone(playerPos, cameraAngleRad);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);    //透過OFF
    DrawPlayer(playerPos, playerAngleRad);
}

void MiniMap::DrawBackground()
{
    /*DrawBox(mapPosX, mapPosY,
        mapPosX + mapPixelSize, mapPosY + mapPixelSize,
        GetColor(100, 255, 0), TRUE);*/

    // === ミニマップの中心 ===
    int centerX = mapPosX + mapPixelSize / 2;
    int centerY = mapPosY + mapPixelSize / 2;
    int radius = mapPixelSize / 2;

    int outerRadius = mapPixelSize / 2;
    int borderWidth = 4; // 外枠の太さ
    int innerRadius = outerRadius - borderWidth;

    // --- 外側を緑で塗る(枠線を含む)
    DrawCircle(centerX, centerY, outerRadius, GetColor(0, 0, 0), TRUE);

    // --- 上から中身だけを「円でくり抜く」ことでリング状になる
    DrawCircle(centerX, centerY, innerRadius, GetColor(100, 255, 0), TRUE);

    // 画像
    DrawRotaGraph(centerX, centerY, 0.15f, 0.0f, imgMapTree_, true);

}

void MiniMap::DrawCameraViewCone(const MapVector2& playerPos, float cameraAngleRad)
{
    // ミニマップ座標に変換
    int px = static_cast<int>((playerPos.x + worldHalfSize) * scale) + mapPosX;
    int pz = static_cast<int>((-playerPos.z + worldHalfSize) * scale) + mapPosY;

    if (!IsInsideCircle(px, pz)) return;

    // カメラの方向（視界の中心方向）
    float angle = cameraAngleRad - DX_PI / 2.0f;

    // 視界の長さ（半径に相当）
    float viewLength = 30.0f;

    // 視界の開き角（左右にどれだけ開くか）
    float fovHalfAngle = 0.5f; // 約57度くらい（0.5ラジアン）

    // 左・右方向の端を計算（底辺の左右端）
    float leftX = px + std::cos(angle + fovHalfAngle) * viewLength;
    float leftY = pz + std::sin(angle + fovHalfAngle) * viewLength;

    float rightX = px + std::cos(angle - fovHalfAngle) * viewLength;
    float rightY = pz + std::sin(angle - fovHalfAngle) * viewLength;

    // 三角形を描画（カメラの視界：黄色など）
    DrawTriangleAA(
        px, pz,
        static_cast<int>(leftX), static_cast<int>(leftY),
        static_cast<int>(rightX), static_cast<int>(rightY),
        GetColor(255, 255, 255), TRUE); // 黄色で塗りつぶし
}

void MiniMap::DrawPlayer(const MapVector2& playerPos, float playerAngleRad)
{
    // +worldHalfSize で中心原点を左上基準にずらす
    // * scale で画面上のスケールに変換
    // + offsetX / Y で画面上に配置
    int px = static_cast<int>((playerPos.x + worldHalfSize) * scale) + mapPosX;
    int pz = static_cast<int>((-playerPos.z + worldHalfSize) * scale) + mapPosY;

    // === ミニマップ内に収まるように制限===
    if (!IsInsideCircle(px, pz)) return;

    // プレイヤー(青)
    //DrawCircle(px, pz, 4, GetColor(0, 0, 255), TRUE);

    // === 向きを示す三角形（矢印） ===
    // 向きの角度（ラジアン）
    float angle = playerAngleRad - DX_PI / 2.0f; // 適宜調整してね

    // 矢印のサイズ
    float size = 8.0f;

    // 先端（進行方向）
    float frontX = px + std::cos(angle) * size;
    float frontY = pz + std::sin(angle) * size;

    // 左右の角（扇形に広げた方向）
    float sideOffset = 2.5f; // 三角の広がり角度
    float leftX = px + std::cos(angle + sideOffset) * size;
    float leftY = pz + std::sin(angle + sideOffset) * size;
    float rightX = px + std::cos(angle - sideOffset) * size;
    float rightY = pz + std::sin(angle - sideOffset) * size;

    // 三角形を描画
    DrawTriangle(
        static_cast<int>(frontX), static_cast<int>(frontY),
        static_cast<int>(leftX), static_cast<int>(leftY),
        static_cast<int>(rightX), static_cast<int>(rightY),
        GetColor(0, 0, 255), TRUE);
}

void MiniMap::DrawEnemies(const std::vector<std::shared_ptr<EnemyBase>>& enemies)
{
    for (const auto& enemy : enemies)
    {
        auto pos = enemy->GetCollisionPos();  // 仮に座標取得関数があるとする

        // +worldHalfSize で中心原点を左上基準にずらす
        // * scale で画面上のスケールに変換
        // + offsetX / Y で画面上に配置
        int ex = static_cast<int>((pos.x + worldHalfSize) * scale) + mapPosX;
        int ez = static_cast<int>((-pos.z + worldHalfSize) * scale) + mapPosY;

        // 通常の円の半径
        int radius = 3;  

        // BOSSのときは円を大きく
        if (enemy->GetEnemyType() == EnemyBase::TYPE::BOSS)
        {
            radius = 9;
        }

        // 敵(赤)
        DrawCircle(ex, ez, radius, GetColor(255, 0, 0), TRUE);
    }
}

void MiniMap::DrawItems(const std::vector<MapVector2>& items)
{
    for (const auto& item : items)
    {
        int ix = static_cast<int>((item.x + worldHalfSize) * scale) + mapPosX;
        int iz = static_cast<int>((-item.z + worldHalfSize) * scale) + mapPosY;

        // === ミニマップ内に収まるように制限===
        if (!IsInsideCircle(ix, iz)) return;

        size = 5;
        // 頂点座標
        topX = ix;
        topY = iz - size;
        rightX = ix + size;
        rightY = iz;
        bottomX = ix;
        bottomY = iz + size;
        leftX = ix - size;
        leftY = iz;
        int color = GetColor(0, 100, 255);

        // 三角形を2つ使って塗りつぶす
        DrawTriangle(topX, topY, rightX, rightY, bottomX, bottomY, color, TRUE);
        DrawTriangle(bottomX, bottomY, leftX, leftY, topX, topY, color, TRUE);

        // 水(水色)
        //DrawBox(ix - 3, iz - 3, ix + 3, iz + 3, GetColor(0, 120, 255), TRUE);
    }
}

bool MiniMap::IsInsideCircle(int x, int y) const
{
    int centerX = mapPosX + mapPixelSize / 2;
    int centerY = mapPosY + mapPixelSize / 2;
    int radius = mapPixelSize / 2;

    int dx = x - centerX;
    int dy = y - centerY;

    return dx * dx + dy * dy <= radius * radius;
}