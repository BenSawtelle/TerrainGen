// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <math.h>
#include <time.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

/*
std::vector<glm::vec3> CreateWorldFloor(int width, int length, int height) {
	//temporary assignments
	width = 10;
	length = 10;
	height = 1;

	float xinc = 2.0f / width; //screen size is -1 to 1 (2)
	float yinc = 2.0f / length; //screen size is -1 to 1 (2)
	//int bufferSize = width * length * 3;
	//GLfloat g_vertex_buffer_data[bufferSize];

	std::vector<glm::vec3> g_vertex_buffer_data;
	int index = 0;
	glm::vec3 vertex;

	for (int y = 0; y < length; y++) {
		for (int x = 0; x < width; x++) {
			index = x + y * length;

			float xCoord = x * xinc - 1; //coordinate should be between -1 and 1, not 0 and 2
			float yCoord = y * yinc - 1; //coordinate should be between -1 and 1, not 0 and 2

			vertex.x = float(xCoord);
			vertex.y = float(yCoord);
			vertex.z = float(0);
			g_vertex_buffer_data.push_back(vertex);

		}
	}

	return g_vertex_buffer_data;
}
*/

float ReadBufferZ(int x, int z, int width, std::vector<glm::vec3> &buffer) {
	int index = (x & (width - 1)) + (z & (width - 1)) * width;
	return buffer[index].y;
}

void WriteBufferZ(float value, int x, int z, int width, std::vector<glm::vec3> &buffer) {
	int index = (x & (width - 1)) + (z & (width - 1)) * width;
	buffer[index].y = value;
}

std::vector<glm::vec3> CreateWorldFloor(int width, int length, int height) {
	

	
	//temporary assignments
	width = 1000;
	length = width; //must be a square (will probably change to overlay the height map on a smaller non square region)
	height = 500;
	

	//for heightmap algorithm, height and width must be 2^n + 1;
	float widthCheck = log10(width) / log10(2.0);
	float lengthCheck = log10(length) / log10(2.0);

	width = pow(2, ceil(widthCheck));
	height = pow(2, ceil(lengthCheck));

	std::cout << "width: " << width << "   height: " << height << '\n';


	float xinc = 2.0f / width; //screen size is -1 to 1 (2 "width" range)
	float zinc = 2.0f / length; //screen size is -1 to 1 (2)
	float yinc = 2.0f / height;
								//int bufferSize = width * length * 3;
								//GLfloat g_vertex_buffer_data[bufferSize];

	
	//heightmap
	std::vector<glm::vec3> g_vertex_buffer_data;
	int index = 0;
	glm::vec3 vertex;

	float x = 0;
	float z = 0;

	while (z < length) {
		if (index % 2 == 0) {
			vertex.x = x * xinc - 1;
			vertex.z = z * zinc - 1;
			g_vertex_buffer_data.push_back(vertex);
		}

		else if (index % 2 == 1) {
			vertex.x = x * xinc - 1;
			vertex.z = (z+1) * zinc - 1;
			x++;
			g_vertex_buffer_data.push_back(vertex);
		}

		if (x >= width) {
			//g_vertex_buffer_data.push_back(GL_PRIMITIVE_RESTART_INDEX);
			z++;
			x = 0;
		}
		index++;
	}





	srand(time(NULL));

	//corner heights and coordinates
	float seedA, seedB, seedC, seedD;
	float heightScale = 1.0;

	//start heightmap algorithm (diamond square)
	//seed corners
	seedA = (rand() % height) * yinc - 1;
	seedB = (rand() % height) * yinc - 1;
	seedC = (rand() % height) * yinc - 1;
	seedD = (rand() % height) * yinc - 1;

	int stepSize = width / 2;
	int halfStep = stepSize / 2;

	g_vertex_buffer_data[0 + (0 * width)].y = seedA;
	g_vertex_buffer_data[stepSize + (0 * width)].y = seedB;
	g_vertex_buffer_data[0 + (stepSize * width)].y= seedC;
	g_vertex_buffer_data[stepSize + (stepSize * width)].y = seedD;

	do {
		int maxRandHeight = int(ceil(heightScale * stepSize));
		int meanRandHeight = heightScale * stepSize / 2;

		for (int z = halfStep; z < width; z += stepSize) {
			for (int x = halfStep; x < width; x += stepSize) {
				float a = ReadBufferZ(x - halfStep, z - halfStep, width, g_vertex_buffer_data);
				float b = ReadBufferZ(x + halfStep, z - halfStep, width, g_vertex_buffer_data);
				float c = ReadBufferZ(x - halfStep, z + halfStep, width, g_vertex_buffer_data);
				float d = ReadBufferZ(x + halfStep, z + halfStep, width, g_vertex_buffer_data);



				float variance = ((rand() % maxRandHeight) - meanRandHeight) * yinc;

				float average = (a + b + c + d) / 4 + variance;
				WriteBufferZ(average, x, z, width, g_vertex_buffer_data);
			}
		}
		
		for (int z = 0; z < width; z += stepSize) {
			for (int x = halfStep; x < width; x += stepSize) {
				float a = ReadBufferZ(x - halfStep, z, width, g_vertex_buffer_data);
				float b = ReadBufferZ(x + halfStep, z, width, g_vertex_buffer_data);
				float c = ReadBufferZ(x, z + halfStep, width, g_vertex_buffer_data);
				float d = ReadBufferZ(x, z - halfStep, width, g_vertex_buffer_data);

				float e = ReadBufferZ(x - stepSize, z + halfStep, width, g_vertex_buffer_data);
				float f = ReadBufferZ(x - halfStep, z + stepSize, width, g_vertex_buffer_data);

				float variance = ((rand() % maxRandHeight) - meanRandHeight) * yinc;
				float average = (a + b + c + d) / 4 + variance;
				WriteBufferZ(average, x, z, width, g_vertex_buffer_data);

				float variance2 = ((rand() % maxRandHeight) - meanRandHeight) * yinc;

				average = (a + b + c + d) / 4 + variance2;
				WriteBufferZ(average, x - halfStep, z + halfStep, width, g_vertex_buffer_data);

			}
		}
		
		stepSize /= 2;
		halfStep = stepSize / 2;

	} while (stepSize > 1);
	
	return g_vertex_buffer_data;
}



/*
void IndexVBO(std::vector<glm::vec3> &  vertices, std::vector<glm::vec3> & indices, std::vector<glm::vec3> & indexed_vertices) {

	for vertex 
		if vertex matches other vertex
			add index value
		else vertex is unique
			


	for (int i = 0; i < vertices.size; i++) {
		for (int k = 0; k < i; k++) {

		}
	}

}
*/



int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Tutorial 09 - VBO Indexing", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSetWindowPos(window, 400, 300);


	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	GLuint ScaleMatrixID = glGetUniformLocation(programID, "S");

	// Load the texture
	GLuint Texture = loadDDS("uvmap.DDS");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	std::vector<glm::vec3> vertices = CreateWorldFloor(0, 0, 0);
	
	//glm::mat4 scaleVector = glm::scale(glm::vec3(10, 10, 1));


	// Read our .obj file
	//std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	//bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	//std::vector<glm::vec2> indexed_uvs;
	//std::vector<glm::vec3> indexed_normals;
	//indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);



	//IndexVBP(vertices, indices, indexed_vertices);




	// Load it into a VBO

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	//GLuint uvbuffer;
	//glGenBuffers(1, &uvbuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	//glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	//GLuint normalbuffer;
	//glGenBuffers(1, &normalbuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	//glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	//GLuint elementbuffer;
	//glGenBuffers(1, &elementbuffer);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	do {

		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
											 // printf and reset
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		// Bind our texture in Texture Unit 0
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		//glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		/*
		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		
		// Index buffer
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,      // mode
			vertices.size(),    // count
			GL_UNSIGNED_SHORT,   // type
			(void*)0           // element array buffer offset
		);

		*/

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_PROGRAM_POINT_SIZE);


		glDrawArrays(GL_POINTS, 0, vertices.size() * sizeof(glm::vec3));
		glDisableVertexAttribArray(0);
		//glDisableVertexAttribArray(1);
		//glDisableVertexAttribArray(2);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	//glDeleteBuffers(1, &uvbuffer);
	//glDeleteBuffers(1, &normalbuffer);
	//glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);
	//glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

