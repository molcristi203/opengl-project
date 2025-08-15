#include "btBulletDynamicsCommon.h"
#include "Camera.hpp"

class Player
{
public:
	Player(btDynamicsWorld* dynamicsWorld, btVector3 startPosition);
	void move(btVector3 velocity);
	btVector3 getPosition();
	~Player();

private:
	btDynamicsWorld* dynamicsWorld;
	btCapsuleShape* capsuleShape;
	btRigidBody* rigidBody;
};