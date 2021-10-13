#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <ZUI.hpp>

class Renderer {
private:
	sf::RenderWindow window;
	bool running;
	bool paused;

	sf::Font font;

	zui::Frame frame;
	zui::TextButton start, stepStart, next, stop;

	std::vector<std::vector<int>> maze;
	sf::Vector2i player;
	float tileSize;
	int mazeNum;

	enum State{SOLVE, STEP, STOP} state;


	void initWindow();

	void loadMap(std::string name, int size, int tileSize);

	int solve(int x, int y);

public:
	Renderer();
	~Renderer();

	bool isRunning();

	void update();
	void pollEvents();
	void render();

	sf::Vector2f getMousePosition();
	sf::Vector2f getWinSize();
};
