#include "assignment.hpp"
#include "glm/ext.hpp"
#define CAM_SPEED 0.05f

// ===---------------TRIANGLE-----------------===

Triangle::Triangle()
{
    // allocate the memory to hold the program and shader data
    mProgramHandle = glCreateProgram();
    mVertHandle    = glCreateShader(GL_VERTEX_SHADER);
    mFragHandle    = glCreateShader(GL_FRAGMENT_SHADER);
}

void Triangle::loadShaders()
{
    std::string shaderRoot{ShaderPath};
    vertexSource =
        glx::readShaderSource(shaderRoot + "triangle.vert", IncludeDir);
    fragmentSource =
        glx::readShaderSource(shaderRoot + "triangle.frag", IncludeDir);

    if (auto result{glx::compileShader(vertexSource.sourceString, mVertHandle)};
        result)
    {
        throw OpenGLError(*result);
    }

    if (auto result =
            glx::compileShader(fragmentSource.sourceString, mFragHandle);
        result)
    {
        throw OpenGLError(*result);
    }

    // communicate to OpenGL the shaders used to render the Triangle
    glAttachShader(mProgramHandle, mVertHandle);
    glAttachShader(mProgramHandle, mFragHandle);

    if (auto result = glx::linkShaders(mProgramHandle); result)
    {
        throw OpenGLError(*result);
    }
	setupUniformVariables();
}

void Triangle::loadDataToGPU(std::array<float, 18> const& vertices)
{
    // create buffer to hold triangle vertex data
    glCreateBuffers(1, &mVbo);
    // allocate and initialize buffer to vertex data
    glNamedBufferStorage(
        mVbo, glx::size<float>(vertices.size()), vertices.data(), 0);

    // create holder for all buffers
    glCreateVertexArrays(1, &mVao);
    // bind vertex buffer to the vertex array
    glVertexArrayVertexBuffer(mVao, 0, mVbo, 0, glx::stride<float>(6));

    // enable attributes for the two components of a vertex
    glEnableVertexArrayAttrib(mVao, 0);
    glEnableVertexArrayAttrib(mVao, 1);

    // specify to OpenGL how the vertices and colors are laid out in the buffer
    glVertexArrayAttribFormat(
        mVao, 0, 3, GL_FLOAT, GL_FALSE, glx::relativeOffset<float>(0));
    glVertexArrayAttribFormat(
        mVao, 1, 3, GL_FLOAT, GL_FALSE, glx::relativeOffset<float>(3));

    // associate the vertex attributes (coordinates and color) to the vertex
    // attribute
    glVertexArrayAttribBinding(mVao, 0, 0);
    glVertexArrayAttribBinding(mVao, 1, 0);
}

void Triangle::reloadShaders()
{
    if (glx::shouldShaderBeReloaded(vertexSource))
    {
        glx::reloadShader(
            mProgramHandle, mVertHandle, vertexSource, IncludeDir);
    }

    if (glx::shouldShaderBeReloaded(fragmentSource))
    {
        glx::reloadShader(
            mProgramHandle, mFragHandle, fragmentSource, IncludeDir);
    }
}

void Triangle::render([[maybe_unused]] bool paused,
                      [[maybe_unused]] int width,
                      [[maybe_unused]] int height,
                       Camera cam)
{
    reloadShaders();

	// **************************************
	// assign values to MVP matrices
	// **************************************



	//glm::perspective for pinhole, research other ones
	auto projMat{glm::perspective(glm::radians(60.0f), static_cast<float> (width)/(height), nearVal, farVal)};
	
	// if we wanted to change the camera it would be here
	// giving camera in world space, so if we wanted to move it we would want to move the camera in camera space
	auto viewMat{ glm::lookAt(cam.mEye, cam.mEye + cam.mCentre, cam.mUp) };
	
	if (!paused) {
		// change value of position
		//position = static_cast<float>(glfwGetTime()) * 64.0f;
		if (position >= 360.0f) {
			position = 0.0f;
		}
		position += 2.0f;

	}

	//need 4x4 matrix as first argument
	// M*V*P is the transformation matrix 
	auto modelMat{ glm::rotate(math::Matrix4{1.0f}, glm::radians(position), glm::vec3{0.0f, 1.0f, 0.0f}) }; //use position here!

    // tell OpenGL which program object to use to render the Triangle
    glUseProgram(mProgramHandle);

	// **************************************
	// bind matrices to memory
	// **************************************

	glUniformMatrix4fv(mUniformProjectionLoc, 1, GL_FALSE, glm::value_ptr(projMat)); //do this 3 times, once per uniform variable
	glUniformMatrix4fv(mUniformViewLoc, 1, GL_FALSE, glm::value_ptr(viewMat));
	glUniformMatrix4fv(mUniformModelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));

    // tell OpenGL which vertex array object to use to render the Triangle
    glBindVertexArray(mVao);
    // actually render the Triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Triangle::freeGPUData()
{
    // unwind all the allocations made
    glDeleteVertexArrays(1, &mVao);
    glDeleteBuffers(1, &mVbo);
    glDeleteShader(mFragHandle);
    glDeleteShader(mVertHandle);
    glDeleteProgram(mProgramHandle);
}

void Triangle::setupUniformVariables()
{
	mUniformModelLoc = glGetUniformLocation(mProgramHandle, "model");
	mUniformProjectionLoc = glGetUniformLocation(mProgramHandle, "proj");
	mUniformViewLoc = glGetUniformLocation(mProgramHandle, "view");
	// do this for two more
}

Camera::Camera(glm::vec3 eye, glm::vec3 centre, glm::vec3 up) :
    mEye{eye}, mCentre{centre}, mUp{up}, mYaw{YAW}, mPitch{PITCH}, mSensitivity{SENSITIVITY}
{}


// ===------------IMPLEMENTATIONS-------------===

Program::Program(int width, int height, std::string title, Camera cam) :
    settings{}, callbacks{}, paused{}, mWindow{ nullptr }, mCamera{ cam }, firstMouse{true}, lastX{settings.size.width /2.0f}, lastY{settings.size.width/2.0f}
{
    settings.size.width  = width;
    settings.size.height = height;
    settings.title       = title;

    if (!glx::initializeGLFW(errorCallback))
    {
        throw OpenGLError("Failed to initialize GLFW with error callback");
    }

    mWindow = glx::createGLFWWindow(settings);
    if (mWindow == nullptr)
    {
        throw OpenGLError("Failed to create GLFW Window");
    }

	
	callbacks.keyPressCallback = [&](int key, int, int action, int) {
        std::cout << key << std::endl;
        std::cout << action << std::endl;

        if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
			paused = !paused;
		}
        //https://learnopengl.com/Getting-started/Camera
        if (key == GLFW_KEY_UP && ( action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            mCamera.mEye += CAM_SPEED * mCamera.mCentre;
        }
        if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            mCamera.mEye -= CAM_SPEED * mCamera.mCentre;
        }
        if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            mCamera.mEye -= glm::normalize(glm::cross(mCamera.mCentre, mCamera.mUp)) * CAM_SPEED;
        }
        if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            mCamera.mEye += glm::normalize(glm::cross(mCamera.mCentre, mCamera.mUp)) * CAM_SPEED;
        }
	};


    // as adapted from https://learnopengl.com/Getting-started/Camera
    callbacks.mouseMoveCallback = [&](double xpos, double ypos) {
        if (firstMouse) {
            lastX = (float) xpos;
            lastY = (float) xpos;
            firstMouse = false;
        }

        float xoffset = (float) xpos - lastX;
        float yoffset = lastY - (float) ypos;
        lastX = (float) xpos;
        lastY = (float) ypos;
        xoffset *= SENSITIVITY;
        yoffset *= SENSITIVITY;

        mCamera.mYaw += xoffset;
        mCamera.mPitch += yoffset;

        if (mCamera.mPitch > 89.0f) mCamera.mPitch = 89.0f;
        if (mCamera.mPitch < -89.0f) mCamera.mPitch = -89.0f;

        glm::vec3 direction;
        direction.x = cos(glm::radians(mCamera.mYaw)) * cos(glm::radians(mCamera.mPitch));
        direction.y = sin(glm::radians(mCamera.mPitch));
        direction.z = sin(glm::radians(mCamera.mYaw)) * cos(glm::radians(mCamera.mPitch));
        mCamera.mCentre = glm::normalize(direction);
    };


    createGLContext();
}

void Program::run(Triangle& tri)
{
    glEnable(GL_DEPTH_TEST);

    glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!glfwWindowShouldClose(mWindow))
    {
        int width;
        int height;

        glfwGetFramebufferSize(mWindow, &width, &height);
        // setup the view to be the window's size
        glViewport(0, 0, width, height);
        // tell OpenGL the what color to clear the screen to
        glClearColor(0, 0, 0, 1);
        // actually clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        tri.render(paused, width, height, mCamera);

        glfwSwapBuffers(mWindow);
        glfwPollEvents();

    }
}

void Program::freeGPUData()
{
    glx::destroyGLFWWindow(mWindow);
    glx::terminateGLFW();
}

void Program::createGLContext()
{
    using namespace magic_enum::bitwise_operators;

    glx::bindWindowCallbacks(mWindow, callbacks);
    glfwMakeContextCurrent(mWindow);
    glfwSwapInterval(1);

    if (!glx::createGLContext(mWindow, settings.version))
    {
        throw OpenGLError("Failed to create OpenGL context");
    }

    glx::initializeGLCallback(glx::ErrorSource::All,
                              glx::ErrorType::All,
                              glx::ErrorSeverity::High |
                                  glx::ErrorSeverity::Medium);
}

// ===-----------------DRIVER-----------------===

int main()
{

    Camera cam{ glm::vec3{0.0f, 0.0f, 2.0f}, glm::vec3{0.0f,0.0f,-1.0f}, glm::vec3{0.0f, 1.0f, 0.0f } };

    try
    {

        // clang-format off
        std::array<float, 18> vertices
        {
            // Vertices          Colours
            0.4f, -0.4f, 0.0f,   1.0f, 0.0f, 0.0f,
           -0.4f, -0.4f, 0.0f,   0.0f, 1.0f, 0.0f,
            0.0f,  0.4f, 0.0f,   0.0f, 0.0f, 1.0f
        };
        // clang-format on

        Program prog{1280, 720, "CSC305 Lab 6", cam};
        Triangle tri{};

        tri.loadShaders();
        tri.loadDataToGPU(vertices);

        prog.run(tri);

        prog.freeGPUData();
        tri.freeGPUData();
    }
    catch (OpenGLError& err)
    {
        fmt::print("OpenGL Error:\n\t{}\n", err.what());
    }

    return 0;
}
