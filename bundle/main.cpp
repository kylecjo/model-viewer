#include "assignment.hpp"
#include "glm/ext.hpp"
#include <atlas/utils/LoadObjFile.hpp>

#define CAM_SPEED 0.2f

// ===---------------OBJECT-----------------===

void Object::loadShaders()
{
    std::string shaderRoot{ ShaderPath };
    vertexSource =
        glx::readShaderSource(shaderRoot + "triangle.vert", IncludeDir);
    fragmentSource =
        glx::readShaderSource(shaderRoot + "triangle.frag", IncludeDir);

    if (auto result{ glx::compileShader(vertexSource.sourceString, mVertHandle) };
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

void Object::reloadShaders()
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

void Object::freeGPUData()
{
    // unwind all the allocations made
    glDeleteVertexArrays(1, &mVao);
    glDeleteBuffers(1, &mVbo);
    glDeleteBuffers(1, &mEbo);
    glDeleteShader(mFragHandle);
    glDeleteShader(mVertHandle);
    glDeleteProgram(mProgramHandle);
}

void Object::setupUniformVariables()
{
    mUniformModelLoc = glGetUniformLocation(mProgramHandle, "model");
    mUniformProjectionLoc = glGetUniformLocation(mProgramHandle, "proj");
    mUniformViewLoc = glGetUniformLocation(mProgramHandle, "view");
    mUniformColourLoc = glGetUniformLocation(mProgramHandle, "colour");
    mUniformAmbientLoc = glGetUniformLocation(mProgramHandle, "ambient");
    mUniformPointLightPosLoc = glGetUniformLocation(mProgramHandle, "pointLightPos");
    mUniformPointLightColLoc = glGetUniformLocation(mProgramHandle, "pointLightCol");
    mUniformDirectionalColLoc = glGetUniformLocation(mProgramHandle, "directionalCol");
    mUniformDirectionalDirLoc = glGetUniformLocation(mProgramHandle, "directionalDir");
    mUniformCameraPosLoc = glGetUniformLocation(mProgramHandle, "cameraPos");
    mUniformSpecularFlagLoc = glGetUniformLocation(mProgramHandle, "specularFlag");
    mUniformDirectionalFlagLoc = glGetUniformLocation(mProgramHandle, "directionalFlag");
}

// ===---------------MESH-----------------===

Mesh::Mesh(atlas::utils::ObjMesh mesh, Colour colour) :
     mColour{colour}
{
    mProgramHandle = glCreateProgram();
    mVertHandle = glCreateShader(GL_VERTEX_SHADER);
    mFragHandle = glCreateShader(GL_FRAGMENT_SHADER);
    
    std::vector<SimpleVertex> sVertices;
    std::vector<GLuint> sIndices;

    for (atlas::utils::Vertex v : mesh.shapes[0].vertices) {
        SimpleVertex sV;
        sV.position = v.position;
        sV.normal = v.normal;
        sVertices.push_back(sV);
    }
    for (size_t i : mesh.shapes[0].indices) {
        sIndices.push_back((GLuint)i);
    }
    mVertices = sVertices;
    mIndices = sIndices;
}


void Mesh::loadDataToGPU() {

    // create holder for all buffers
    glCreateVertexArrays(1, &mVao);

    // create buffer to hold triangle vertex data
    glCreateBuffers(1, &mVbo);
    // allocate and initialize buffer to vertex data
    glNamedBufferStorage(
        mVbo, mVertices.size() *  sizeof(SimpleVertex), mVertices.data(), 0);

    glCreateBuffers(1, &mEbo);
    glNamedBufferStorage(
        mEbo, mIndices.size() * sizeof(GLuint), mIndices.data(), 0);


    // bind vertex buffer to the vertex array
    glVertexArrayVertexBuffer(mVao, 0, mVbo, 0, glx::stride<float>(6));
    // bind element buffer to vertex array
    glVertexArrayElementBuffer(mVao, mEbo);

    // enable attributes for the two components of a vertex
    glEnableVertexArrayAttrib(mVao, 0);
    glEnableVertexArrayAttrib(mVao, 1);

    // specify to OpenGL how the vertices and colors are laid out in the buffer
    glVertexArrayAttribFormat(
        mVao, 0, 3, GL_FLOAT, GL_FALSE, glx::relativeOffset<float>(0));
    glVertexArrayAttribFormat(
        mVao, 1, 3, GL_FLOAT, GL_FALSE, glx::relativeOffset<float>(3));

    // associate the vertex attributes (coordinates and normals) to the vertex
    // attribute
    glVertexArrayAttribBinding(mVao, 0, 0);
    glVertexArrayAttribBinding(mVao, 1, 0);


}

void Mesh::render([[maybe_unused]] bool paused,
    [[maybe_unused]] int width,
    [[maybe_unused]] int height,
    Camera cam, glm::vec3 ambient, PointLight pointLight, Directional directional, bool specularFlag, bool directionalFlag)
{
    reloadShaders();

    // **************************************
    // assign values to MVP matrices
    // **************************************

    //glm::perspective for pinhole, research other ones
    auto projMat{ glm::perspective(glm::radians(60.0f), static_cast<float> (width) / (height), nearVal, farVal) };

    // if we wanted to change the camera it would be here
    // giving camera in world space, so if we wanted to move it we would want to move the camera in camera space
    auto viewMat{ glm::lookAt(cam.mEye, cam.mEye + cam.mCentre, cam.mUp) };

    //need 4x4 matrix as first argument
    // M*V*P is the transformation matrix 
    auto modelMat{ math::Matrix4{1.0f}}; 

    // tell OpenGL which program object to use to render the Triangle
    glUseProgram(mProgramHandle);

    // **************************************
    // bind matrices to memory
    // **************************************

    glUniformMatrix4fv(mUniformProjectionLoc, 1, GL_FALSE, glm::value_ptr(projMat)); //do this 3 times, once per uniform variable
    glUniformMatrix4fv(mUniformViewLoc, 1, GL_FALSE, glm::value_ptr(viewMat));
    glUniformMatrix4fv(mUniformModelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniform3fv(mUniformColourLoc, 1, glm::value_ptr(mColour));
    glUniform3fv(mUniformAmbientLoc, 1, glm::value_ptr(ambient));
    glUniform3fv(mUniformPointLightPosLoc, 1, glm::value_ptr(pointLight.mPos));
    glUniform3fv(mUniformPointLightColLoc, 1, glm::value_ptr(pointLight.L()));
    glUniform3fv(mUniformCameraPosLoc, 1, glm::value_ptr(cam.mEye));
    glUniform3fv(mUniformDirectionalDirLoc, 1, glm::value_ptr(directional.mDir));
    glUniform3fv(mUniformDirectionalColLoc, 1, glm::value_ptr(directional.L()));
    glUniform1i(mUniformSpecularFlagLoc, (GLint)specularFlag);
    glUniform1i(mUniformDirectionalFlagLoc, (GLint)directionalFlag);


    // tell OpenGL which vertex array object to use to render the Triangle
    glBindVertexArray(mVao);
    // actually render the Triangle
    int size; 
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    glDrawElements(GL_TRIANGLES, size/sizeof(GLuint), GL_UNSIGNED_INT, 0 ); //(mode, starting index in enabled arrays, number of vertexes to be rendered)

}



// ===---------------CUBE-----------------===

Cube::Cube(float length, Colour colour)
{
    // allocate the memory to hold the program and shader data
    mProgramHandle = glCreateProgram();
    mVertHandle = glCreateShader(GL_VERTEX_SHADER);
    mFragHandle = glCreateShader(GL_FRAGMENT_SHADER);

    mColour = colour;
    mLength = length;

    std::array<float, 18*12> vertices{ //12 triangles each take 18 bytes
        // Vertices                     /Normals
        -mLength, -mLength, -mLength,   0.0f, 0.0f, -1.0f,
        mLength, -mLength, -mLength,    0.0f, 0.0f, -1.0f,
        mLength, mLength, -mLength,     0.0f, 0.0f, -1.0f,

  
        mLength, mLength, -mLength,     0.0f, 0.0f, -1.0f,
        -mLength, mLength, -mLength,    0.0f, 0.0f, -1.0f,
        -mLength, -mLength, -mLength,   0.0f, 0.0f, -1.0f,

        -mLength, -mLength, mLength,    0.0f, 0.0f, 1.0f,
        mLength, -mLength, mLength,     0.0f, 0.0f, 1.0f,
        mLength, mLength, mLength,      0.0f, 0.0f, 1.0f,

        mLength, mLength, mLength,      0.0f, 0.0f, 1.0f,
        -mLength, mLength, mLength,     0.0f, 0.0f, 1.0f,
        -mLength, -mLength, mLength,    0.0f, 0.0f, 1.0f,

        -mLength, mLength, mLength,     -1.0f, 0.0f, 0.0f,
       -mLength, mLength, -mLength,     -1.0f, 0.0f, 0.0f,
        -mLength, -mLength, -mLength,   -1.0f, 0.0f, 0.0f,

        -mLength, -mLength, -mLength,   -1.0f, 0.0f, 0.0f,
       -mLength, -mLength, mLength,     -1.0f, 0.0f, 0.0f,
        -mLength, mLength, mLength,     -1.0f, 0.0f, 0.0f,

        mLength, mLength, mLength,      1.0f, 0.0f, 0.0f,
        mLength, mLength, -mLength,     1.0f, 0.0f, 0.0f,
        mLength, -mLength, -mLength,    1.0f, 0.0f, 0.0f,

        mLength, -mLength, -mLength,    1.0f, 0.0f, 0.0f,
        mLength, -mLength, mLength,     1.0f, 0.0f, 0.0f,
        mLength, mLength, mLength,      1.0f, 0.0f, 0.0f,

        -mLength,-mLength, -mLength,    -1.0f, 0.0f, 0.0f,
       mLength, -mLength, -mLength,     -1.0f, 0.0f, 0.0f,
        mLength, -mLength, mLength,     -1.0f, 0.0f, 0.0f,

        mLength,-mLength, mLength,      -1.0f, 0.0f, 0.0f,
       -mLength, -mLength, mLength,     -1.0f, 0.0f, 0.0f,
        -mLength, -mLength, -mLength,   -1.0f, 0.0f, 0.0f,
    
        -mLength, mLength, -mLength,    0.0f, 1.0f, 0.0f,
        mLength, mLength, -mLength,     0.0f, 1.0f, 0.0f,
        mLength, mLength, mLength,      0.0f, 1.0f, 0.0f,

        mLength, mLength, mLength,      0.0f, 1.0f, 0.0f,
        -mLength, mLength, mLength,     0.0f, 1.0f, 0.0f,
        -mLength, mLength, -mLength,    0.0f, 1.0f, 0.0f
    };
    // clang-format on

    

    mVertices = vertices;
}



void Cube::loadDataToGPU()
{
    // create buffer to hold triangle vertex data
    glCreateBuffers(1, &mVbo);
    // allocate and initialize buffer to vertex data
    glNamedBufferStorage(
        mVbo, glx::size<float>(mVertices.size()), mVertices.data(), 0);

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

    // associate the vertex attributes (coordinates and normals) to the vertex
    // attribute
    glVertexArrayAttribBinding(mVao, 0, 0);
    glVertexArrayAttribBinding(mVao, 1, 0);
}



void Cube::render([[maybe_unused]] bool paused,
    [[maybe_unused]] int width,
    [[maybe_unused]] int height,
    Camera cam, glm::vec3 ambient, PointLight pointLight, Directional directional, bool specularFlag, bool directionalFlag)
{
    reloadShaders();

    // **************************************
    // assign values to MVP matrices
    // **************************************



    //glm::perspective for pinhole, research other ones
    auto projMat{ glm::perspective(glm::radians(60.0f), static_cast<float> (width) / (height), nearVal, farVal) };

    // if we wanted to change the camera it would be here
    // giving camera in world space, so if we wanted to move it we would want to move the camera in camera space
    auto viewMat{ glm::lookAt(cam.mEye, cam.mEye + cam.mCentre, cam.mUp) };

    //if (!paused) {
    //    // change value of position
    //    //position = static_cast<float>(glfwGetTime()) * 64.0f;
    //    if (position >= 360.0f) {
    //        position = 0.0f;
    //    }
    //    position += 1.0f;

    //}

    //need 4x4 matrix as first argument
    // M*V*P is the transformation matrix 
    auto modelMat{ glm::rotate(math::Matrix4{1.0f}, glm::radians(0.0f), glm::vec3{0.0f, 1.0f, 0.0f}) }; //use position here!

    // tell OpenGL which program object to use to render the Triangle
    glUseProgram(mProgramHandle);

    // **************************************
    // bind matrices to memory
    // **************************************

    glUniformMatrix4fv(mUniformProjectionLoc, 1, GL_FALSE, glm::value_ptr(projMat)); //do this 3 times, once per uniform variable
    glUniformMatrix4fv(mUniformViewLoc, 1, GL_FALSE, glm::value_ptr(viewMat));
    glUniformMatrix4fv(mUniformModelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniform3fv(mUniformColourLoc, 1, glm::value_ptr(mColour));
    glUniform3fv(mUniformAmbientLoc, 1, glm::value_ptr(ambient));
    glUniform3fv(mUniformPointLightPosLoc, 1, glm::value_ptr(pointLight.mPos));
    glUniform3fv(mUniformPointLightColLoc, 1, glm::value_ptr(pointLight.L()));
    glUniform3fv(mUniformCameraPosLoc, 1, glm::value_ptr(cam.mEye));
    glUniform3fv(mUniformDirectionalDirLoc, 1, glm::value_ptr(directional.mDir));
    glUniform3fv(mUniformDirectionalColLoc, 1, glm::value_ptr(directional.L()));
    glUniform1i(mUniformSpecularFlagLoc, (GLint) specularFlag);
    glUniform1i(mUniformDirectionalFlagLoc, (GLint) directionalFlag);


    // tell OpenGL which vertex array object to use to render the Triangle
    glBindVertexArray(mVao);
    // actually render the Triangle
    glDrawArrays(GL_TRIANGLES, 0, 3*12); //(mode, starting index in enabled arrays, number of vertexes to be rendered)
    glBindVertexArray(0);
}

// ===---------------CAMERA-----------------===


Camera::Camera(glm::vec3 eye, glm::vec3 centre, glm::vec3 up) :
    mEye{eye}, mCentre{centre}, mUp{up}, mYaw{YAW}, mPitch{PITCH}, mSensitivity{SENSITIVITY}
{}

// ===---------------LIGHTS-----------------===

Ambient::Ambient(Colour col, float rad) :
    mColour{ col }, mRadiance{ rad }
{}

PointLight::PointLight(atlas::math::Point pos, Colour col, float rad) :
    mPos{ pos }, mColour{ col }, mRadiance{ rad }
{}


Directional::Directional(atlas::math::Vector dir, Colour col, float rad) :
    mDir{dir}, mColour{col}, mRadiance{rad}
{}



// ===------------IMPLEMENTATIONS-------------===

Program::Program(int width, int height, std::string title, Camera cam, glm::vec3 ambient, PointLight pointLight, Directional directional) :
    settings{}, callbacks{}, paused{}, mWindow{ nullptr }, mCamera{ cam }, mAmbient{ambient}, mPointLight{pointLight}, mDirectional{directional},
    firstMouse{ true }, lastX{ settings.size.width / 2.0f }, lastY{ settings.size.width / 2.0f }, mSpecularFlag{}, mDirectionalFlag{}, meshFlag{}
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


        if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
			mSpecularFlag = !mSpecularFlag;
		}
        if (key == GLFW_KEY_M && action == GLFW_RELEASE) {
            meshFlag = !meshFlag;
        }
        if (key == GLFW_KEY_L && action == GLFW_RELEASE) {
            mDirectionalFlag = !mDirectionalFlag;
        }
        //https://learnopengl.com/Getting-started/Camera
        if (key == GLFW_KEY_W && ( action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            mCamera.mEye += CAM_SPEED * mCamera.mCentre;
        }
        if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            mCamera.mEye -= CAM_SPEED * mCamera.mCentre;
        }
        if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
        {
            mCamera.mEye -= glm::normalize(glm::cross(mCamera.mCentre, mCamera.mUp)) * CAM_SPEED;
        }
        if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
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

void Program::run(Object& obj, Object& obj2)
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

        if (!meshFlag) {
            obj.render(paused, width, height, mCamera, mAmbient, mPointLight, mDirectional, mSpecularFlag, mDirectionalFlag);
        }
        else {
            obj2.render(paused, width, height, mCamera, mAmbient, mPointLight, mDirectional, mSpecularFlag, mDirectionalFlag);

        }

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

    Camera cam{ glm::vec3{0.0f, 0.0f, 3.0f}, glm::vec3{0.0f,0.0f, -1.0f}, glm::vec3{0.0f, 1.0f, 0.0f } };

    try
    {

        Ambient a{ Colour{1.0f, 1.0f, 1.0f}, 0.1f };
        glm::vec3 ambient = a.L();

        PointLight p{ atlas::math::Point{0.0f, 0.0f, 2.0f}, Colour{1.0f, 1.0f, 1.0f }, 0.9f };
        Directional d{ atlas::math::Vector{1.0f, 0.0f, 0.0f} , Colour{ 1.0f, 1.0f, 1.0f }, 0.9f };

        Program prog{1280, 720, "CSC305 Assignment 3", cam, ambient, p, d};
        
        std::string shaderRoot{ ShaderPath };

        atlas::utils::ObjMesh m = atlas::utils::loadObjMesh(shaderRoot + "suzanne.obj").value();
        Mesh mesh{ m, Colour {0.2f, 0.8f, 0.0f} };
        mesh.loadShaders();
        mesh.loadDataToGPU();

        Cube cube{ 1.0f, Colour{1.0f, 0.647f, 0.0f} };
        cube.loadShaders();
        cube.loadDataToGPU();
        
        prog.run(cube, mesh);
        prog.freeGPUData();
        cube.freeGPUData();
        mesh.freeGPUData();
        
    }
    catch (OpenGLError& err)
    {
        fmt::print("OpenGL Error:\n\t{}\n", err.what());
    }

    return 0;
}
