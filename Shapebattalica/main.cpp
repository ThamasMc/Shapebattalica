#include <SFML/Graphics.hpp>
#include <iostream>
#include "Game.h"


int main() {
    Vec2::test();

    Game g("config.txt");
    g.run();

	return 0;
}