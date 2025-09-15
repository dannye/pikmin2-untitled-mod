#include "PikiAI.h"

#include "Game/Entities/Bomb.h"

namespace PikiAI {

ActPickUp::ActPickUp(Game::Piki* piki) : Action(piki)
{
	mApproachPos = new ActApproachPos(piki);
	mName = "PickUp";
}

void ActPickUp::init(ActionArg* arg) {
	bool isPickUpArg = false;
	if (arg) {
		isPickUpArg = strcmp("ActPickUpArg", arg->getName()) == 0;
	}
	P2ASSERT(isPickUpArg);
	ActPickUpArg* pickUpArg = static_cast<ActPickUpArg*>(arg);
	mBomb = pickUpArg->mBomb;
	P2ASSERT(mBomb && mBomb->mIsPikiBomb);

	ApproachPosActionArg approachPosActionArg(mBomb->mPosition, mBomb->getBodyRadius() * mBomb->getScaleMod(), -1.0f);
	mApproachPos->init(&approachPosActionArg);
}

int ActPickUp::exec() {
	if (mBomb == nullptr || !mBomb->isAlive() || mBomb->getStateID() != Game::Bomb::BOMB_Wait || mBomb->mCarrier) {
		return ACTEXEC_Fail;
	}

	mApproachPos->mGoalPosition = mBomb->mPosition;
	int approachResult = mApproachPos->exec();
	if (approachResult == ACTEXEC_Success) {
		// TODO: add animation
		mParent->mBomb = mBomb;
		mParent->updateMatrix();
		mBomb->startCapture(&mParent->mCaptureMatrix);
		mBomb->hardConstraintOff();
		mBomb->mCarrier = mParent;
		return ACTEXEC_Success;
	}

	return ACTEXEC_Continue;
}

void ActPickUp::cleanup() {}

} // namespace PikiAI
