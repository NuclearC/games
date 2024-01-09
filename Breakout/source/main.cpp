#include <SDL3/SDL.h>

#include <cmath>
#include <numbers>
#include <vector>

#include <string>

SDL_Window* window;
SDL_Renderer* renderer;

constexpr int WINDOW_WIDTH = 640;
constexpr int WINDOW_HEIGHT = 480;

SDL_FRect player_rect{};

SDL_FPoint ball{};
SDL_FPoint new_pos{};
SDL_FPoint ball_velocity{};

float ball_speed = 0.f;
float ball_radius = 5.0f;

float brick_width = 64.0f;
float brick_height = 20.0f;

std::vector<std::string> level_map = {
	  "####",
	 "MX  XM",
	"XX XX XX",
	 "M#  #M",
	  "#XX#",
};

float Dot(const SDL_FPoint& p1, const SDL_FPoint& p2) {
	return std::sqrt(p1.x * p2.x + p1.y * p2.y);
}

float Length(const SDL_FPoint& fp) {
	return std::sqrt(fp.x * fp.x + fp.y * fp.y);
}

bool CheckCollision(const SDL_FRect& r1, const SDL_FRect& r2) {
	if (r1.x < r2.x + r2.w &&
		r1.x + r1.w > r2.x &&
		r1.y < r2.y + r2.h &&
		r1.y + r1.h > r2.y)
	{
		return true;
	}

	return false;
}

void SetBallDirection(float angle) {
	ball_velocity.x = std::cos(angle);
	ball_velocity.y = std::sin(angle);
}

bool HandleEvent(SDL_Event& e) {
	if (e.type == SDL_EVENT_QUIT) {
		return false;
	}

	if (e.type == SDL_EVENT_MOUSE_MOTION) {
		player_rect.x = std::max(0.f, std::min(player_rect.x + e.motion.xrel, 640.0f - player_rect.w));
	}
	else if (e.type == SDL_EVENT_KEY_DOWN) {
		if (e.key.keysym.scancode == SDL_Scancode::SDL_SCANCODE_ESCAPE) {
			return false;
		}
	}

	return true;
}

void CollidePlayer() {

	SDL_FRect ball_rect{ ball.x - ball_radius, ball.y - ball_radius, 2.0f * ball_radius, 2.0f * ball_radius };

	bool is_colliding = CheckCollision(ball_rect, player_rect);
	
	if (is_colliding){
		if (ball.y >= player_rect.y && ball.y <= player_rect.y + player_rect.h) {
			ball_velocity.x *= -1.0f;
			new_pos.x = ball.x + ball_velocity.x * ball_speed;
		}else  {

			float xd = ball.x - (player_rect.x + player_rect.w / 2.0f);

			xd /= player_rect.w / 2.0f;
			xd = std::max(-1.0f, std::min(1.0f, xd));

			float ang = std::acos(xd);

			SetBallDirection(-ang);
			// ball_velocity.y *= -1.0f;
			new_pos.y = ball.y + ball_velocity.y * ball_speed;
			new_pos.x = ball.x + ball_velocity.x * ball_speed;
		}
	}
}

void CollideBrick(const SDL_FRect& brick, char& ch) {

	SDL_FRect ball_rect{ ball.x - ball_radius, ball.y - ball_radius, 2.0f * ball_radius, 2.0f * ball_radius };

	bool is_colliding = CheckCollision(ball_rect, brick);
	if (is_colliding) {
		if (ball.y >= brick.y && ball.y <= brick.y + brick.h) {
			ball_velocity.x *= -1.0f;
			new_pos.x = ball.x + ball_velocity.x * ball_speed;
		}
		else {
			ball_velocity.y *= -1.0f;
			new_pos.y = ball.y + ball_velocity.y * ball_speed;
		}

		ch = ' ';
	}
}

void CollideBricks() {
	for (int r = 0; r < level_map.size(); r++) {
		auto& row = level_map[r];
		if (row.size() & 1)
			throw std::exception();

		float row_y = 50.0f + static_cast<float>(r) * brick_height;
		float row_x = static_cast<float>(WINDOW_WIDTH / 2) - static_cast<float>(row.size() / 2) * brick_width;

		for (int c = 0; c < row.size(); c++) {
			if (row[c] == ' ') continue;

			SDL_FRect brick_rect{ row_x + static_cast<float>(c) * brick_width, row_y, brick_width, brick_height };

			CollideBrick(brick_rect, row[c]);
		}
	}
}

void Simulate() {
	new_pos = SDL_FPoint{ ball.x + ball_velocity.x * ball_speed ,ball.y + ball_velocity.y * ball_speed};

	if (new_pos.x - ball_radius < 0 || new_pos.x + ball_radius > (float)WINDOW_WIDTH) {
		ball_velocity.x *= -1.0f;
		new_pos.x = ball.x + ball_velocity.x * ball_speed;
	} else if (new_pos.y - ball_radius < 0) {
		ball_velocity.y *= -1.0f;
		new_pos.y = ball.y + ball_velocity.y * ball_speed;
	}
	else if (new_pos.y + ball_radius > (float)WINDOW_HEIGHT) {
		throw std::exception();
	}

	CollidePlayer();
	CollideBricks();

	ball = new_pos;
}

void Draw() {
	SDL_SetRenderDrawColor(renderer, 100, 50, 0, 255);
	SDL_RenderFillRect(renderer, &player_rect);

	std::vector<SDL_Vertex> ball_vertices;
	constexpr float step = 0.5f;
	for (float angle = 0.f; angle < std::numbers::pi * 2.0f; angle += step) {
		SDL_Vertex v;
		v.position = { ball.x + std::cos(angle) * ball_radius,
			ball.y + std::sin(angle) * ball_radius };
		v.color = { 25,150,25 };
		v.tex_coord = { 0,0 };
		ball_vertices.push_back(v);
		v.position = { ball.x + std::cos(angle + step) * ball_radius,
			ball.y + std::sin(angle + step) * ball_radius };
		ball_vertices.push_back(v);
		v.position = { ball.x, ball.y };
		ball_vertices.push_back(v);

	}

	SDL_RenderGeometry(renderer, nullptr, ball_vertices.data(), ball_vertices.size(), nullptr, 0);

	for (int r = 0; r < level_map.size(); r++) {
		const auto& row = level_map[r];
		if (row.size() & 1)
			throw std::exception();

		float row_y = 50.0f + static_cast<float>(r) * brick_height;
		float row_x = static_cast<float>(WINDOW_WIDTH / 2) - static_cast<float>(row.size() / 2) * brick_width;

		for (int c = 0; c < row.size(); c++) {
			if (row[c] == ' ') continue;

			SDL_FRect brick_rect{ row_x + static_cast<float>(c) * brick_width, row_y, brick_width, brick_height };

			switch (row[c]) {
			case '#':
			{
				SDL_SetRenderDrawColor(renderer, 120 + r * 10, 0, 0, 255);
				SDL_RenderFillRect(renderer, &brick_rect);
				SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
				SDL_RenderRect(renderer, &brick_rect);

			} break;
			case 'M': {
				SDL_SetRenderDrawColor(renderer, 0, 120 + r * 10, 0, 255);
				SDL_RenderFillRect(renderer, &brick_rect);
				SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
				SDL_RenderRect(renderer, &brick_rect);
			} break;
			case 'X': {
				SDL_SetRenderDrawColor(renderer, 0,0, 120 + r * 10, 255);
				SDL_RenderFillRect(renderer, &brick_rect);
				SDL_SetRenderDrawColor(renderer, 0, 0, 200, 255);
				SDL_RenderRect(renderer, &brick_rect);
			} break;
			}
		}
	}
}

int main()
{
	ball_speed = 2.5f;
	SetBallDirection(0.64f);

	player_rect.w = 80.0f;
	player_rect.h = 15.0f;

	ball.x = WINDOW_WIDTH / 2;
	ball.y = WINDOW_HEIGHT / 2;

	player_rect.x = WINDOW_WIDTH/2.0f - player_rect.w / 2.0f;
	player_rect.y = 400.0f;

	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow("Breakout", WINDOW_WIDTH, WINDOW_HEIGHT, {});
	renderer = SDL_CreateRenderer(window, nullptr, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_CaptureMouse(SDL_TRUE);

	SDL_Event e{};

	bool should_close = false;
	do {
		while (SDL_PollEvent(&e)) {
			should_close = !HandleEvent(e) || should_close;
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		Simulate();
		Draw();

		SDL_RenderPresent(renderer);

	} while (!should_close);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
