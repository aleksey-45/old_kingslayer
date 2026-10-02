#pragma once
#include <SDL.h>
#include <vector>
#include <cmath>
#include <random>

class ParticleSystem {
public:
    struct Particle {
        SDL_FPoint position;  // 当前位置
        SDL_FPoint velocity;  // 移动速度
        SDL_Color color;      // 粒子颜色
        float lifetime;       // 剩余生命周期（0.0-1.0）
        float max_lifetime;   // 最大生命周期（秒）
        float size;           // 粒子尺寸
    };

    void init(SDL_Renderer* renderer_, int max_particles_ = 1000);

    // 触发粒子爆发
    void burst(int x, int y, const std::vector<SDL_Color>& colors);

    // 更新粒子状态
    void update(float delta_time);

    // 渲染粒子
    void render();

private:
    SDL_Renderer* renderer;
    std::vector<Particle> particles;
    int max_particles;
    std::mt19937 gen{ std::random_device{}() }; // 随机数生成器
};