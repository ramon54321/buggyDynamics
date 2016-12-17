#ifndef __graphics3d__
#define __graphics3d__



GLFWwindow* windowHandle;
int i; /* Simple iterator */
GLuint vao, vbo[2]; /* Create handles for our Vertex Array Object and two Vertex Buffer Objects */
int IsCompiled_VS, IsCompiled_FS;
int IsLinked;
int maxLength;
char *vertexInfoLog;
char *fragmentInfoLog;
char *shaderProgramInfoLog;

glm::mat4 projection;
float* projectionMatrixBuffer;

/* These pointers will receive the contents of our shader source code files */
GLchar *vertexsource, *fragmentsource;

/* These are handles used to reference the shaders */
GLuint vertexshader, fragmentshader;

/* This is a handle to the shader program */
GLuint shaderprogram;

/* A simple function that will read a file into an allocated char pointer buffer */
char* filetobuf(char *file)
{
    FILE *fptr;
    long length;
    char *buf;

    fptr = fopen(file, "rb"); /* Open file for reading */
    if (!fptr) /* Return NULL on failure */
        return NULL;
    fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
    length = ftell(fptr); /* Find out how many bytes into the file we are */
    buf = (char*)malloc(length+1); /* Allocate a buffer for the entire length of the file and a null terminator */
    fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
    fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
    fclose(fptr); /* Close the file */
    buf[length] = 0; /* Null terminator */

    return buf; /* Return the buffer */
}

int initGraphics(){

	if(!glfwInit()){
		std::cout << "Glfw init failed." << std::endl;
	} else {
		std::cout << "Glfw init successful." << std::endl;
	}


	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Init matricies
	projection = glm::perspective(glm::pi<float>() * 0.25f, 1.0f, 0.1f, 500.0f);
	projectionMatrixBuffer = glm::value_ptr(projection);


	// Create window
	windowHandle = glfwCreateWindow(1024, 1024, "Proxy Engine", NULL, NULL);
	if(!windowHandle){
		std::cout << "Failed to create window." << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(windowHandle);

	glfwSwapInterval(0);

	if(glewInit() != GLEW_OK){
		std::cout << "Failed to init GLEW." << std::endl;
		glfwTerminate();
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	

	// SHADERS

	/* Read our shaders into the appropriate buffers */
	vertexsource = filetobuf("vertex.shader");
	fragmentsource = filetobuf("fragment.shader");
	//std::cout << vertexsource << std::endl;
	//std::cout << fragmentsource << std::endl;

	/* Create an empty vertex shader handle */
	vertexshader = glCreateShader(GL_VERTEX_SHADER);

	/* Send the vertex shader source code to GL */
	/* Note that the source code is NULL character terminated. */
	/* GL will automatically detect that therefore the length info can be 0 in this case (the last parameter) */
	glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);

	/* Compile the vertex shader */
	glCompileShader(vertexshader);

	glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &IsCompiled_VS);
	if(IsCompiled_VS == GL_FALSE)
	{
	   glGetShaderiv(vertexshader, GL_INFO_LOG_LENGTH, &maxLength);

	   /* The maxLength includes the NULL character */
	   vertexInfoLog = (char *)malloc(maxLength);

	   glGetShaderInfoLog(vertexshader, maxLength, &maxLength, vertexInfoLog);

	   /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
	   /* In this simple program, we'll just leave */
	   free(vertexInfoLog);
	   std::cout << "Er 1" << std::endl;
	   return -3;
	}

	/* Create an empty fragment shader handle */
	fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);

	/* Send the fragment shader source code to GL */
	/* Note that the source code is NULL character terminated. */
	/* GL will automatically detect that therefore the length info can be 0 in this case (the last parameter) */
	glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);

	/* Compile the fragment shader */
	glCompileShader(fragmentshader);

	glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &IsCompiled_FS);
	if(IsCompiled_FS == GL_FALSE)
	{
	   glGetShaderiv(fragmentshader, GL_INFO_LOG_LENGTH, &maxLength);

	   /* The maxLength includes the NULL character */
	   fragmentInfoLog = (char *)malloc(maxLength);

	   glGetShaderInfoLog(fragmentshader, maxLength, &maxLength, fragmentInfoLog);

	   /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
	   /* In this simple program, we'll just leave */
	   //free(fragmentInfoLog);

	   for(int i = 0; i < maxLength; i++){
		   std::cout << fragmentInfoLog[i];
	   }

	   std::cout << "Er 2" << std::endl;
	   return -3;
	}

	/* If we reached this point it means the vertex and fragment shaders compiled and are syntax error free. */
	/* We must link them together to make a GL shader program */
	/* GL shader programs are monolithic. It is a single piece made of 1 vertex shader and 1 fragment shader. */
	/* Assign our program handle a "name" */
	shaderprogram = glCreateProgram();

	/* Attach our shaders to our program */
	glAttachShader(shaderprogram, vertexshader);
	glAttachShader(shaderprogram, fragmentshader);

	/* Bind attribute index 0 (coordinates) to in_Position and attribute index 1 (color) to in_Color */
	/* Attribute locations must be setup before calling glLinkProgram. */
	glBindAttribLocation(shaderprogram, 0, "in_Position");
	glBindAttribLocation(shaderprogram, 1, "in_Color");

	/* Link our program */
	/* At this stage, the vertex and fragment programs are inspected, optimized and a binary code is generated for the shader. */
	/* The binary code is uploaded to the GPU, if there is no error. */
	glLinkProgram(shaderprogram);

	/* Again, we must check and make sure that it linked. If it fails, it would mean either there is a mismatch between the vertex */
	/* and fragment shaders. It might be that you have surpassed your GPU's abilities. Perhaps too many ALU operations or */
	/* too many texel fetch instructions or too many interpolators or dynamic loops. */

	glGetProgramiv(shaderprogram, GL_LINK_STATUS, (int *)&IsLinked);
	if(IsLinked == GL_FALSE)
	{
	   /* Noticed that glGetProgramiv is used to get the length for a shader program, not glGetShaderiv. */
	   glGetProgramiv(shaderprogram, GL_INFO_LOG_LENGTH, &maxLength);

	   /* The maxLength includes the NULL character */
	   shaderProgramInfoLog = (char *)malloc(maxLength);

	   /* Notice that glGetProgramInfoLog, not glGetShaderInfoLog. */
	   glGetProgramInfoLog(shaderprogram, maxLength, &maxLength, shaderProgramInfoLog);

	   /* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
	   /* In this simple program, we'll just leave */
	   free(shaderProgramInfoLog);
	   std::cout << "Er 3" << std::endl;
	   return -3;
	}
}

float cameraPositionX = 0;
float cameraPositionZ = 25;
float cameraRotZ = 0;

glm::mat4 applyCameraTransform(glm::mat4 transform){

	
	transform = glm::translate(transform, glm::vec3(-cameraPositionX, -5, -cameraPositionZ));
	transform = glm::rotate(transform, cameraRotZ , glm::vec3(0, 1, 0));

	return transform;
}

void render_nullbox(int drawMethod, float x, float y, float z, float s, const float* quat){

	GLuint vaox, vbox[2];

	/* We're going to create a simple diamond made from lines */
	const GLfloat diamond[8][3] = {
	{  1.0,  1.0, 1.0  }, //0
	{  1.0,  1.0, -1.0  },
	{  1.0, -1.0, 1.0  }, //2
	{  1.0,  -1.0, -1.0  },
	{  -1.0,  1.0, 1.0  },//4
	{  -1.0,  1.0, -1.0  },
	{  -1.0, -1.0, 1.0  },//6
	{  -1.0,  -1.0, -1.0  } };

	const GLfloat colors[8][3] = {
	{  1.0,  0.0,  0.0  },
	{  0.0,  1.0,  0.0  },
	{  0.0,  0.0,  1.0  },
	{  1.0,  1.0,  1.0  },
	{  1.0,  0.0,  0.0  },
	{  0.0,  1.0,  0.0  },
	{  0.0,  0.0,  1.0  },
	{  1.0,  1.0,  1.0  } };


	// Cube indexs
	GLuint indexsCube[6 * 6] = { 5, 7, 3, 3, 1, 5,
								4, 6, 7, 7, 5, 4,
								4, 1, 5, 4, 0, 1,
								6, 2, 3, 3, 7, 6,
								6, 2, 0, 0, 4, 6,
								0, 2, 3, 3, 1, 0 };

	/* Allocate and assign a Vertex Array Object to our handle */
	glGenVertexArrays(1, &vaox);

	/* Bind our Vertex Array Object as the current used object */
	glBindVertexArray(vaox);

	/* Allocate and assign two Vertex Buffer Objects to our handle */
	glGenBuffers(2, vbox);

	/* Bind our first VBO as being the active buffer and storing vertex attributes (coordinates) */
	glBindBuffer(GL_ARRAY_BUFFER, vbox[0]);

	/* Copy the vertex data from diamond to our buffer */
	/* 8 * sizeof(GLfloat) is the size of the diamond array, since it contains 8 GLfloat values */
	glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(GLfloat), diamond, GL_STATIC_DRAW);

	/* Specify that our coordinate data is going into attribute index 0, and contains two floats per vertex */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Enable attribute index 0 as being used */
	glEnableVertexAttribArray(0);

	/* Bind our second VBO as being the active buffer and storing vertex attributes (colors) */
	glBindBuffer(GL_ARRAY_BUFFER, vbox[1]);

	/* Copy the color data from colors to our buffer */
	/* 12 * sizeof(GLfloat) is the size of the colors array, since it contains 12 GLfloat values */
	glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(GLfloat), colors, GL_STATIC_DRAW);

	/* Specify that our color data is going into attribute index 1, and contains three floats per vertex */
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Enable attribute index 1 as being used */
	glEnableVertexAttribArray(1);

	// Matricies

	glm::mat4 transform = glm::mat4(1.0f);
	// Camera
	transform = applyCameraTransform(transform);

	// Model
	glm::quat rotQuat = glm::quat(quat[0], quat[1], quat[2], quat[3]);
	glm::mat4 rotationMatrix = glm::toMat4(rotQuat);

	transform = glm::translate(transform, glm::vec3(x, y, z));
	transform = glm::scale(transform, glm::vec3(s / 2, s / 2, s / 2));
	transform *= rotationMatrix;

	


	/*
	glm::mat4 transform = glm::mat4(1.0f);
	// Camera
	transform = applyCameraTransform(transform);

	// Model
	glm::quat rotQuat = glm::quat(quat[0], quat[1], quat[2], quat[3]);
	glm::mat4 rotationMatrix = glm::toMat4(rotQuat);

	transform = glm::translate(transform, glm::vec3(x, y, z));
	transform = glm::scale(transform, glm::vec3(s / 2, s / 2, s / 2));
	transform *= rotationMatrix;
	*/

	float* transformMatrixBuffer = glm::value_ptr(transform);

	GLuint projLoc = glGetUniformLocation(shaderprogram, "projectionMatrix");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, projectionMatrixBuffer);

	GLuint transLoc = glGetUniformLocation(shaderprogram, "transformMatrix");
	glUniformMatrix4fv(transLoc, 1, GL_FALSE, transformMatrixBuffer);

	GLuint indexVbo;
	/* Allocate and assign two Vertex Buffer Objects to our handle */
	glGenBuffers(1, &indexVbo);

	/* Bind our first VBO as being the active buffer and storing vertex attributes (coordinates) */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVbo);

	/* Copy the vertex data from diamond to our buffer */
	/* 8 * sizeof(GLfloat) is the size of the diamond array, since it contains 8 GLfloat values */
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * 6 * sizeof(GLuint), &indexsCube[0], GL_STATIC_DRAW);

	/* Invoke glDrawArrays telling that our data is a line loop and we want to draw 2-4 vertexes */
	if(drawMethod == DRAW_METHOD_TRIANGLES){
		glDrawElements(GL_TRIANGLES, 6 * 6, GL_UNSIGNED_INT, (void*)0);
		return;
	}

	if(drawMethod == DRAW_METHOD_WIRES){
		glDrawElements(GL_LINE_STRIP, 6 * 6, GL_UNSIGNED_INT, (void*)0);
		return;
	}

	if(drawMethod == DRAW_METHOD_POINTS){
		glDrawElements(GL_POINTS, 6 * 6, GL_UNSIGNED_INT, (void*)0);
		return;
	}
}


void render_arrow(float* v, float* p){
	
	// MESH
	GLuint vaox, vbox[2];
	const GLfloat diamond[2][3] = {
	{  p[0],  p[1],  p[2]  }, //0
	{  v[0] + p[0],  v[1] + p[1], v[2] + p[2]  } };

	const GLfloat colors[2][3] = {
	{  1.0,  1.0,  0.0  },
	{  1.0,  1.0,  0.0  } };

	// indexs
	GLuint indexsArrow[2] = {0, 1};

	/* Allocate and assign a Vertex Array Object to our handle */
	glGenVertexArrays(1, &vaox);

	/* Bind our Vertex Array Object as the current used object */
	glBindVertexArray(vaox);

	/* Allocate and assign two Vertex Buffer Objects to our handle */
	glGenBuffers(2, vbox);

	/* Bind our first VBO as being the active buffer and storing vertex attributes (coordinates) */
	glBindBuffer(GL_ARRAY_BUFFER, vbox[0]);

	/* Copy the vertex data from diamond to our buffer */
	/* 8 * sizeof(GLfloat) is the size of the diamond array, since it contains 8 GLfloat values */
	glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(GLfloat), diamond, GL_STATIC_DRAW);

	/* Specify that our coordinate data is going into attribute index 0, and contains two floats per vertex */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Enable attribute index 0 as being used */
	glEnableVertexAttribArray(0);

	/* Bind our second VBO as being the active buffer and storing vertex attributes (colors) */
	glBindBuffer(GL_ARRAY_BUFFER, vbox[1]);

	/* Copy the color data from colors to our buffer */
	/* 12 * sizeof(GLfloat) is the size of the colors array, since it contains 12 GLfloat values */
	glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(GLfloat), colors, GL_STATIC_DRAW);

	/* Specify that our color data is going into attribute index 1, and contains three floats per vertex */
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Enable attribute index 1 as being used */
	glEnableVertexAttribArray(1);

	// RENDER

	// Matricies
	glm::mat4 transform = glm::mat4(1.0f);

	// Camera
	transform = applyCameraTransform(transform);

	// Model
	transform = glm::translate(transform, glm::vec3(0, 0, 0));
	transform = glm::scale(transform, glm::vec3(1, 1, 1));
	float* transformMatrixBuffer = glm::value_ptr(transform);

	GLuint projLoc = glGetUniformLocation(shaderprogram, "projectionMatrix");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, projectionMatrixBuffer);

	GLuint transLoc = glGetUniformLocation(shaderprogram, "transformMatrix");
	glUniformMatrix4fv(transLoc, 1, GL_FALSE, transformMatrixBuffer);


	GLuint indexVbox;
	/* Allocate and assign two Vertex Buffer Objects to our handle */
	glGenBuffers(1, &indexVbox);

	/* Bind our first VBO as being the active buffer and storing vertex attributes (coordinates) */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVbox);

	/* Copy the vertex data from diamond to our buffer */
	/* 8 * sizeof(GLfloat) is the size of the diamond array, since it contains 8 GLfloat values */
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * sizeof(GLuint), &indexsArrow[0], GL_STATIC_DRAW);

	/* Invoke glDrawArrays telling that our data is a line loop and we want to draw 2-4 vertexes */
	glLineWidth(3);
	glDrawElements(GL_LINE_STRIP, 2, GL_UNSIGNED_INT, (void*)0);
}

void render_plane(){

	GLuint vaox, vbox[2];

	/* We're going to create a simple diamond made from lines */
	const GLfloat diamond[8][3] = {
	{  1.0,  1.0, 1.0  }, //0
	{  1.0,  1.0, -1.0  },
	{  1.0, -1.0, 1.0  }, //2
	{  1.0,  -1.0, -1.0  },
	{  -1.0,  1.0, 1.0  },//4
	{  -1.0,  1.0, -1.0  },
	{  -1.0, -1.0, 1.0  },//6
	{  -1.0,  -1.0, -1.0  } };

	const GLfloat colors[8][3] = {
	{  1.0,  0.0,  0.0  },
	{  0.0,  1.0,  0.0  },
	{  0.0,  0.0,  1.0  },
	{  1.0,  1.0,  1.0  },
	{  1.0,  0.0,  0.0  },
	{  0.0,  1.0,  0.0  },
	{  0.0,  0.0,  1.0  },
	{  1.0,  1.0,  1.0  } };


	// Cube indexs
	GLuint indexsCube[6 * 6] = { //5, 7, 3, 3, 1, 5,
								//4, 6, 7, 7, 5, 4,
								//4, 1, 5, 4, 0, 1,
								6, 2, 3, 3, 7, 6
								//6, 2, 0, 0, 4, 6,
								//0, 2, 3, 3, 1, 0 
								};

	/* Allocate and assign a Vertex Array Object to our handle */
	glGenVertexArrays(1, &vaox);

	/* Bind our Vertex Array Object as the current used object */
	glBindVertexArray(vaox);

	/* Allocate and assign two Vertex Buffer Objects to our handle */
	glGenBuffers(2, vbox);

	/* Bind our first VBO as being the active buffer and storing vertex attributes (coordinates) */
	glBindBuffer(GL_ARRAY_BUFFER, vbox[0]);

	/* Copy the vertex data from diamond to our buffer */
	/* 8 * sizeof(GLfloat) is the size of the diamond array, since it contains 8 GLfloat values */
	glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(GLfloat), diamond, GL_STATIC_DRAW);

	/* Specify that our coordinate data is going into attribute index 0, and contains two floats per vertex */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Enable attribute index 0 as being used */
	glEnableVertexAttribArray(0);

	/* Bind our second VBO as being the active buffer and storing vertex attributes (colors) */
	glBindBuffer(GL_ARRAY_BUFFER, vbox[1]);

	/* Copy the color data from colors to our buffer */
	/* 12 * sizeof(GLfloat) is the size of the colors array, since it contains 12 GLfloat values */
	glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(GLfloat), colors, GL_STATIC_DRAW);

	/* Specify that our color data is going into attribute index 1, and contains three floats per vertex */
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Enable attribute index 1 as being used */
	glEnableVertexAttribArray(1);

	// Matricies

	glm::mat4 transform = glm::mat4(1.0f);

	// Camera
	transform = applyCameraTransform(transform);

	// Model
	transform = glm::translate(transform, glm::vec3(0, 0, 0));
	transform = glm::scale(transform, glm::vec3(10, 0, 10));

	float* transformMatrixBuffer = glm::value_ptr(transform);

	GLuint projLoc = glGetUniformLocation(shaderprogram, "projectionMatrix");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, projectionMatrixBuffer);

	GLuint transLoc = glGetUniformLocation(shaderprogram, "transformMatrix");
	glUniformMatrix4fv(transLoc, 1, GL_FALSE, transformMatrixBuffer);

	GLuint indexVbo;
	/* Allocate and assign two Vertex Buffer Objects to our handle */
	glGenBuffers(1, &indexVbo);

	/* Bind our first VBO as being the active buffer and storing vertex attributes (coordinates) */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVbo);

	/* Copy the vertex data from diamond to our buffer */
	/* 8 * sizeof(GLfloat) is the size of the diamond array, since it contains 8 GLfloat values */
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * 6 * sizeof(GLuint), &indexsCube[0], GL_STATIC_DRAW);

	/* Invoke glDrawArrays telling that our data is a line loop and we want to draw 2-4 vertexes */
	glDrawElements(GL_TRIANGLES, 6 * 6, GL_UNSIGNED_INT, (void*)0);
}


#endif // __grahics3d__
