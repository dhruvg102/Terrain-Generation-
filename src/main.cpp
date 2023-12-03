#include <iostream>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <vector>

#include "shader.h"
#include "camera.h"

GLFWwindow* glfw_glad_Setup();

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);


std::vector<float> populate_vertices(int width, int height, int nChannels, unsigned char* data)
{
	// vertex generation
	std::vector<float> vertices;
	float yScale = 64.0f / 256.0f, yShift = 16.0f;  // apply a scale+shift to the height data
	int rez = 1;
	unsigned bytePerPixel = nChannels;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			// retrieve texel for (i,j) tex coord
			unsigned char* pixelOffset = data + (j + width * i) * bytePerPixel;
			// raw height at coordinate
			unsigned char y = pixelOffset[0];

			// vertex
			vertices.push_back(-height / 2.0f + height * i / (float)height);	// vx
			vertices.push_back((int)y * yScale - yShift);						// vy
			vertices.push_back(-width / 2.0f + width * j / (float)width);		// vz
		}
	}
	return vertices;
}

std::vector<unsigned int> index_genderation(int width, int height)
{
	// index generation
	std::vector<unsigned int> indices;
	for (unsigned int i = 0; i < height - 1; i++)       // for each row a.k.a. each strip
	{
		for (unsigned int j = 0; j < width; j++)      // for each column
		{
			for (unsigned int k = 0; k < 2; k++)      // for each side of the strip
			{
				indices.push_back(j + width * (i + k));
			}
		}
	}
	return indices;
}

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
int useWireframe = 0;
int displayGrayscale = 0;

// camera - give pretty starting point
Camera camera(glm::vec3(67.0f, 27.5f, 169.9f),
	glm::vec3(0.0f, 1.0f, 0.0f),
	-128.1f, -42.4f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	GLFWwindow* window = glfw_glad_Setup();

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.0, -1.0);

	// build and compile our shader program
	// ------------------------------------
	Shader heightMapShader("../res/shaders/vs.vert", "../res/shaders/fs.frag");


	// load height map texture
	int width, height, nChannels;
	unsigned char* data = stbi_load("../res/iceland_heightmap.png", &width, &height, &nChannels, 0);

	std::vector<float> vertices = populate_vertices(width, height, nChannels, data);

	stbi_image_free(data);

	std::vector<unsigned int> indices = index_genderation(width, height);

	const unsigned int NUM_STRIPS = height - 1;
	const unsigned int NUM_VERTS_PER_STRIP = width * 2;

	GLuint terrVAO;
	glGenVertexArrays(1, &terrVAO);
	glBindVertexArray(terrVAO);

	GLuint terrVBO;
	glGenBuffers(1, &terrVBO);
	glBindBuffer(GL_ARRAY_BUFFER, terrVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	GLuint terrIBO;
	glGenBuffers(1, &terrIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0], GL_STATIC_DRAW);


	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// be sure to activate shader when setting uniforms/drawing objects
		heightMapShader.use();


		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		heightMapShader.setMat4("projection", projection);
		heightMapShader.setMat4("view", view);

		// world transformation
		glm::mat4 model = glm::mat4(1.0f);
		heightMapShader.setMat4("model", model);

		// render the terrain
		glBindVertexArray(terrVAO);
		int x = GL_FILL;
		if (useWireframe)
		{
			x = GL_LINE;
		}
		glPolygonMode(GL_FRONT_AND_BACK, x);
		for (unsigned strip = 0; strip < NUM_STRIPS; strip++)
		{
			glDrawElements(GL_TRIANGLE_STRIP,   // primitive type
				NUM_VERTS_PER_STRIP + 2,   // number of indices to render
				GL_UNSIGNED_INT,     // index data type
				(void*)(sizeof(unsigned) * (NUM_VERTS_PER_STRIP + 2) * strip)); // offset to starting index
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &terrVAO);
	glDeleteBuffers(1, &terrVBO);
	glDeleteBuffers(1, &terrIBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}


GLFWwindow* glfw_glad_Setup()
{
	// glfw: initialize and configure
  // ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL: Terrain CPU", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return nullptr;
	}

	return window;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever a key event occurs, this callback is called
// ---------------------------------------------------------------------------------------------
void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_SPACE:
			useWireframe = 1 - useWireframe;
			break;
		case GLFW_KEY_G:
			displayGrayscale = 1 - displayGrayscale;
			break;
		default:
			break;
		}
	}
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}