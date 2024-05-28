
//Third Party Libraries
#include "include/glad/glad.h"
#include <SDL.h>

//c++ libs (STL)
#include <iostream>
#include <vector>
#include <fstream>
#include <string>


#undef main
using namespace std;

//Globals (generally are prefixed with 'g' in this application)

//Screen Dimensions (Window)
int gScreenWidth  = 640;
int gScreenHeight = 480;
SDL_Window*   gGraphicsApplicationWindow = nullptr;
SDL_GLContext gOpenGLContext			 = nullptr;

//Main loop flag
bool gQuit = false; // if true, we quit


// VAO (VertexArrayObject)
GLuint gVertexArrayObject = 0;

//VBO (VertexBufferObject)
GLuint gVertexBufferObject = 0;

// Shader
// Program Object (for our shaders)
// Stores the unique id for the graphics pipeline
// Used for OpenGL draw calls.
GLuint gGraphicsPipelineShaderProgram = 0;

/*
//***************************Shader Define*****************************************
// Vertex Shader
// Vertex shader executes once per vertex, and will be in charge of
// the final position of the vertex.
const std::string gVertexShaderSource =
"#version 410 core\n"
"in vec4 position;\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(position.x, position.y, position.z, position.w);\n"
"}\n";

// Fragment Shader
// The fragment shader execures once per fragment(i.e roughly for every pixel that will be rasterized),
// and in part determines the final color that will be sent to the screen.
const std::string gFragmentShaderSource =
"#version 410 core\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"	color = vec4(1.0f, 0.5f, 0.0f, 1.0f);\n"//the oronge color on triangle, comes from here
"}\n";
//*********************************************************************************
*/

std::string LoadShaderAsString(const std::string& filename)
{
	//Resulting shader program loaded as a single string
	std::string result = "";

	std::string line = "";
	std::ifstream myFile(filename.c_str());

	if (myFile.is_open())
	{
		while (std::getline(myFile, line))
		{
			result += line + "\n";
		}
		myFile.close();
	}

	return result;
}

GLuint CompileShader(GLuint type, const std::string& source)// where the actual compilation happen
{
	GLuint shaderObject;

	//Create the shader
	if (type == GL_VERTEX_SHADER)
	{
		shaderObject = glCreateShader(GL_VERTEX_SHADER);
	}
	else if (type == GL_FRAGMENT_SHADER)
	{
		shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	}

	const char* src = source.c_str();
	glShaderSource(shaderObject, 1, &src, nullptr);// set up the source code 
	glCompileShader(shaderObject);// and compile it

	//Retrieve the result of our compilation
	int result;
	//The goal with glGetShaderiv is to retrieve the compilation status
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		char* errorMessage = new char[length];//Could also use alloca here.
		glGetShaderInfoLog(shaderObject, length, &length, errorMessage);

		if (type == GL_VERTEX_SHADER)
		{
			cout << "ERROR: GL_VERTEX_SHADER compilation failed!\n" << errorMessage << endl;
		}
		else if (type == GL_FRAGMENT_SHADER)
		{
			cout << "ERROR: GL_FRAGMENT_SHADER compilation failed!\n" << errorMessage << endl;
		}

		//Reclaim our memory
		delete[] errorMessage;

		//Delete our broken shader
		glDeleteShader(shaderObject);

		return 0;
	}

	

	return shaderObject;
}

GLuint CreateShaderProgram(const std::string& vertexshadersource, 
						 const std::string& fragmentshadersource)
{
	//Create a new program object
	GLuint programObject = glCreateProgram();//GL Create Graphics Pipeline

	//Compile our shaders
	GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexshadersource);
	GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentshadersource);

	//link our two shader programs together
	//consider this the equivalent of taking two .cpp files, and linking them into
	//one executeable file
	glAttachShader(programObject, myVertexShader);
	glAttachShader(programObject, myFragmentShader);
	glLinkProgram(programObject); // compile multiple files togather

	//Validate our program
	glValidateProgram(programObject);//error checking state

	// glDetachShader , glDeleteShader

	return programObject;
}


void CreateGraphicsPipeline()
{
	std::string vertexShaderSource   = LoadShaderAsString("./shader/vertexshader.glsl");
	std::string fragmentShaderSource = LoadShaderAsString("./shader/fragmentshader.glsl");

	gGraphicsPipelineShaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
}

//Some Info's
void GetOpenGLVersionInfo()
{
	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Version: " << glGetString(GL_VERSION) << endl;
	cout << "Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

void VertexSpecification()
{
	//Lives on the CPU
	const std::vector<GLfloat>vertexData{
		//x      y     z
		-0.8f, -0.8f, 0.0f,// VERTEX 1 (left vertex position)
		1.0f, 0.0f, 0.0f, //color
		0.8f, -0.8f, 0.0f,//  VERTEX 2 (right vertex position)
		0.0f, 1.0f, 0.0f, //color
		0.0f, 0.8f, 0.0f,//   VERTEX 3 (top vertex position)
		0.0f, 0.0f, 1.0f, //color
	};

	//Start setting things up on the GPU
	glGenVertexArrays(1, &gVertexArrayObject);//layout for work with
	glBindVertexArray(gVertexArrayObject);//start kind a working with it

	// Start generating our **VBO**
	glGenBuffers(1, &gVertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);  //Bind is equivalent to 'Selecting the active' buffer object
	// 'vertexPositions' (which is on the CPU), onto a buffer that will on the GPU
	glBufferData(GL_ARRAY_BUFFER,
		vertexData.size() * sizeof(GL_FLOAT),
		vertexData.data(),
		GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); //x,y,z positions hold not just position datas, hold colors, texture coordinates, normal, etc. in OpenGL
	glVertexAttribPointer(0,//attribure 0 corresponds to the enabled glEnableVertexAttribArray
					      3,//Number of components
						  GL_FLOAT,//type
						  GL_FALSE,//Is the data normalized
						  sizeof(GL_FLOAT) * 6,//Stride(actual stride is 6)(jump)
						  (void*)0);//Offset

	//Now linking up the attributes in our VAO
	//Color information
	glEnableVertexAttribArray(1); 
	glVertexAttribPointer(1,
						  3,//r,g,b
						  GL_FLOAT,
						  GL_FALSE,
						  sizeof(GL_FLOAT) * 6,//hoping(jump)
						  (void*)(sizeof(GL_FLOAT)*3));

	glBindVertexArray(0);// Unselecting vertex array
	//glDisableVertexAttribArray(0);
}

void InitalizeProgram()
{
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		cout << "SDL2 could not initialize video subsystem" << endl;
		exit(1);
	}

	//Setup the OpenGL Context
	//Use OpenGL 4.1 core or greater
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);//using OpenGL 4.1 (Available for mac, windows and linux)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);//NOTE: Mac unfortunatelly doesn't support beyond version 4.1 (can be try version 3.3)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	//we want to request a double buffer for smooth updating
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	//Creating an application window using OpenGL that supports SDL
	gGraphicsApplicationWindow = SDL_CreateWindow("OpenGL First Program", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED
								, gScreenWidth, gScreenHeight
								, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN);

	//check if Window did not create
	if (gGraphicsApplicationWindow == nullptr)
	{
		cout << "SDL_Window was not able to be created" << endl;

		exit(1);
	}

	// Create an OpenGL Graphics Context
	gOpenGLContext = SDL_GL_CreateContext(gGraphicsApplicationWindow);
	if (gOpenGLContext == nullptr)
	{
		cout << "OpenGL context not available\n";
		exit(1);
	}

	//Initialize GLAD Library
	if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
	{
		cout << "glad was not initialized" << endl;
		exit(1);
	}

	GetOpenGLVersionInfo();
}

void Input()
{
	SDL_Event e;//Handles various events in SDL that are related to input and output

	while (SDL_PollEvent(&e) != 0)//Handle events on queue
	{
		// If the users post an event to quit
		// An example is hitting the "x" in the corner of the window.
		if (e.type == SDL_QUIT) 
		{
			cout << "Goodbye!" << endl;
			gQuit = true;
		}
	}
}

void PreDraw()//Responsible for setting OpenGL state
{
	// Disable depth test and face culling
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// Initialize clear color
	// This is the background of the screen
	glViewport(0, 0, gScreenWidth, gScreenHeight);
	glClearColor(1.f, 1.f, 0.f, 1.f);

	// Clear color buffer and Depth Buffer
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);// Change background color

	// Use our shader
	glUseProgram(gGraphicsPipelineShaderProgram);
}

void Draw()
{
	//In order to draw we have got to figure out which 
	//vertex array object are we going to be using

	// Enable our Attributes
	glBindVertexArray(gVertexArrayObject);

	// Select the vertex buffer object we want to enable
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);

	//Render Data
	glDrawArrays(GL_TRIANGLES, 0, 3);

	//Stop using our current graphics pipeline
	//Note: This is not necessary if we only have one graphics pipeline.
	glUseProgram(0);
}

void MainLoop()
{
	while (!gQuit)
	{
		Input();//Handle input

		PreDraw();//Setup anything(i.e. OpenGL State) that needs to take place before draw calls

		Draw();//Draw Calls in OpenGL

		// Update the screen of specified window
		SDL_GL_SwapWindow(gGraphicsApplicationWindow);
	}
}

void CleanUp()
{
	SDL_DestroyWindow(gGraphicsApplicationWindow);
	SDL_Quit();
}

int main(int argc, char*args[]) {
	//1. Sets ups SDL Window and OpenGL (Graphics Program set up)
	InitalizeProgram();

	//2. Setup our geometry
	VertexSpecification();

	//3. Create our graphics pipeline
	// - At a minimum, this means the vertex and fragment shader
	CreateGraphicsPipeline();

	//4. Call the main application loop
	MainLoop();

	//5. Call the cleanup function when our program terminates
	CleanUp();

	return 0;
}
