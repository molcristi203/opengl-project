#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>
#include "btBulletDynamicsCommon.h"
#include "Object3D.hpp"
#include "Player.hpp"

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
glm::vec3 lightPos;
glm::vec3 spotLightPos;
glm::vec3 spotLightDir;
GLfloat cutoff;
GLfloat cutoff2;
glm::vec3 spotLightPos2;
glm::vec3 spotLightDir2;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLint lightPosLoc;
GLint cameraPosLoc;
GLint flatShadingLoc;
GLint spotLightPosLoc;
GLint spotLightDirLoc;
GLint cutOffLoc;
GLint spotLightPosLoc2;
GLint spotLightDirLoc2;
GLint cutOffLoc2;

// camera
gps::Camera* myCamera;

gps::Camera* animationCamera;

gps::Camera* currentCamera;

GLboolean cameraAnimation = false;

GLfloat cameraSpeed = 17.0f;

GLboolean pressedKeys[1024];

// models
gps::Model3D teapot;
GLfloat angle;

Object3D groundObj;
Object3D teapotObj;
Object3D ground2Obj;
Object3D target1;
Object3D bench1;
Object3D target2;
Object3D target3;
Object3D m4;
Object3D tent1;
Object3D tent2;
Object3D ural;
Object3D ak;
Object3D bench2;
Object3D target2_2;
Object3D target3_2;
Object3D tunel;
Object3D cub;
Object3D target1_2;
Object3D target4;
Object3D target5;
Object3D target6;
Object3D pistol;
Object3D sniper;

// shaders
gps::Shader myBasicShader;

GLfloat currentFrame = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat delta = 0.0f;

GLfloat yaw = -90.0f;
GLfloat pitch = 0.0f;

GLboolean mouseFirst = true;
GLfloat lastX;
GLfloat lastY;
GLfloat sensitivity = 0.1f;
GLfloat fov = 45.0f;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;
gps::Shader depthMapShader;
GLint lightSpaceMatrixLoc;
GLint lightSpaceMatrixLoc2;
GLint shadowModelLoc;

const unsigned int SHADOW_WIDTH = 4096;
const unsigned int SHADOW_HEIGHT = 4096;

const GLfloat near_plane = 0.1f, far_plane = 20.0f;

btDefaultCollisionConfiguration* colissionConfiguration;
btCollisionDispatcher* dispatcher;
btBroadphaseInterface* overlappingPairCache;
btSequentialImpulseConstraintSolver* solver;
btDiscreteDynamicsWorld* dynamicsWorld;

Player* player;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void computeDelta() {
    currentFrame = glfwGetTime();
    delta = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (cameraAnimation)
    {
        return;
    }

    if (mouseFirst) {
        mouseFirst = false;
        lastX = xpos;
        lastY = ypos;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    pitch = std::min(pitch, 89.0f);
    pitch = std::max(pitch, -89.0f);

    currentCamera->rotate(pitch, yaw);

    view = currentCamera->getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniform3fv(cameraPosLoc, 1, glm::value_ptr(currentCamera->getPosition()));
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (cameraAnimation)
    {
        return;
    }

    if (fov >= 1.0f && fov <= 45.0f) {
        fov -= yoffset;
    }
    if (fov <= 1.0f) {
        fov = 1.0f;
    }

    if (fov >= 45.0f) {
        fov = 45.0f;
    }
}

void processMovement() {
    if (cameraAnimation)
    {
        return;
    }

	if (pressedKeys[GLFW_KEY_W]) {
        
        glm::vec3 frontDirection = currentCamera->getFrontDirection();

        btVector3 direction = btVector3(frontDirection.x, frontDirection.y, frontDirection.z);
        direction.normalize();

        player->move(direction * cameraSpeed);

		//update view matrix
        view = currentCamera->getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
        glUniform3fv(cameraPosLoc, 1, glm::value_ptr(currentCamera->getPosition()));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		//currentCamera->move(gps::MOVE_BACKWARD, cameraSpeed * delta);
    
        glm::vec3 frontDirection = -currentCamera->getFrontDirection();

        btVector3 direction = btVector3(frontDirection.x, frontDirection.y, frontDirection.z);
        direction.normalize();

        player->move(direction * cameraSpeed);

        //update view matrix
        view = currentCamera->getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
        glUniform3fv(cameraPosLoc, 1, glm::value_ptr(currentCamera->getPosition()));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		//currentCamera->move(gps::MOVE_LEFT, cameraSpeed * delta);

        glm::vec3 rightDirection = currentCamera->getRightDirection();

        btVector3 direction = btVector3(rightDirection.x, rightDirection.y, rightDirection.z);
        direction.normalize();

        player->move(direction * cameraSpeed);

        //update view matrix
        view = currentCamera->getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
        glUniform3fv(cameraPosLoc, 1, glm::value_ptr(currentCamera->getPosition()));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		//currentCamera->move(gps::MOVE_RIGHT, cameraSpeed * delta);

        glm::vec3 rightDirection = -currentCamera->getRightDirection();

        btVector3 direction = btVector3(rightDirection.x, rightDirection.y, rightDirection.z);
        direction.normalize();

        player->move(direction * cameraSpeed);

        //update view matrix
        view = currentCamera->getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
        glUniform3fv(cameraPosLoc, 1, glm::value_ptr(currentCamera->getPosition()));
	}

    if (pressedKeys[GLFW_KEY_T]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (pressedKeys[GLFW_KEY_R]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    if (pressedKeys[GLFW_KEY_Y]) {
        myBasicShader.useShaderProgram();
        glUniform1i(flatShadingLoc, 0);
    }

    if (pressedKeys[GLFW_KEY_U]) {
        myBasicShader.useShaderProgram();
        glUniform1i(flatShadingLoc, 1);
    }

    if (pressedKeys[GLFW_KEY_I]) {
        btVector3 pos =  player->getPosition();
        std::cout << pos.x() << " " << pos.y() << " " << pos.z() << std::endl;
    }

    if (pressedKeys[GLFW_KEY_P]) 
    {
        animationCamera->setPosition(glm::vec3(0.0f, 0.5f, 52.0f));
        currentCamera = animationCamera;
        cameraAnimation = true;
    }
    
}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetScrollCallback(myWindow.getWindow(), scrollCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initModels() {
    teapotObj.Load("models/teapot/teapot20segUT.obj", "", 1.0f, glm::vec3(-9.24f, 1.0f, 40.62f), glm::vec3(0, 0, 0), BOX_COLLIDER, false);
    dynamicsWorld->addRigidBody(teapotObj.GetRigidBody());

    ground2Obj.Load("models/ground2/Ground.obj", "", 0.0f, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0, 0, 0), MESH_COLLIDER, false);
    dynamicsWorld->addRigidBody(ground2Obj.GetRigidBody());

    target1.Load("models/targets/target1.obj", "", 0.0f, glm::vec3(7.5f, -0.1f, -20.0f), glm::vec3(glm::pi<float>(), 0, 0), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(target1.GetRigidBody());

    bench1.Load("models/targets/bench1.obj", "", 0, glm::vec3(7.0f, -0.65f, 22.0f), glm::vec3(0, 0, 0), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(bench1.GetRigidBody());

    target2.Load("models/targets/target2.obj", "", 0.0f, glm::vec3(-5.0f, -0.4f, -20.0f), glm::vec3(glm::pi<float>(), 0, 0), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(target2.GetRigidBody());

    target3.Load("models/targets/target3.obj", "", 0.0f, glm::vec3(-5.0f, 0.1f, -19.8f), glm::vec3(0, 0, 0), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(target3.GetRigidBody());

    m4.Load("models/m4/classicm4.obj", "", 0.0f, glm::vec3(7.0f, -0.23f, 22.0f), glm::vec3(0, 0, glm::radians(90.0f)), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(m4.GetRigidBody());

    tent1.Load("models/tent/tent.obj", "", 0.0f, glm::vec3(15.0f, -.99, 35.0f), glm::vec3(glm::radians(180.0f), 0.0f, 0.0f), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(tent1.GetRigidBody());

    tent2.Load("models/tent/tent.obj", "", 0.0f, glm::vec3(5.0f, -0.99f, 35.0f), glm::vec3(glm::radians(180.0f), 0.0f, 0.0f), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(tent2.GetRigidBody());

    ural.Load("models/ural/ural.obj", "", 0.0f, glm::vec3(-5.0f, -1.05f, 35.0f), glm::vec3(glm::radians(145.0f), 0.0f, 0.0f), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(ural.GetRigidBody());

    ak.Load("models/ak47/ak47.obj", "", 0.0f, glm::vec3(0.0f, -0.25f, 22.0f), glm::vec3(glm::radians(90.0f), glm::radians(90.0f), 0), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(ak.GetRigidBody());

    bench2.Load("models/targets/bench1.obj", "", 0, glm::vec3(0.0f, -0.65f, 22.0f), glm::vec3(0, 0, 0), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(bench2.GetRigidBody());
    
    target2_2.Load("models/targets/target2.obj", "", 0.0f, glm::vec3(5.0f, -0.4f, -20.0f), glm::vec3(glm::pi<float>(), 0, 0), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(target2_2.GetRigidBody());

    target3_2.Load("models/targets/target3.obj", "", 0.0f, glm::vec3(5.0f, 0.1f, -19.8f), glm::vec3(0, 0, 0), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(target3_2.GetRigidBody());

    tunel.Load("models/tunel/tunel.obj", "", 0.0f, glm::vec3(0, -1, 54), glm::vec3(), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(tunel.GetRigidBody());

    cub.Load("models/cub/cub.obj", "", 0.0f, glm::vec3(0.0f, -1.0f, 54.0f), glm::vec3(), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(cub.GetRigidBody());

    target1_2.Load("models/targets/target1.obj", "", 0.0f, glm::vec3(-7.5f, -0.1f, -20.0f), glm::vec3(glm::pi<float>(), 0, 0), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(target1_2.GetRigidBody());

    target4.Load("models/targets/target4.obj", "", 0.0f, glm::vec3(0, -1, -39), glm::vec3(), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(target4.GetRigidBody());

    target5.Load("models/targets/target5.obj", "", 0.0f, glm::vec3(14, -1, -39), glm::vec3(glm::radians(180.0f), 0, 0), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(target4.GetRigidBody());

    target6.Load("models/targets/target6.obj", "", 0.0f, glm::vec3(-14, -1, -39), glm::vec3(glm::radians(180.0f), 0, 0), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(target4.GetRigidBody());

    pistol.Load("models/pistol/pistol.obj", "", 0.0f, glm::vec3(7.4f, -0.23f, 22.4f), glm::vec3(0, glm::radians(90.0f), 0), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(pistol.GetRigidBody());

    sniper.Load("models/sniper/sniper.obj", "", 0.0f, glm::vec3(0.65f, -0.6f, 22.2f), glm::vec3(glm::radians(-90.0f), glm::radians(-75.0f), glm::radians(90.0f)), BOX_COLLIDER, true);
    dynamicsWorld->addRigidBody(sniper.GetRigidBody());
}

void initShaders() {
	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");

    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();

    depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
}

void initSkyBox() {
    std::vector<const GLchar*> faces;

    faces.push_back("skybox/Lycksele/posx.jpg");
    faces.push_back("skybox/Lycksele/negx.jpg");
    faces.push_back("skybox/Lycksele/posy.jpg");
    faces.push_back("skybox/Lycksele/negy.jpg");
    faces.push_back("skybox/Lycksele/posz.jpg");
    faces.push_back("skybox/Lycksele/negz.jpg");

    mySkyBox.Load(faces);
}

void initUniforms() {
	myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = currentCamera->getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(fov),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 200.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 0.3f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    lightPos = glm::vec3(15.0f, 0.5f, 35.0f);
    lightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightPos");
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

    cameraPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "cameraPos");
    glUniform3fv(cameraPosLoc, 1, glm::value_ptr(currentCamera->getPosition()));

    flatShadingLoc = glGetUniformLocation(myBasicShader.shaderProgram, "flatShading");
    glUniform1i(flatShadingLoc, 0);

    lightSpaceMatrixLoc = glGetUniformLocation(depthMapShader.shaderProgram, "fragPosLightSpace");
    lightSpaceMatrixLoc2 = glGetUniformLocation(myBasicShader.shaderProgram, "fragPosLightSpace");
    shadowModelLoc = glGetUniformLocation(depthMapShader.shaderProgram, "model");

    spotLightPos = glm::vec3(-3.5f, 0.3f, 31.2f);
    spotLightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "spotLightPos");
    glUniform3fv(spotLightPosLoc, 1, glm::value_ptr(spotLightPos));
    glCheckError();

    spotLightDir = glm::vec3(0.5f, -0.2, -1);
    spotLightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "spotLightDir");
    glUniform3fv(spotLightDirLoc, 1, glm::value_ptr(spotLightDir));
    glCheckError();

    spotLightPos2 = glm::vec3(-2.1f, 0.3f, 32.3f);
    spotLightPosLoc2 = glGetUniformLocation(myBasicShader.shaderProgram, "spotLightPos2");
    glUniform3fv(spotLightPosLoc2, 1, glm::value_ptr(spotLightPos2));
    glCheckError();

    spotLightDir2 = glm::vec3(0.5f, -0.2, -1);
    spotLightDirLoc2 = glGetUniformLocation(myBasicShader.shaderProgram, "spotLightDir2");
    glUniform3fv(spotLightDirLoc2, 1, glm::value_ptr(spotLightDir2));
    glCheckError();

    cutoff = glm::radians(50.0f);
    cutOffLoc = glGetUniformLocation(myBasicShader.shaderProgram, "cutOff");
    glUniform1f(cutOffLoc, cutoff);
    glCheckError();

    cutoff2 = glm::radians(50.0f);
    cutOffLoc2 = glGetUniformLocation(myBasicShader.shaderProgram, "cutOff2");
    glUniform1f(cutOffLoc2, cutoff2);
    glCheckError();
}

void drawObjects(gps::Shader shader, bool depthPass, GLint modelLoc) 
{
    teapotObj.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    glCheckError();
    ground2Obj.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    glCheckError();
    target1.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    glCheckError();
    bench1.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    glCheckError();
    target2.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    glCheckError();
    target3.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    glCheckError();
    m4.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    tent1.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    tent2.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    ural.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    ak.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    bench2.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    target2_2.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    target3_2.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    tunel.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    target1_2.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    target4.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    target5.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    target6.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    pistol.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    sniper.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
    cub.Render(shader, depthPass, modelLoc, normalMatrixLoc, view);
}

void initFBO() {
    glGenFramebuffers(1, &shadowMapFBO);
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
        0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void animateObjectLocation(Object3D& object, const std::vector<btVector3> keyframes)
{
    //std::cout << (object.GetPosition() - keyframes.at(object.currentKeyframe)).length2() << std::endl;

    object.GetRigidBody()->activate();

    if ((object.GetPosition() - keyframes.at(object.currentPositionKeyframe)).length2() < 0.2)
    {
        object.currentPositionKeyframe = (static_cast<unsigned long long>(object.currentPositionKeyframe) + 1) % keyframes.size();
    }
    else
    {
        object.SetPosition(lerp(object.GetPosition(), keyframes.at(object.currentPositionKeyframe), 0.02));
    }
}

void animateObjectRotation(Object3D& object, const std::vector<btQuaternion> keyframes)
{

    object.GetRigidBody()->activate();

    //std::cout << object.GetRotation().angle(keyframes.at(object.currentRotationKeyframe)) << std::endl;

    if (keyframes.at(object.currentRotationKeyframe).angle(object.GetRotation()) < 0.001f)
    {
        object.currentRotationKeyframe = (object.currentRotationKeyframe + 1) % keyframes.size();
    }
    else
    {
        object.SetRotation(object.GetRotation().slerp(keyframes.at(object.currentRotationKeyframe), 0.1));
    }
}

glm::mat4 computeLightSpaceTrMatrix() {
    glm::mat4 lightView = glm::lookAt(glm::vec3(0, 5, 51), glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat near_plane = 0.1f, far_plane = 100.0f;
    glm::mat4 lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

    return lightSpaceTrMatrix;
}

void initBullet() {
    colissionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(colissionConfiguration);
    overlappingPairCache = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();

    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, colissionConfiguration);

    dynamicsWorld->setGravity(btVector3(0, -9.81, 0));

    player = new Player(dynamicsWorld, btVector3(0.0f, 2.0f, 3.0f));
}

void renderScene() {
    depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(computeLightSpaceTrMatrix()));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    drawObjects(depthMapShader, true, shadowModelLoc);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    myBasicShader.useShaderProgram();

    projection = glm::perspective(glm::radians(fov),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 200.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    view = currentCamera->getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    //bind the shadow map
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(computeLightSpaceTrMatrix()));

    drawObjects(myBasicShader, false, modelLoc);

    mySkyBox.Draw(skyboxShader, view, projection);
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data

    delete player;

    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete colissionConfiguration;

    delete myCamera;
    delete animationCamera;
}

void animateCamera() 
{
    //z = -51

    if (currentCamera->getPosition().z < -40)
    {
        cameraAnimation = false;
    }
    else
    {
        currentCamera->move(gps::MOVE_FORWARD, 10 * delta);
    }
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();

    myCamera = new gps::Camera(
        glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    animationCamera = new gps::Camera(
        glm::vec3(0.0f, 0.5f, 52.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    currentCamera = myCamera;

    initBullet();
    initSkyBox();
    glCheckError();
	initShaders();
    glCheckError();
	initUniforms();
    glCheckError();
    initFBO();
    initModels();
    setWindowCallbacks();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        computeDelta();
        processMovement();

        dynamicsWorld->stepSimulation(delta);

        if (!cameraAnimation)
        {
            btVector3 playerPos = player->getPosition();
            currentCamera->setPosition(glm::vec3(playerPos.x(), playerPos.y(), playerPos.z()));
        }
        else 
        {
            animateCamera();
        }

	    renderScene();
        glCheckError();

        animateObjectLocation(target1, { btVector3(10.0f, -0.1f, -20.0f), btVector3(7.5f, -0.1f, -20.0f) });

        animateObjectLocation(target2, { btVector3(-7.5f, -0.4f, -20.0f), btVector3(-5.0f, -0.4f, -20.0f) });
        animateObjectLocation(target3, { btVector3(-7.5f, 0.1f, -19.8f), btVector3(-5.0f, 0.1f, -19.8f) });
        animateObjectRotation(target3, { btQuaternion(0, glm::pi<float>(), glm::radians(-90.0f)), btQuaternion(0, glm::pi<float>(), glm::radians(90.0f)) });

        animateObjectLocation(target2_2, { btVector3(7.5f, -0.4f, -20.0f), btVector3(5.0f, -0.4f, -20.0f) });
        animateObjectLocation(target3_2, { btVector3(7.5f, 0.1f, -19.8f), btVector3(5.0f, 0.1f, -19.8f) });
        animateObjectRotation(target3_2, { btQuaternion(0, glm::pi<float>(), glm::radians(-90.0f)), btQuaternion(0, glm::pi<float>(), glm::radians(90.0f)) });

        animateObjectLocation(target1_2, { btVector3(-10.0f, -0.1f, -20.0f), btVector3(-7.5f, -0.1f, -20.0f) });

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
