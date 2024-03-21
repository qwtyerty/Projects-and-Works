#include <cstdint>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Cat.h"
#include "Mouse.h"
#include "RNG.h"

// Define helper functions
std::vector<std::vector<int>> readCSVFromStream(std::ifstream &stream);
std::vector<std::vector<int>> createPlayGrid(std::vector<std::vector<int>> grid);
void displayMaze(std::vector<std::vector<int>> grid, GLuint VAO, GLuint shaderProgram);
void displayGrid(std::vector<std::vector<int>> grid, GLuint VAO, GLuint gridShaderProgram, GLFWwindow* window);
void startNextTurn(std::vector<std::vector<int>>& grid, Mouse* mouse, Cat* cat);
void applyMove(std::vector<std::vector<int>>& grid, GridType gridType, std::pair<int, int> previousCoordinates, std::pair<int, int> newCoordinates, Mouse* pMouse, int* pCheeseEaten);
void spawnNewCheese(std::vector<std::vector<int>>& grid);



// Defines

const int WIN_WIDTH	=			1024;	/* Window Width */
const int WIN_HEIGHT =			1024;	/* Window Height */
const int PROCESS_SPEED =		20;		/* Moves done pers second */
const GLfloat GRID_LINE_WIDTH =	2.0f;	/* Width of the grid lines*/

// Color Defines
const GLfloat BG_COLOR[] =		{ 0.75f,  0.75f, 0.75f, 1.0F };
const GLfloat GRID_COLOR[] =	{ 0.91f,  0.91f, 0.91f };	
const GLfloat PATH_COLOR[] =	{ 0.0f,  0.0f, 0.0f };
const GLfloat WALL_COLOR[] =	{ 0.9f,  0.9f, 0.9f };
const GLfloat MOUSE_COLOR[] =	{ 0.7f,  0.7f, 0.7f };
const GLfloat CAT_COLOR[] =		{ 1.0f,  0.6f, 0.0f };
const GLfloat CHEESE_COLOR[] =	{ 0.9f,  0.9f, 0.5f };


// Define a square tile as a set of vertices
const GLfloat VERTICES[] =
{
	0.0, 0.0,
	0.0, 1.0,
	1.0, 1.0,
	1.0, 0.0
};

// Order to read the vertices to construct the square tile
const GLuint INDICES[] =
{
	0, 1, 2,
	2, 3, 0
};

// Vertex Shader Code
const GLchar* sVShaderCode =
"#version 330 core\n"
"layout (location = 0) in vec2 v_pos;\n"
"uniform float POSITION;\n"
"uniform vec2 GRID_COUNT;\n "
"void main(){\n"
"	vec2 GRID_SIZE = vec2(2/ GRID_COUNT.x, 2 / GRID_COUNT.y);\n"
"	vec2 pos_center = vec2((mod(POSITION, GRID_COUNT.x) * GRID_SIZE.x) - 1 + (GRID_SIZE.x/2),\n"
"		1.0 - GRID_SIZE.y - (floor(POSITION / GRID_COUNT.x) *GRID_SIZE.y) + (GRID_SIZE.y/2));\n"
"	gl_Position = vec4(\n"
"		pos_center.x - (GRID_SIZE.x /2) + (v_pos.x * GRID_SIZE.x), \n"
"		pos_center.y - (GRID_SIZE.y /2) + (v_pos.y * GRID_SIZE.y),\n"
"		0.0, 1.0);\n"
"}\0";

// Fragment Shader Code
const GLchar* sFShaderCode =
"#version 330 core\n"
"out vec4 fragment_color;\n"
"uniform vec3 COLOR;\n"
"void main(){\n"
"	fragment_color = vec4(COLOR, 1.0);\n"
"}\0";

// Grid Vertex Shader Code
const GLchar* sGridVShaderCode =
"#version 330 core\n"
"layout (location = 0) in vec2 v_pos;\n"
"void main(){\n"
"  gl_Position = vec4(v_pos * 2 - 1, 0.5, 1.0);\n"
"}\0";

// Grid Fragment Shader Code
const GLchar* sGridFShaderCode =
"#version 330 core\n"
"out vec4 fragment_color;\n"
"uniform vec3 COLOR;\n"
"uniform vec2 WIN_SIZE;\n"
"uniform vec2 GRID_COUNT;\n"
"uniform float GRID_LINE_WIDTH;\n"
"void main(){\n"
"  vec2 GRID_SIZE = vec2(WIN_SIZE.x / GRID_COUNT.x, WIN_SIZE.y / GRID_COUNT.y);\n"
"  vec2 mod_val = vec2(mod(gl_FragCoord.x, GRID_SIZE.x), mod(gl_FragCoord.y, GRID_SIZE.y));\n"
"  if(\n"
"    (mod_val.x < GRID_LINE_WIDTH / 2 || mod_val.x > GRID_SIZE.x - GRID_LINE_WIDTH / 2) &&\n"
"    (mod_val.y < GRID_LINE_WIDTH * 2 || mod_val.y > GRID_SIZE.y - GRID_LINE_WIDTH * 2)\n"
"  )\n"
"    fragment_color = vec4(COLOR, 1.0);\n"
"  else if(\n"
"    (mod_val.y < GRID_LINE_WIDTH / 2 || mod_val.y > GRID_SIZE.y - GRID_LINE_WIDTH / 2) &&\n"
"    (mod_val.x < GRID_LINE_WIDTH * 2 || mod_val.x > GRID_SIZE.x - GRID_LINE_WIDTH * 2)\n"
"  )\n"
"    fragment_color = vec4(COLOR, 1.0);\n"
"  else discard;\n"
"}\0";

int main(int argc, char** argv)
{
	std::ifstream fileStream;

	// Validate the file and attempt to open it
	if (argc > 1)
	{
		fileStream.open(argv[1]);
		if (!fileStream.is_open())
		{
			std::cerr << "Failed to open file: '" << argv[1] << "'.\n";
			return 1;
		}
	}
	else
	{
		std::cerr << "No file provided, file required.";
		return 1;
	}

	// Read the CSV and create the play grid
	std::vector<std::vector<int>> vectorGrid = readCSVFromStream(fileStream);
	std::vector<std::vector<int>> playGrid = createPlayGrid(vectorGrid);

	// Clear the vector grid
	vectorGrid.clear();

	//initalize the mouse
	Mouse mouse(&playGrid);

	//Initialize the cat
	Cat cat(&playGrid, &mouse);

	if (!glfwInit()) 
	{
		std::cerr << "Failed to init GLFW.\n";
	}

	// Pass hints to window to know to use 3.3 Core
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

	// Create the window
	GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Cat and Mouse Maze", NULL, NULL);
	if (window == NULL) 
	{
		std::cerr << "Failed creating GLFW window.\n";
		glfwTerminate();

	}

	// Make the context of the window current
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
	{
		std::cerr << "Failed creating GLAD (OpenGL).\n";
		glfwTerminate();
	}

	// Compile shaders
	GLuint vShader, fShader, shaderProgram;
	vShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vShader, 1, &sVShaderCode, NULL);
	glCompileShader(vShader);
	fShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fShader, 1, &sFShaderCode, NULL);
	glCompileShader(fShader);

	// Link shaders to program and delete shaders
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vShader);
	glAttachShader(shaderProgram, fShader);
	glLinkProgram(shaderProgram);
	glDeleteShader(vShader);
	glDeleteShader(fShader);

	// Repeat for Grid Shaders
	GLuint gridVShader, gridFShader, gridShaderProgram;
	gridVShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(gridVShader, 1, &sGridVShaderCode, NULL);
	glCompileShader(gridVShader);
	gridFShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(gridFShader, 1, &sGridFShaderCode, NULL);
	glCompileShader(gridFShader);

	gridShaderProgram = glCreateProgram();
	glAttachShader(gridShaderProgram, gridVShader);
	glAttachShader(gridShaderProgram, gridFShader);
	glLinkProgram(gridShaderProgram);
	glDeleteShader(gridFShader);
	glDeleteShader(gridFShader);


	// Generate and bind VAO, VBO, and EBO
	GLuint VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDICES), INDICES, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
	glad_glEnableVertexAttribArray(0);

	// Get the current time for time keeping
	GLdouble currentTime, previousTime, deltaTime, sumTime;
	currentTime = previousTime = glfwGetTime();
	deltaTime = sumTime = 0.0;

	// main function loop
	while (!glfwWindowShouldClose(window)) 
	{
		// Set the background color
		glClearColor(BG_COLOR[0], BG_COLOR[1], BG_COLOR[2], BG_COLOR[3]);
		glClear(GL_COLOR_BUFFER_BIT);

		// Calculate elapsed time and ensure speed is slowed to be easily visible
		currentTime = glfwGetTime();
		deltaTime = currentTime - previousTime;
		previousTime = currentTime;
		sumTime += deltaTime;

		// Only calculate moves at PROCESS_SPEED FPS
		if (sumTime >= 1.0 / PROCESS_SPEED) 
		{
			startNextTurn(playGrid, &mouse, &cat);
			sumTime = 0.0;
		}

		// Display the object and poll events
		displayMaze(playGrid, VAO, shaderProgram);
		displayGrid(playGrid, VAO, gridShaderProgram, window);
		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	// Close the window
	glfwTerminate();

	return 0;
}


/**
 *	@brief		Read a csv file from a file stream
 *	@details	Read a csv file from a file stream and stores it into a vector grid
 *	@param		stream:		File stream to read from
 *	@retval		2D vector representing the csv file
 */
std::vector<std::vector<int>> readCSVFromStream(std::ifstream &stream) 
{
	// Read and store the contents of the file in a 2d Vector
	std::string line, values;
	std::vector<std::vector<int>> vectorGrid;

	// Iterate over the file line-by-line storing lines into the vectorGrid
	while (std::getline(stream, line))
	{
		std::vector<int> row;
		std::istringstream s(line);

		while (std::getline(s, values, ','))
		{
			row.push_back(std::stoi(values));
		}

		vectorGrid.push_back(row);
	}

	return vectorGrid;
}

/**
 *	@brief		Converts a 2D vector representing the CSV file into a play grid
 *	@details	Converts a 2D vector by reading the first line for diminesions then reading over the
 *				vector and padding the resulting vector to have walls on all sides 
 *	@param		grid: Grid generated from a csv file
 *	@retval		Converted 2D vector representing the maze
 */
std::vector<std::vector<int>> createPlayGrid(std::vector<std::vector<int>> grid)
{
	// Create the play grid 2D vector
	std::vector<std::vector<int>> playGrid;

	int width = grid[0][0];
	int height = grid[0][1];

	//Pad first row with 1's to pad with walls
	std::vector<int> padRow(width+2, 1);
	playGrid.push_back(padRow);

	//Inclusive as we are shifted over by 1
	for (int i = 1; i <= height; i++)
	{
		// Initialize new row
		std::vector<int> row;

		// Pad the left and right sides with 0's (Walls)
		row.push_back(1);
		row.insert(std::end(row), std::begin(grid[i]), std::end(grid[i]));
		row.push_back(1);

		// Append to the play grid
		playGrid.push_back(row);
	}

	//Pad final row with walls
	playGrid.push_back(padRow);

	return playGrid;
}

/**
 *	@brief		Converts a 2D vector from values into graphics
 *	@details	Converts a 2D vector from values into graphics
 *	@param		grid:			2D vector representing the grid
 *	@param		VAO:			Vectex Object List
 *	@param		shaderProgram:	Shader Program for rendering the objects
 *	@retval		None
 */
void displayMaze(std::vector<std::vector<int>> grid, GLuint VAO, GLuint shaderProgram) 
{
	int i = 0;
	GLfloat color[3];

	// Interate over the grid and count as a 1d array
	for (auto& row : grid)
	{
		for (auto& value : row)
		{
			switch (value)
			{

				// Set the color of the tile
				case 0:
					std::memcpy(color, PATH_COLOR, sizeof(PATH_COLOR));
					break;

				case 1:
					std::memcpy(color, WALL_COLOR, sizeof(WALL_COLOR));
					break;

				case 2:
					std::memcpy(color, MOUSE_COLOR, sizeof(MOUSE_COLOR));
					break;

				case 3:
					std::memcpy(color, CAT_COLOR, sizeof(CAT_COLOR));
					break;

				case 4:
					std::memcpy(color, CHEESE_COLOR, sizeof(CHEESE_COLOR));
					break;

				default:
					break;

			}

			// Display grid objects using triangles using the set color
			glBindVertexArray(VAO);
			glUseProgram(shaderProgram);
			glUniform1f(glGetUniformLocation(shaderProgram, "POSITION"), (GLfloat) i);
			glUniform2f(glGetUniformLocation(shaderProgram, "GRID_COUNT"), (GLfloat)grid.size(), (GLfloat)grid[0].size());
			glUniform3f(
				glGetUniformLocation(shaderProgram, "COLOR"),
				color[0],
				color[1],
				color[2]);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			// Increment counter
			i++;
		}
	}
}

/**
 *	@brief		Converts a 2D vector from values into graphics
 *	@details	Converts a 2D vector from values into graphics
 *	@param		grid:			2D vector representing the grid
 *	@param		VAO:			Vectex Object List
 *	@param		shaderProgram:	Shader Program for rendering the objects
 *	@param		window:			Window to read dimensions from
 *	@retval		None
 */
void displayGrid(std::vector<std::vector<int>> grid, GLuint VAO, GLuint gridShaderProgram, GLFWwindow* window)
{
	// Use the gridShader
	glUseProgram(gridShaderProgram);
	glUniform3f(
		glGetUniformLocation(gridShaderProgram, "COLOR"),
		GRID_COLOR[0],
		GRID_COLOR[1],
		GRID_COLOR[2]
	);

	// Pass variables to the shader program and draw grid objects using the triangle
	int windowWidth, windowHeight;
	glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
	glUniform2f(
		glGetUniformLocation(gridShaderProgram, "WIN_SIZE"),
		(GLfloat)windowWidth,
		(GLfloat)windowHeight);
	glUniform2f(
		glGetUniformLocation(gridShaderProgram, "GRID_COUNT"),
		(GLfloat)grid.size(),
		(GLfloat)grid[0].size());
	glUniform1f(
		glGetUniformLocation(gridShaderProgram, "GRID_LINE_WIDTH"),
		GRID_LINE_WIDTH);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

/**
 *	@brief		Select moves for the cat and mouse then move them
 *	@details	Call movement functions for the cat and mouse then moving them, then executing the moves
 *				then handling edge cases of the cat eatting the mouse and the mouse eatting the cheese
 *	@param		grid:		Reference to the grid for updating possitions
 *	@param		pMouse:		Pointer to the mouse
 *	@param		pCat:		Pointer to the cat
 * 
 */
void startNextTurn(std::vector<std::vector<int>>& grid, Mouse* pMouse, Cat* pCat) 
{
	// Execute moves for cat and mouse
	pMouse->selectNextMove();
	pMouse->movePosition();
	pCat->selectNextMove();
	pCat->movePosition();

	int cheeseEaten = 0;
	// Apply the movements
	applyMove(grid, MOUSE, pMouse->getPreviousPosition(), pMouse->getCurrentPosition(), pMouse, &cheeseEaten);
	applyMove(grid, CAT, pCat->getPreviousPosition(), pCat->getCurrentPosition(), pMouse, &cheeseEaten);
	
	// Check the status of the cheese
	if (cheeseEaten == 1)
	{
		spawnNewCheese(grid);
	}
}

/**
 *	@brief		Applies moves to the grid 
 *	@details	Applies moves to the grid and handle the mouse and cat both moving to the same spot, by
 *				having the mouse be eaten and spawned in a new random spot
 *	@param		grid:					Reference to the grid for updating
 *	@param		gridType:				gridType to apply at the new position
 *	@param		previousCoordinates:	Set of the previous coordinates to make empty
 *	@param		newCoordinates:			Set of the new coordinates to place the entity
 *	@param		pMouse:					Pointer to the mouse for spawning if eaten
 *	@param		pcheeseEaten:			Pointer to flag for setting if cheese has been eaten
 *	@retval		None
 */
void applyMove(std::vector<std::vector<int>>& grid, GridType gridType, std::pair<int, int> previousCoordinates, std::pair<int, int> newCoordinates, Mouse* pMouse, int* pCheeseEaten)
{

	// Verify that two entities don't enter the same gridTile, if so don't move that tile
	GridType tile = (GridType) grid[newCoordinates.first][newCoordinates.second];
	if (tile != EMPTY && tile != CHEESE )
	{
		// Cat eats the mouse
		if (tile == MOUSE && gridType == CAT) 
		{
			std::pair<int, int> mousePos = pMouse->getCurrentPosition();
			pMouse->spawnNewPosition();
			grid[mousePos.first][mousePos.second] = MOUSE;
		}
	}

	// Cheese is eaten or stepped on
	if (tile == CHEESE)
	{
		// Cat has stepped on the cheese, spawn a new one by setting a flag
		*pCheeseEaten = 1;
	}

	// Set the previous location to empty then set the new coordinate
	grid[previousCoordinates.first][previousCoordinates.second] = EMPTY;
	grid[newCoordinates.first][newCoordinates.second] = gridType;

	return;
}

/**
 *	@brief		Spawns cheese in a new location
 *	@details	Spawns cheese in a new location for when it has been eaten
 *	@param		grid:	Reference to the grid to place the cheese in
 *	@retval		None
 */
void spawnNewCheese(std::vector<std::vector<int>>& grid)
{
	// Casts are used as width and height are expected to be less than
	// INT_MAX

	// Select a random coordinate
	int targetRow = RNGgenRandomInt(0, (int) grid.size());
	int targetCol = RNGgenRandomInt(0, (int) grid[0].size());

	// Check if a location is valid, select new coordinates otherwise
	while (grid[targetRow][targetCol] != EMPTY)
	{
		targetRow = RNGgenRandomInt(0, (int) grid.size());
		targetCol = RNGgenRandomInt(0, (int) grid[0].size());
	}

	// Set the coordinate to cheese
	grid[targetRow][targetCol] = CHEESE;
}
