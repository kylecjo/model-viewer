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
using Colour = atlas::math::Vector;

static constexpr float nearVal{1.0f};
static constexpr float farVal{10000000000.0f};

static const std::vector<std::string> IncludeDir{ShaderPath};

struct OpenGLError : std::runtime_error
{
    OpenGLError(const std::string& what_arg) : std::runtime_error(what_arg){};
    OpenGLError(const char* what_arg) : std::runtime_error(what_arg){};
};

struct SimpleVertex
{
    math::Point position{};
    math::Normal normal{};
};
// default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SENSITIVITY = 0.05f;

class Light {
public:
    Colour mColour;
    float mRadiance;
};

class Ambient : public Light {
public:
    Ambient(Colour col, float rad);

    Colour mColour;

    float mRadiance;

    Colour L() {
        return mColour * mRadiance;
    }

};

class PointLight : public Light {
public:
    PointLight(atlas::math::Point pos, Colour col, float rad);

    atlas::math::Point mPos;

    Colour mColour;

    float mRadiance;

    Colour L() {
        return mRadiance * mColour;
    }

};


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

class Object
{
public:
    void loadShaders();

    void reloadShaders();

    void freeGPUData();

    virtual void loadDataToGPU() = 0;

    virtual void render(bool paused, int width, int height, Camera cam, glm::vec3 ambient, PointLight pointLight) = 0;

protected:
    void setupUniformVariables(); //called at end of render

    float position;

    // Vertex buffers.
    GLuint mVao;
    GLuint mVbo;
    // Index Buffer
    GLuint mEbo;

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

    // Light
    GLuint mUniformAmbientLoc;
    // Point Light data
    GLuint mUniformPointLightPosLoc;
    GLuint mUniformPointLightColLoc;

    GLuint mUniformColourLoc;

    GLuint mUniformCameraPosLoc;
};


class Mesh : public Object
{
public:
    Mesh(atlas::utils::ObjMesh, Colour colour);
    Colour mColour;
    std::vector<SimpleVertex> mVertices;
    std::vector<GLuint> mIndices;
    void loadDataToGPU();
    void render(bool paused, int width, int height, Camera cam, glm::vec3 ambient, PointLight pointLight);

};



class Cube : public Object
{
public:
    Cube(float length, Colour colour);

    void loadDataToGPU();

    void render(bool paused, int width, int height, Camera cam, glm::vec3 ambient, PointLight pointLight);
private:
    Colour mColour;
    float mLength;
    std::array<float, 18*12> mVertices;

};



class Program
{
public:
    Program(int width, int height, std::string title, Camera cam, glm::vec3 ambient, PointLight pointLight);

    void run(Object& obj, Object& obj2);

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
    bool meshFlag;
    bool firstMouse;
    float lastX;
    float lastY;

    Camera mCamera; 
    
    glm::vec3 mAmbient;
    PointLight mPointLight;
};
