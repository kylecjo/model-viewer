#pragma once

#include "paths.hpp"

#include <exception>
#include <iostream>
#include <string>

#include <atlas/glx/Buffer.hpp>
#include <atlas/glx/Context.hpp>
#include <atlas/glx/ErrorCallback.hpp>
#include <atlas/glx/GLSL.hpp>
#include <atlas/utils/Cameras.hpp>
#include <atlas/utils/LoadObjFile.hpp>

#include <fmt/printf.h>
#include <magic_enum.hpp>

using namespace atlas;

static constexpr float nearVal{1.0f};
static constexpr float farVal{10000000000.0f};

static const std::vector<std::string> IncludeDir{ShaderPath};

struct OpenGLError : std::runtime_error
{
    OpenGLError(const std::string& what_arg) : std::runtime_error(what_arg){};
    OpenGLError(const char* what_arg) : std::runtime_error(what_arg){};
};

// default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SENSITIVITY = 0.1f;


class Camera
{
public:
    Camera(glm::vec3 eye, glm::vec3 centre, glm::vec3 up);
    glm::vec3 mEye;
    glm::vec3 mCentre;
    glm::vec3 mUp;
    glm::vec3 mRight;
    float mYaw;
    float mPitch;
    float mSensitivity;

private:

};

class Triangle
{
public:
    Triangle();

    void loadShaders();

    void loadDataToGPU(std::array<float, 18> const& vertices);

    void reloadShaders();

    void render(bool paused, int width, int height, Camera cam);

    void freeGPUData();

private:
    void setupUniformVariables(); //called at end of render

    float position;

    // Vertex buffers.
    GLuint mVao;
    GLuint mVbo;

    // Shader data.
    GLuint mVertHandle;
    GLuint mFragHandle;
    GLuint mProgramHandle;
    glx::ShaderFile vertexSource;
    glx::ShaderFile fragmentSource;

    // Uniform variable data.
    GLuint mUniformModelLoc;
    GLuint mUniformViewLoc;
    GLuint mUniformProjectionLoc;
};

class Program
{
public:
    Program(int width, int height, std::string title, Camera cam);

    void run(Triangle& object);

    void freeGPUData();

private:
    static void errorCallback(int code, char const* message)
    {
        fmt::print("error ({}): {}\n", code, message);
    }

    void createGLContext();

    GLFWwindow* mWindow;
    glx::WindowSettings settings;
    glx::WindowCallbacks callbacks;

    bool paused;
    bool firstMouse;
    float lastX;
    float lastY;

    Camera mCamera; 
};
