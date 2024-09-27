#pragma once

#include <SFML/Graphics.hpp>
#include "Scene_Play.h"

class Scene_Play; // Resolving circular dependency

class Game
{
	sf::RenderWindow m_window; // The window we will draw to
	sf::Font m_font;
	sf::Text m_text;
	bool m_paused = false;
	bool m_running = true;

	std::shared_ptr<Scene_Play> m_scene;

	int m_frameRateLimit = 60;

	void init(const std::string& config); // Initialize the GameState with a config file
	void setPaused(bool paused);

public:
	Game(const std::string& config); // constructor, takes in the game config

	void run();
	sf::RenderWindow& window();
	sf::Font& font();
	sf::Text& text();
	const int frameRateLimit() const;
};