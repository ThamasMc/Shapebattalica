#include "Game.h"
#include <iostream>
#include <fstream>
#include <math.h>

Game::Game(const std::string& config)
{
	init(config);
}

void Game::init(const std::string& path)
{
	// Read in the config values
	std::ifstream fin(path);
	std::string directive, fontPath;
	int wWidth, wHeight, fullscreen, fontSize, red, green, blue = 0;
	bool failedToFind = true;

	while (fin >> directive)
	{
		failedToFind = false;
		std::cout << directive << std::endl;
		if (directive == "Window")
		{
			// Read window values
			fin >> wWidth >> wHeight >> m_frameRateLimit >> fullscreen;
		}
		else if (directive == "Font")
		{
			// Read font values
			fin >> fontPath >> fontSize >> red >> green >> blue;
			// Setting up font
			if (!m_font.loadFromFile(fontPath))
			{
				std::cerr << "Could not load the font" << std::endl;
				exit(-1);
			}
			m_text = sf::Text("Score: " + 0, m_font, fontSize);
			m_text.setFillColor(sf::Color(red, green, blue));
			m_text.setPosition(0 + m_text.getCharacterSize(), (0 + m_text.getCharacterSize()));
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

	// Create window
	/*
	if (fullscreen == 1)
	{
		m_window.create(sf::VideoMode(wWidth, wHeight), "Shapebattlia", sf::Style::Fullscreen);

	}
	else {
		m_window.create(sf::VideoMode(wWidth, wHeight), "Shapebattlia");
	}
	*/
	m_window.create(sf::VideoMode(1280, 720), "Shapebattalia");
	m_window.setFramerateLimit(m_frameRateLimit);

	m_scene = std::make_shared<Scene_Play>("scenePlayConfig.txt", this);
}

void Game::run()
{
	// some systems should function while paused (like rendering)
	// while others should not (like movement/input)
	while (m_scene->running())
	{
		m_scene->update();
	}
}

void Game::setPaused(bool paused)
{
	m_paused = paused;
}

sf::RenderWindow& Game::window()
{
	return m_window;
}

sf::Font& Game::font()
{
	return m_font;
}

sf::Text& Game::text()
{
	return m_text;
}

const int Game::frameRateLimit() const
{
	return m_frameRateLimit;
}