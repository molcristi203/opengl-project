#include <iostream>
#include <string>
#include "Model3D.hpp"
#include "btBulletDynamicsCommon.h"
#include "Mesh.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

enum COLLIDER_TYPE { BOX_COLLIDER, MESH_COLLIDER };

class Object3D
{
public:
	~Object3D();
	btRigidBody* GetRigidBody();
	void Render(gps::Shader shader, bool depthPass, GLint modelLoc, GLint &normalMatrixLoc, glm::mat4 view);
	void Load(std::string fileName, std::string basePath, float mass, glm::vec3 position, glm::vec3 rotation, COLLIDER_TYPE colliderType, bool isKinematic);
	void SetPosition(btVector3 newPosition);
	void SetRotation(btQuaternion newRotation);
	btVector3 GetPosition();
	btQuaternion GetRotation();

	int currentPositionKeyframe = 0;
	int currentRotationKeyframe = 0;

private:
	gps::Model3D thisModel;
	btCollisionShape* collisionShape;
	btDefaultMotionState* motionState;
	btRigidBody* rigidBody;

	GLuint verticesVBO;
	GLuint triangleVAO;

	bool isKinematic;
};