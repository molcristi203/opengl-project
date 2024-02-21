#include "Player.hpp"
#include <iostream>

Player::Player(btDynamicsWorld* dynamicsWorld, btVector3 startPosition)
{
	capsuleShape = new btCapsuleShape(0.5, 2);

	btRigidBody::btRigidBodyConstructionInfo info(1, nullptr, capsuleShape);
	rigidBody = new btRigidBody(info);
	rigidBody->setUserPointer(this);
	rigidBody->setAngularFactor(btVector3(0.0f, 1.0f, 0.0f));

	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(startPosition);
	rigidBody->setWorldTransform(startTransform);
	rigidBody->setFriction(1);
	rigidBody->setDamping(0.8, 0.8);

	dynamicsWorld->addRigidBody(rigidBody);
}

void Player::move(btVector3 velocity)
{
	//std::cout << velocity.x() << " " << velocity.y() << " " << velocity.z();
	//rigidBody->applyCentralForce(velocity);
	rigidBody->activate(true);

	float currentSpeed = rigidBody->getLinearVelocity().length();

	// If the current speed is below the maximum speed, apply force
	if (currentSpeed < 100) {
		rigidBody->applyCentralForce(velocity);
	}
	else {
		// If the speed exceeds the limit, clamp it to the maximum speed
		btVector3 clampedVelocity = rigidBody->getLinearVelocity().normalized() * 15;
		rigidBody->setLinearVelocity(clampedVelocity);
	}
}

btVector3 Player::getPosition()
{
	btTransform transform;
	transform = this->rigidBody->getWorldTransform();

	return transform.getOrigin();
}

Player::~Player()
{
	//dynamicsWorld->removeRigidBody(rigidBody);
	//delete rigidBody;
	//delete capsuleShape;
}