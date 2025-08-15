#include "Object3D.hpp"

Object3D::~Object3D()
{
	delete this->rigidBody;
	delete this->motionState;
	delete this->collisionShape;
}

btRigidBody* Object3D::GetRigidBody()
{
	return this->rigidBody;
}

void Object3D::Render(gps::Shader shader, bool depthPass, GLint modelLoc, GLint &normalMatrixLoc, glm::mat4 view)
{
	shader.useShaderProgram();

	glm::mat4 model = glm::mat4(1.0f);
	
	btTransform transform;
	this->rigidBody->getMotionState()->getWorldTransform(transform);

	transform.getOpenGLMatrix(glm::value_ptr(model));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass)
	{
		glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	this->thisModel.Draw(shader);
}

void Object3D::Load(std::string fileName, std::string basePath, float mass, glm::vec3 position, glm::vec3 rotation, COLLIDER_TYPE colliderType, bool isKinematic)
{
	if (basePath.empty())
	{
		this->thisModel.LoadModel(fileName);
	}
	else
	{
		this->thisModel.LoadModel(fileName, basePath);
	}

	this->isKinematic = isKinematic;

	struct BoundingBox boundingBox = this->thisModel.GetBoundingBox();

	if (colliderType == BOX_COLLIDER)
	{
		this->collisionShape = new btBoxShape(btVector3(boundingBox.size.x / 2, boundingBox.size.y / 2, boundingBox.size.z / 2));
	}
	else if (colliderType == MESH_COLLIDER)
	{
		gps::Mesh mesh = this->thisModel.getFirstMesh();
		std::vector<gps::Vertex> vertices = mesh.vertices;
		std::vector<GLuint> indices = mesh.indices;

		btTriangleMesh* triangleMesh = new btTriangleMesh();

		for (size_t i = 0; i < indices.size(); i += 3)
		{
			glm::vec3& v0 = vertices[indices[i]].Position;
			glm::vec3& v1 = vertices[indices[i + 1]].Position;
			glm::vec3& v2 = vertices[indices[i + 2]].Position;

			triangleMesh->addTriangle(btVector3(v0.x, v0.y, v0.z), btVector3(v1.x, v1.y, v1.z), btVector3(v2.x, v2.y, v2.z));
		}

		collisionShape = new btBvhTriangleMeshShape(triangleMesh, true, true);
	}

	btTransform transform;
	transform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z));

	transform.setOrigin(btVector3(position.x, position.y, position.z));

	this->motionState = new btDefaultMotionState(transform);

	btRigidBody::btRigidBodyConstructionInfo info(mass, this->motionState, this->collisionShape, btVector3(0, 0, 0));
	this->rigidBody = new btRigidBody(info);

	if (isKinematic) {
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		rigidBody->setActivationState(DISABLE_DEACTIVATION);
	}
}

void Object3D::SetPosition(btVector3 newPosition)
{
	if (isKinematic)
	{
		btTransform transform;
		rigidBody->getMotionState()->getWorldTransform(transform);
		transform.setOrigin(newPosition);
		rigidBody->getMotionState()->setWorldTransform(transform);
	}
	else
	{
		rigidBody->getWorldTransform().setOrigin(newPosition);
	}
	
}

void Object3D::SetRotation(btQuaternion newRotation)
{
	if (isKinematic)
	{
		btTransform transform;
		rigidBody->getMotionState()->getWorldTransform(transform);
		transform.setRotation(newRotation);
		rigidBody->getMotionState()->setWorldTransform(transform);
	}
	else
	{
		rigidBody->getWorldTransform().setRotation(newRotation);
	}
}

btVector3 Object3D::GetPosition()
{
	return rigidBody->getWorldTransform().getOrigin();
}

btQuaternion Object3D::GetRotation()
{
	return rigidBody->getWorldTransform().getRotation();
}
