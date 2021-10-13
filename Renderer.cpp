#include "Renderer.hpp"
#include <fstream>
#include <chrono>
#include <random>

void Renderer::initWindow()
{
	sf::ContextSettings settings;
	settings.antialiasingLevel = 4;

	window.create(sf::VideoMode(800, 700), "PathFinder", sf::Style::Default, settings);
	window.setFramerateLimit(60);

	frame.setWindow(window);
}

Renderer::Renderer()
{
	this->initWindow();

	running = true;
	paused = false;

	font.loadFromFile("Amel Brush Font.ttf");
	frame.addEntity(start);
	frame.addEntity(stepStart);
	frame.addEntity(stop);
	frame.addEntity(next);

	start.setPosition(550, 50);
	start.setSize(sf::Vector2f(200, 75));
	start.setOutlineColor(sf::Color::Green);
	start.setOutlineThickness(2);
	start.setFillColor(sf::Color::Transparent);
	start.setSelectionFillColor(sf::Color(0, 255, 0, 84));
	start.setSelectionOutlineColor(sf::Color(63, 255, 202));
	start.setSelectionOutlineThickness(4);
	start.setCharacterSize(48);
	start.setTextFillColor(sf::Color::White);
	start.setFont(font);
	start.setString("START");

	stepStart = start;
	stepStart.setPosition(550, 250);
	stepStart.setString("STEP");

	next = start;
	next.setPosition(550, 350);
	next.setString("NEXT");
	next.setInactive();

	stop.setSize(sf::Vector2f(200, 75));
	stop.setPosition(550, 150);
	stop.setFillColor(sf::Color::Transparent);
	stop.setOutlineColor(sf::Color::Red);
	stop.setOutlineThickness(3);
	stop.setSelectionFillColor(sf::Color(255, 0, 0, 126));
	stop.setSelectionOutlineColor(sf::Color(255, 165, 0));
	stop.setSelectionOutlineThickness(5);
	stop.setCharacterSize(48);
	stop.setTextFillColor(sf::Color::White);
	stop.setFont(font);
	stop.setString("STOP");

	start.setAction([this] 
		{
			start.setInactive();
			stepStart.setInactive();
			next.setInactive();

			state = SOLVE;
			solve(player.x, player.y);
		}
	);

	stop.setAction([this]
		{
			start.setActive();
			stepStart.setActive();
			next.setInactive();

			state = STOP;
			paused = false;
			if (mazeNum == 1)
				loadMap("Maze.txt", 23, 21);
			else if (mazeNum == 2)
				loadMap("Maze 2.txt", 37, 14);
		}
	);

	stepStart.setAction([this]
		{
			start.setInactive();
			stepStart.setInactive();
			next.setActive();

			state = STEP;
			solve(player.x, player.y);
		}
	);

	next.setAction([this]
		{
			paused = false;
		}
	);

	mazeNum = 1;
	stop.callAction();
}

Renderer::~Renderer()
{

}

bool Renderer::isRunning()
{
	return this->running;
}

#include <iostream>
void Renderer::loadMap(std::string name, int mapSize, int tile)
{
	tileSize = tile;

	maze.clear();

	std::string line;
	std::ifstream file;
	file.open(name.c_str(), std::ios::in);

	for (int i = 0; i < mapSize; i++) {
		maze.emplace_back(std::vector<int>(mapSize, 0));

		std::getline(file, line);
		for (int j = 0; j < mapSize; j++) {
			if (line[j] == 'P') {
				maze[i][j] = 2;
				player = { j, i };
			}
			if (line[j] == '@') {
				maze[i][j] = 0;
			}
			else if (line[j] == 'F') {
				maze[i][j] = 9;
			}
			else if (line[j] == ' ') {
				maze[i][j] = 1;
			}
		}
	}
	file.close();

	for (int i = 0; i < maze.size(); i++) {
		std::cout << "{ ";
		for (int j = 0; j < maze[i].size(); j++){
			std::cout << maze[i][j];
			if(j != maze[i].size() - 1)std::cout << ", ";
		}
		std::cout << " },\n";
	}
}

int Renderer::solve(int x, int y)
{
	if (state == STOP)
		return 0;
	else
	{
		if (x < 0 || y < 0 || x >= maze[0].size() || y >= maze.size() || !maze[y][x] || maze[y][x] == 3 || maze[y][x] == 5) {
			return -1;
		}

		if (maze[y][x] == 9) return 1;

		maze[y][x] = 3;

		if (state == STEP) {
			paused = true;
		}

		do {
			update();
			render();
		} while (paused);

		int dir[4][2] = { {0, -1}, {1, 0}, {0, 1}, {-1, 0} };

		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

		shuffle(dir, dir + 4, std::default_random_engine(seed));

		int i = 0;
		int result = -1;
		while (result == -1 && i < 4) {
			result = solve(x + dir[i][0], y + dir[i][1]);
			i++;
		}

		if (result == -1)
			maze[y][x] = 4;
		else if (result == 1)
			maze[y][x] = 5;

		else maze[y][x] = 1;

		if (result == -1 && x >= 0 && y >= 0 && x < maze[0].size() && y < maze.size() && maze[y][x]) {
			if (state == STEP) {
				paused = true;
			}

			do {
				update();
				render();
			} while (paused);
		}

		return result;
	}
}

void Renderer::update()
{
	this->pollEvents();

	frame.update();
}

void Renderer::pollEvents()
{
	sf::Event event;
	sf::Vector2f mousePos = this->getMousePosition();
	while (this->window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed || event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
			this->running = false;

		if (event.type == sf::Event::KeyPressed) {
			if (event.key.code == sf::Keyboard::Space)
				this->paused = !this->paused;

			else if (state == STEP && (event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::N))
				next.callAction();

			else if (event.key.code == sf::Keyboard::S){
				if (mazeNum & 1)mazeNum = 2;
				else mazeNum = 1;
				stop.callAction();
			}

			else if (event.key.code == sf::Keyboard::Z) {
				stop.callAction();

				start.setInactive();
				stop.setInactive();
				stepStart.setInactive();
				next.setInactive();

				std::vector<std::vector<int>> copy(maze);

				while (true)
				{
					state = SOLVE;
					solve(player.x, player.y);
					maze = copy;
				}
			}
		}

		frame.pollEvents(event);

	}
}

void Renderer::render()
{
	this->window.clear();

	sf::Vector2f offset(25, 25);
	sf::Vertex square[4];
	
	for (int i = 0; i < maze.size(); i++) {
		for (int j = 0; j < maze[i].size(); j++) {
			for (int k = 0; k < 4; k++) {

				square[k].position = offset + (sf::Vector2f(j, i) + sf::Vector2f(((k&1)^(k>>1)), ((k>>1)&1))) * tileSize;
				
				if (player.x == j && player.y == i)
					square[k].color = sf::Color::Red;
				else if(maze[i][j] == 2 || maze[i][j] == 9)
					square[k].color = sf::Color::Magenta;
				else if(maze[i][j] == 3)
					square[k].color = sf::Color::Yellow;
				else if(maze[i][j] == 4)
					square[k].color = sf::Color::Red;
				else if(maze[i][j] == 5)
					square[k].color = sf::Color::Green;
				else if(maze[i][j])
					square[k].color = sf::Color::White;
				else
					square[k].color = sf::Color::Black;
			}

			window.draw(square, 4, sf::Quads);
		}
	}

	frame.draw();

	this->window.display();
}

sf::Vector2f Renderer::getMousePosition()
{
	return window.mapPixelToCoords(sf::Mouse::getPosition(this->window));
}

sf::Vector2f Renderer::getWinSize()
{
	return sf::Vector2f(this->window.getSize());
}