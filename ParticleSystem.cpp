#include "ParticleSystem.h"
#include "Game.h"

void ParticleSystem::init(SDL_Renderer* renderer_, int max_particles_) {
	renderer = renderer_, max_particles = max_particles_;
	particles.reserve(max_particles);
}

void ParticleSystem::render() {
	SDL_Rect rect;
	for (const auto& p : particles) {
		rect.x = static_cast<int>(p.position.x);
		rect.y = static_cast<int>(p.position.y);
		rect.w = rect.h = static_cast<int>(p.size);
		SDL_SetRenderDrawColor(renderer, p.color.r, p.color.g, p.color.b, static_cast<Uint8>(255 * p.lifetime));
		SDL_RenderFillRect(renderer, &rect);
	}
}

void ParticleSystem::update(float delta_time) {
	for (auto it = particles.begin(); it != particles.end();) {
		it->lifetime -= delta_time / it->max_lifetime;
		if (it->lifetime <= 0.0f) {
			it = particles.erase(it);
		}
		else {
			it->position.x += it->velocity.x * delta_time;
			it->position.y += it->velocity.y * delta_time;
			it->velocity.x *= 0.98f;
			it->velocity.y *= 0.98f;
			++it;
		}
	}
}

void ParticleSystem::burst(int x, int y, const std::vector<SDL_Color>& colors) {
	std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * M_PI);
	std::uniform_real_distribution<float> speed_dist(200.0f, 500.0f);
	std::uniform_real_distribution<float> lifetime_dist(0.8f, 1.5f);
	std::uniform_real_distribution<float> size_dist(2.0f, 8.0f);
	std::uniform_int_distribution<int> color_dist(0, static_cast<int>(colors.size()) - 1);
	for (int i = 0; i < max_particles / 2; ++i) {
		if (particles.size() >= max_particles) break;
		Particle p;
		p.position = { static_cast<float>(x), static_cast<float>(y) };
		float angle = angle_dist(gen);
		float speed = speed_dist(gen);
		p.velocity = { speed * cosf(angle), speed * sinf(angle) };
		p.color = colors[color_dist(gen)];
		p.max_lifetime = lifetime_dist(gen);
		p.lifetime = 1.0f;
		p.size = size_dist(gen);
		particles.push_back(p);
	}
}
