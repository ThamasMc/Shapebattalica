#pragma once

#include "Entity.h"
#include "EntityManager.h"
#include "Action.h"
#include "Game.h"

class Game; // Resolving circular dependency

class Scene
{
protected:
	Game* game;
	EntityManager m_entities; // vector of entities we maintain
	bool m_paused = false;

public:
	Scene();
	// Scene(Game* engine); // constructor, takes in the game config
	void setPaused(bool paused);

	virtual void update() = 0;
	virtual void setTarget(const Vec2& target) = 0;
	virtual void sDoAction(const Action& action) = 0;
	virtual void sRender() = 0;
};
