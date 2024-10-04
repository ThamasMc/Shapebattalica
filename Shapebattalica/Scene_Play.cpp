#include "Scene_Play.h"
#include <iostream>
#include <fstream>
#include <math.h>

Scene_Play::Scene_Play(const std::string& config, Game* engine)
{
	game = engine;
	init(config);
}

void Scene_Play::init(const std::string& path)
{
	// Read in the config values
	std::ifstream fin(path);
	std::string directive;
	bool failedToFind = true;

	while (fin >> directive)
	{
		failedToFind = false;
		std::cout << directive << std::endl;
		if (directive == "Player")
		{
			// Read player values
			fin >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB >>
				m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V;
		}
		else if (directive == "Enemy")
		{
			// Read enemy values
			fin >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >>
				m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
		}
		else if (directive == "Bullet")
		{
			// Read bullet values
			fin >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >>
				m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
		}
		else
		{
			std::cerr << "Directive not recognized for: " << directive << std::endl;
			exit(-1);
		}
	}
	if (failedToFind)
	{
		std::cout << "Failed to find" << std::endl;
	}

	spawnPlayer();
}

void Scene_Play::update()
{
	// some systems should function while paused (like rendering)
	// while others should not (like movement/input)
	m_entities.update();

	if (!m_paused)
	{
		// Do the things that you can do if not paused!
		sEnemySpawner();
		sMovement();
		sCollision();
		sLifespan();

		// Increment the current frame, only do when not paused
		m_currentFrame++;
	}

	// Always need to take in input and render, regardless of whether we're paused
	sRender();
}

// respawn the player in the middle of the screen
void Scene_Play::spawnPlayer()
{
	// We create every entity by calling EntityManager.addEntity(tag)
	// This returns an std::shared_ptr<Entity>, so we use 'auto' to save typing
	auto entity = m_entities.addEntity("player");

	// Spawn the player at the center of the window
	float mx = game->window().getSize().x / 2.0f;
	float my = game->window().getSize().y / 2.0f;
	entity->cTransform = std::make_shared<CTransform>(Vec2(mx, my), Vec2(0.0, 0.0), 0.0f);

	// Its shape will have the attributes defined by the m_playerConfig
	entity->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V, sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB),
		sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);

	// Add Collision
	entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);

	// Add an input component to the player so that we can use inputs
	entity->cInput = std::make_shared<CInput>();

	// Since we want this Entity to be our player, set our Scene_Play's player variable to this Entity
	// This goes slightly against the EntityManager paradigm, but we use the player so much it's worth it
	m_player = entity;
}

// Spawn an enemy at a random position
void Scene_Play::spawnEnemy()
{
	auto entity = m_entities.addEntity("enemy");

	// Spawns at a random position on screen
	// Min should be 0 + radius
	// Max should be m_window.getSize().x/y - radius
	int min, maxX, maxY;
	min = 0 + m_enemyConfig.SR;
	maxX = game->window().getSize().x - m_enemyConfig.SR;
	maxY = game->window().getSize().y - m_enemyConfig.SR;
	float ex = (float)randInRange(min, maxX);
	float ey = (float)randInRange(min, maxY);
	Vec2 originVec = Vec2(ex, ey);

	// Determine starting vector, pointing towards player, with random speed from config
	Vec2 targetVec = m_player->cTransform->pos - originVec;
	targetVec.normalize();
	int randSpeed = randInRange(m_enemyConfig.SMIN, m_enemyConfig.SMAX);
	targetVec *= randSpeed;
	entity->cTransform = std::make_shared<CTransform>(originVec, targetVec, 0.0f);

	// Determine random number of vertices from min and max defined in config
	int randVertices = randInRange(m_enemyConfig.VMIN, m_enemyConfig.VMAX);

	// Determine random color - RGB will each be a range from 0 to 255.
	int randR, randG, randB;
	randR = randInRange(0, 255);
	randG = randInRange(0, 255);
	randB = randInRange(0, 255);

	// Construct the entity's shape with random number of vertices, random color, and outline color set from config
	entity->cShape = std::make_shared<CShape>(m_enemyConfig.SR, randVertices, sf::Color(randR, randG, randB),
		sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), m_enemyConfig.OT);
	entity->cShape->circle.setOrigin(m_enemyConfig.SR, m_enemyConfig.SR);
	entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);

	// Give it a score equal to 100 * its vertices
	entity->cScore = std::make_shared<CScore>(100 * randVertices);

	// record when the most recent enemy was spawned
	m_lastEnemySpawnTime = m_currentFrame;

}

// spawns the small enemies when a big one explodes
void Scene_Play::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
	// When we create the smaller enemy, we have to read the values of the original enemy
	// - spawn a number of small enemies equal to the vertices of the original enemy
	// - set each small enemy to the same color as the original, half the size
	// - small enemies are worth double points of the original enemy
	size_t verts = e->cShape->circle.getPointCount();
	float angleSteps = (2 * 3.1415926) / (float)verts;
	// Speed is the parent's velocity's length
	float speed = e->cTransform->velocity.length();

	// Spawn a small enemy for each vertice of the parent enemy
	for (size_t i = 0; i < verts; i++)
	{
		auto smallEntity = m_entities.addEntity("enemy");
		float angle = angleSteps * (float)i;
		Vec2 smallVelocity = Vec2(cosf(angle), sinf(angle));
		smallVelocity *= speed;
		// Position is the same as the parent's, velocity is at an interval based on # of vertices
		// angle is i * angleSteps;
		// New velocity is Vec2(s * cosa, s*sina)
		smallEntity->cTransform = std::make_shared<CTransform>(e->cTransform->pos, smallVelocity, 0.0f);

		smallEntity->cShape = std::make_shared<CShape>(e->cShape->circle);
		float radius = smallEntity->cShape->circle.getRadius() / 2;
		smallEntity->cShape->circle.setRadius(radius);
		smallEntity->cShape->circle.setOrigin(radius, radius);
		smallEntity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR / 2);
		smallEntity->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
		smallEntity->cScore = std::make_shared<CScore>(e->cScore->score * 2);

	}


}

void Scene_Play::spawnBullet(std::shared_ptr<Entity> entity, const Vec2& target)
{
	// - bullet speed is given as a scalar speed
	// - you must set the velocity using formula in notes

	auto bullet = m_entities.addEntity("bullet");
	// cTransform(pos, vel, angle)
	// dVector is target - entity.position
	// We can normalize, and multiply by speed from here
	// or..
	// a is atan2f(dVec.y, dVec.x)
	// final vector Vec2(speed * cos a, speed * sin a)
	Vec2 originPosition = entity->cTransform->pos;
	Vec2 dVec = target - originPosition;
	// Normalizing and multiplying by speed
	dVec.normalize();
	dVec *= m_bulletConfig.S;
	bullet->cTransform = std::make_shared<CTransform>(originPosition, dVec, 0);

	// Give the bullet attributes as according to m_bulletConfig
	bullet->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V, sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB),
		sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB), m_bulletConfig.OT);
	bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);
	bullet->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);
}

void Scene_Play::spawnSpecialWeapon(std::shared_ptr<Entity> entity, const Vec2& target)
{
	auto bullet = m_entities.addEntity("specialWeapon");

	// Determine the direction vector
	Vec2 originPosition = entity->cTransform->pos;
	Vec2 dVec = target - originPosition;
	dVec.normalize();

	// Multiplying by speed, half that of a normal bullet
	dVec *= (m_bulletConfig.S / 2);
	bullet->cTransform = std::make_shared<CTransform>(originPosition, dVec, 0);

	// Shares shape of a normal bullet, but is three times the size
	bullet->cShape = std::make_shared<CShape>(m_bulletConfig.SR * 3, m_bulletConfig.V, sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB),
		sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB), m_bulletConfig.OT);
	bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR * 3);

	// Have it last three times as long
	bullet->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L * 3);
}

void Scene_Play::sMovement()
{
	for (auto e : m_entities.getEntities())
	{
		// Skip if it doesn't have a transform component
		if (e->cTransform == nullptr)
		{
			continue;
		}

		// Handle based on input if it has input
		if (!(e->cInput == nullptr))
		{
			e->cTransform->velocity = { 0.0, 0.0 };
			// Speed is determined by the player config
			if (e->cInput->up)
			{
				e->cTransform->velocity.y -= m_playerConfig.S;
			}
			if (e->cInput->down)
			{
				e->cTransform->velocity.y += m_playerConfig.S;
			}
			if (e->cInput->left)
			{
				e->cTransform->velocity.x -= m_playerConfig.S;
			}
			if (e->cInput->right)
			{
				e->cTransform->velocity.x += m_playerConfig.S;
			}

			// If we have a value for both x and y, calculate the 'correct' vector
			if (!(e->cTransform->velocity.x == 0 || e->cTransform->velocity.y == 0))
			{
				// angle is arctan of y/x, speed is speed
				float a = std::atan2f(e->cTransform->velocity.y, e->cTransform->velocity.x);
				e->cTransform->velocity = Vec2::fromAngleAndSpeed(a, m_playerConfig.S);
			}

			Vec2 outOfBounds = outOfBoundsVec(e);
			if (outOfBounds != Vec2(0.0, 0.0))
			{
				// Detecting a corner
				if (outOfBounds == e->cTransform->velocity * -1)
				{
					e->cTransform->velocity = { 0.0, 0.0 };

				}
				// Allow movement in the non-blocked direction.
				else
				{
					if (outOfBounds.x != e->cTransform->velocity.x)
					{
						e->cTransform->velocity.x = 0.0f;
					}

					if (outOfBounds.y != e->cTransform->velocity.y)
					{
						e->cTransform->velocity.y = 0.0f;
					}
				}
			}

		}
		// Otherwise, just update
		else
		{
			// Handle bounds for enemies remove conditional if you want bullets to bounce too!
			if (e->tag() == "enemy")
			{
				Vec2 outOfBounds = outOfBoundsVec(e);
				if (outOfBounds != Vec2(0.0, 0.0))
				{
					e->cTransform->velocity = outOfBounds;

				}
			}
		}

		e->cTransform->pos += e->cTransform->velocity;
	}
}

void Scene_Play::sLifespan()
{
	// Handle lifespan logic for all entities with a lifespan component
	for (auto e : m_entities.getEntities())
	{
		//  if entity has no lifespan component, skip it
		if (e->cLifespan == nullptr)
		{
			continue;
		}
		//	if entity has > 0 remaining lifespan, subtract 1
		if (e->cLifespan->remaining > 0)
		{
			e->cLifespan->remaining -= 1;
			
			if (e->cShape != nullptr)
			{
				//  if it has lifespan and is alive
				//		scale its alpha channel properly
				auto currentFillColor = e->cShape->circle.getFillColor();
				auto currentOtColor = e->cShape->circle.getOutlineColor();
				float lifespanRatio = (float)e->cLifespan->remaining / (float)e->cLifespan->total;
				currentFillColor.a = 255 * lifespanRatio;
				currentOtColor.a = 255 * lifespanRatio;

				// Special weapon grows and changes all of its colors!
				if (e->tag() == "specialWeapon")
				{
					// Limit flashing to about four times a second - best practices for flashing patterns
					if (m_currentFrame % (game->frameRateLimit() / 4) == 0)
					{
						currentFillColor.r = randInRange(50, 255);
						currentFillColor.g = randInRange(50, 255);
						currentFillColor.b = randInRange(50, 255);
						currentOtColor.r = currentFillColor.r;
						currentOtColor.g = currentFillColor.g;
						currentOtColor.b = currentFillColor.b;

					}

					// Grow bigger!
					// What I want, is to target getting 3x bigger than the original size
					// And linearly achieve that scale based on the lifespan ratio 1 + (2 * (1 - lifespanRatio))
					float linearTripleGrowth = (1 + (2 * (1 - lifespanRatio)));
					e->cShape->circle.setScale(linearTripleGrowth, linearTripleGrowth);
					e->cCollision->radius = e->cShape->circle.getRadius() * linearTripleGrowth;
				}

				e->cShape->circle.setFillColor(currentFillColor);
				e->cShape->circle.setOutlineColor(currentOtColor);

			}
		}
		//	if it has lifespan and time is up destroy the entity
		if (e->cLifespan->remaining <= 0)
		{
			e->destroy();
		}
	}
}

void Scene_Play::sCollision()
{
	// Handle collision logic for different entity types
	// Start with enemies since all collisions are currently based on that, minimize retracing steps
	for (auto e : m_entities.getEntities("enemy"))
	{
		if (m_player->cTransform->pos.dist(e->cTransform->pos) < e->cCollision->radius + m_player->cCollision->radius)
		{
			// When player is hit, return to center and reduce score by score of the shape that hit you
			m_player->cTransform->pos = Vec2(game->window().getSize().x / 2, game->window().getSize().y / 2);
			if (m_score > 0)
			{
				int diff = m_score - e->cScore->score;
				m_score = (diff > 0) ? diff : 0;
			}

			if(e->cLifespan == nullptr)
			{
				spawnSmallEnemies(e);
			}
			e->destroy();
		}

		// When a bullet hits an enemy, destroy both and increment the score by the enemy's worth
		for (auto b : m_entities.getEntities("bullet"))
		{
			if (b->cTransform->pos.dist(e->cTransform->pos) < b->cCollision->radius + e->cCollision->radius)
			{
				b->destroy();
				m_score += e->cScore->score;

				// If it was a permanent enemy, spawnSmallEnemies
				if(e->cLifespan == nullptr)
				{
					spawnSmallEnemies(e);
				}
				e->destroy();
			}

		}

		// When the special hits an enemy, destroy the enemy, but leave the special projectile in motion. Increment score.
		for (auto s : m_entities.getEntities("specialWeapon"))
		{
			if (s->cTransform->pos.dist(e->cTransform->pos) < s->cCollision->radius + e->cCollision->radius)
			{
				m_score += e->cScore->score;

				if(e->cLifespan == nullptr)
				{
					spawnSmallEnemies(e);
				}
				e->destroy();
			}

		}
	}
}

void Scene_Play::sEnemySpawner()
{
	if (m_currentFrame - m_lastEnemySpawnTime > m_enemyConfig.SI)
	{
		spawnEnemy();
	}
	
}

void Scene_Play::sRender()
{
	game->window().clear();


	// Eventually will look something like
	for (auto e : m_entities.getEntities())
	{
		e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);

		// set the rotation of the shape based on the entity's transform->angle
		e->cTransform->angle += 1.0f;
		e->cShape->circle.setRotation(e->cTransform->angle);

		// draw the entity's sf::CircleShape
		game->window().draw(e->cShape->circle);
	}

	game->text().setString("Score: " + std::to_string(m_score));
	game->window().draw(game->text());
	game->window().display();
}

void Scene_Play::sDoAction(const Action& action)
{
	if (action.type() == "START")
	{
		if (action.name() == "UP")
		{
			m_player->cInput->up = true;
		}
		else if (action.name() == "DOWN")
		{
			m_player->cInput->down = true;
		}
		else if (action.name() == "LEFT")
		{
			m_player->cInput->left = true;
		}
		else if (action.name() == "RIGHT")
		{
			m_player->cInput->right = true;
		}
		else if (action.name() == "SHOOT")
		{
			spawnBullet(m_player, m_target);
		}
		else if (action.name() == "SPECIALSHOOT")
		{
			if (m_currentFrame > m_lastSpecialShot + 180)
			{
				spawnSpecialWeapon(m_player, m_target);
				m_lastSpecialShot = m_currentFrame;
			}
		}
	}
	else if (action.type() == "END")
	{
		if (action.name() == "UP")
		{
			m_player->cInput->up = false;
		}
		else if (action.name() == "DOWN")
		{
			m_player->cInput->down = false;
		}
		else if (action.name() == "LEFT")
		{
			m_player->cInput->left = false;
		}
		else if (action.name() == "RIGHT")
		{
			m_player->cInput->right = false;
		}

	}
}

bool Scene_Play::goingOutOfBounds(std::shared_ptr<Entity> entity)
{
	if (entity->cTransform == nullptr)
	{
		return false;
	}

	Vec2 translatedVec = entity->cTransform->pos + entity->cTransform->velocity;
	float radius = entity->cShape->circle.getRadius();
	// Check top
	if (translatedVec.y - radius < 0)
	{
		return true;
	}
	// Check bottom
	if (translatedVec.y + radius > game->window().getSize().y)
	{
		return true;
	}
	// Check left
	if (translatedVec.x - radius < 0)
	{
		return true;
	}
	// Check right
	if (translatedVec.x + radius > game->window().getSize().x)
	{
		return true;
	}

	return false;
}

// Return the "bounce vector" if an entity is moving out of bounds.
// Return Vec2(0.0) when an entity can't be processed, or is not out of bounds.
Vec2 Scene_Play::outOfBoundsVec(std::shared_ptr<Entity> entity)
{
	Vec2 outOfBoundsVec = entity->cTransform->velocity;
	if (entity->cTransform == nullptr)
	{
		return Vec2(0.0, 0.0);
	}

	Vec2 translatedVec = entity->cTransform->pos + entity->cTransform->velocity;
	float radius = entity->cShape->circle.getRadius();
	// Check top
	if (translatedVec.y - radius < 0)
	{
		outOfBoundsVec.y *= -1;
	}
	// Check bottom
	if (translatedVec.y + radius > game->window().getSize().y)
	{
		outOfBoundsVec.y *= -1;
	}
	// Check left
	if (translatedVec.x - radius < 0)
	{
		outOfBoundsVec.x *= -1;
	}
	// Check right
	if (translatedVec.x + radius > game->window().getSize().x)
	{
		outOfBoundsVec.x *= -1;
	}

	if (outOfBoundsVec == entity->cTransform->velocity)
	{
		return Vec2(0.0, 0.0);
	}

	return outOfBoundsVec;
}

// Return a random integer within the range provided
int Scene_Play::randInRange(int min, int max)
{
	return min + (rand() % (1 + max - min));
}

void Scene_Play::setTarget(const Vec2& target)
{
	m_target = target;
}
