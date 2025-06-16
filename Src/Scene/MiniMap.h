#pragma once
#include <vector>

struct MapVector2
{
    float x, z;
};

class MiniMap
{
public:
    MiniMap(float worldSize, int screenSize, int mapPosX = 1600, int mapPosY = 50);

    void Draw(const MapVector2& playerPos, const std::vector<MapVector2>& enemies, 
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

    //Map背景描画
    void DrawBackground();
    //プレイヤーの位置をミニマップ上に変換して描画
    void DrawPlayer(const MapVector2& playerPos);
    //敵をリストで受け取り、ミニマップ上に変換して描画
    void DrawEnemies(const std::vector<MapVector2>& enemies);
    //アイテムをリストで受け取り、ミニマップ上に変換して描画
    void DrawItems(const std::vector<MapVector2>& items);

};


/*#ifndef MINIMAP_H
#define MINIMAP_H

#include <DxLib.h>
#include <memory>
#include <vector> // VECTOR使用

class MiniMap {
public:
    MiniMap(float mapRange = 16000.0f, int size = 256);
    ~MiniMap();

    void SetPlayerPosition(const VECTOR& pos);
    void BeginRender();
    void EndRender();
    void Draw(int x, int y);

private:
    float mapRange_;
    int size_;
    int renderTargetGraph_;
    VECTOR playerPos_;
};

#endif*/