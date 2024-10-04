#pragma once

#include <SFML/Graphics.hpp>
#include "Scene.h"
#include <map>

class Scene; // Resolving circular dependency

class Game
{
	sf::RenderWindow m_window; // The window we will draw to
	sf::Font m_font;
	sf::Text m_text;
	bool m_paused = false;
	bool m_running = true;

	std::map<std::string, std::shared_ptr<Scene>> scenes;
	std::string current_scene = "NONE";

	int m_frameRateLimit = 60;

	void init(const std::string& config); // Initialize the GameState with a config file
	void setPaused(bool paused);

	std::shared_ptr<Scene> currentScene();

public:
	Game(const std::string& config); // constructor, takes in the game config

	void run();
	sf::RenderWindow& window();
	sf::Font& font();
	sf::Text& text();
	const int frameRateLimit() const;

	void sUserInput();
	void changeScene(std::string name, std::shared_ptr<Scene> scene);
};