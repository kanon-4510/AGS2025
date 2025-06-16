#include "MiniMap.h"
#include "DxLib.h"
#include <algorithm> // std::clamp
#include <cmath>     // std::atan2, std::cos, std::sin

MiniMap::MiniMap(float worldSize, int screenSize, int mapPosX, int mapPosY)
    : mapPixelSize(screenSize), mapPosX(mapPosX), mapPosY(mapPosY)
{
    worldHalfSize = worldSize / 2.0f;
    scale = static_cast<float>(mapPixelSize) / worldSize;
}

void MiniMap::Draw(const MapVector2& playerPos, const std::vector<MapVector2>& enemies, 
    const std::vector<MapVector2>& items)
{
    DrawBackground();
    DrawEnemies(enemies);
    DrawItems(items);
    DrawPlayer(playerPos);
}

void MiniMap::DrawBackground()
{
    DrawBox(mapPosX, mapPosY,
        mapPosX + mapPixelSize, mapPosY + mapPixelSize,
        GetColor(100, 255, 0), TRUE);

    // === ミニマップの中心 ===
    int centerX = mapPosX + mapPixelSize / 2;
    int centerY = mapPosY + mapPixelSize / 2;

    //中心に木を描画(仮で黒点)
    DrawCircle(centerX, centerY, 2, GetColor(0, 0, 0), TRUE);
}

void MiniMap::DrawPlayer(const MapVector2& playerPos)
{
    // +worldHalfSize で中心原点を左上基準にずらす
    // * scale で画面上のスケールに変換
    // + offsetX / Y で画面上に配置
    int px = static_cast<int>((playerPos.x + worldHalfSize) * scale) + mapPosX;
    int pz = static_cast<int>((-playerPos.z + worldHalfSize) * scale) + mapPosY;

    // === ミニマップ内に収まるように制限（クランプ） ===
    int minX = mapPosX + 2;
    int maxX = mapPosX + mapPixelSize - 2;
    int minY = mapPosY + 2;
    int maxY = mapPosY + mapPixelSize - 2;

    px = std::clamp(px, minX, maxX);
    pz = std::clamp(pz, minY, maxY);

    // プレイヤー(青)
    DrawCircle(px, pz, 4, GetColor(0, 0, 255), TRUE);
}

void MiniMap::DrawEnemies(const std::vector<MapVector2>& enemies)
{
    for (const auto& e : enemies)
    {
        // +worldHalfSize で中心原点を左上基準にずらす
        // * scale で画面上のスケールに変換
        // + offsetX / Y で画面上に配置
        int ex = static_cast<int>((e.x + worldHalfSize) * scale) + mapPosX;
        int ez = static_cast<int>((-e.z + worldHalfSize) * scale) + mapPosY;

        // 敵(赤)
        DrawCircle(ex, ez, 3, GetColor(255, 0, 0), TRUE);
    }
}

void MiniMap::DrawItems(const std::vector<MapVector2>& items)
{
    for (const auto& item : items)
    {
        int ix = static_cast<int>((item.x + worldHalfSize) * scale) + mapPosX;
        int iz = static_cast<int>((-item.z + worldHalfSize) * scale) + mapPosY;


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

/*#include "MiniMap.h"

MiniMap::MiniMap(float mapRange, int size)
    : mapRange_(mapRange), size_(size), playerPos_(VGet(0, 0, 0))
{
    renderTargetGraph_ = MakeScreen(size_, size_, TRUE);
    SetUseZBuffer3D(TRUE);  // Zバッファ使う設定
    SetWriteZBuffer3D(TRUE); // 書き込み許可（必須）
}

MiniMap::~MiniMap()
{
    DeleteGraph(renderTargetGraph_);
}

void MiniMap::SetPlayerPosition(const VECTOR& pos)
{
    playerPos_ = pos;
}

void MiniMap::BeginRender()
{
    SetDrawScreen(renderTargetGraph_);
    

    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(TRUE);
    ClearDrawScreen();

    // 上空視点のカメラを設定
    VECTOR camPos = VGet(playerPos_.x, playerPos_.y + 300.0f, playerPos_.z);
    VECTOR camTarget = playerPos_;
    VECTOR camUp = VGet(0, 0, -1); // 真上視点

    SetupCamera_Perspective(60.0f); // 正射影にしたい場合は SetupCamera_Ortho()
    SetCameraPositionAndTarget_UpVecY(camPos, camTarget);
}

void MiniMap::EndRender()
{
    SetDrawScreen(DX_SCREEN_BACK); // 戻す
}

void MiniMap::Draw(int x, int y)
{
    DrawGraph(x, y, renderTargetGraph_, FALSE);
}*/