#pragma once

#include "Entity.h"
#include "EntityManager.h"
#include "Game.h"
#include <SFML/Graphics.hpp>

class Game; // Resolving circular dependency

struct PlayerConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S; };
struct EnemyConfig { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI; float SMIN, SMAX; }; 
struct BulletConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S; };

class Scene_Play
{
	Game* game;
	EntityManager m_entities; // vector of entities we maintain
	PlayerConfig m_playerConfig;
	EnemyConfig m_enemyConfig;
	BulletConfig m_bulletConfig;
	int m_score = 0;
	int m_currentFrame = 0;
	int m_lastEnemySpawnTime = 0;
	int m_lastSpecialShot = 0;
	bool m_paused = false;
	bool m_running = true;

	std::shared_ptr<Entity> m_player;

	void init(const std::string& config); // Initialize the GameState with a config file
	void setPaused(bool paused);

	void sMovement();
	void sUserInput();
	void sLifespan();
	void sRender();
	void sEnemySpawner();
	void sCollision();
	
	void spawnPlayer();
	void spawnEnemy();
	void spawnSmallEnemies(std::shared_ptr<Entity> entity);
	void spawnBullet(std::shared_ptr<Entity> entity, const Vec2& mousePos);
	void spawnSpecialWeapon(std::shared_ptr<Entity> entity, const Vec2& mousePos);

	bool goingOutOfBounds(std::shared_ptr<Entity> entity);
	Vec2 outOfBoundsVec(std::shared_ptr<Entity> entity);
	int randInRange(int min, int max);

public:
	Scene_Play(const std::string& config, Game* engine); // constructor, takes in the game config

	void update();
	const bool running() const;
};
