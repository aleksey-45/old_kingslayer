#pragma once
#include <SDL.h>
#include <cmath>
#define SCANLINE_SPACING 2  // 扫描线间隔（每2像素一条线）
#define SCANLINE_ALPHA 80   // 扫描线透明度（0-255）

static SDL_Texture* create_scanline_texture(SDL_Renderer* renderer, int window_height) {
    // 创建覆盖整个窗口高度的1xN纹理
    int texture_height = window_height;
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, 1, texture_height, 32, SDL_PIXELFORMAT_RGBA32);

    Uint32* pixels = (Uint32*)surface->pixels;
    for (int y = 0; y < texture_height; y++) {
        // 每隔SCANLINE_SPACING行绘制一条半透明黑线
        if (y % SCANLINE_SPACING == 0) {
            pixels[y] = SDL_MapRGBA(surface->format, 0, 0, 0, SCANLINE_ALPHA);
        }
        else {
            pixels[y] = SDL_MapRGBA(surface->format, 0, 0, 0, 0); // 透明行
        }
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    return texture;
}

// 在窗口大小变化时更新扫描线纹理
static void handle_resize(SDL_Renderer* renderer, SDL_Texture** scanline, int new_height) {
    if (*scanline) SDL_DestroyTexture(*scanline);
    *scanline = create_scanline_texture(renderer, new_height);
}
