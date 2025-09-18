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

	ApproachPosActionArg approachPosActionArg(mBomb->mPosition, mBomb->getBodyRadius() * mBomb->getScaleMod() * 0.4f, -1.0f);
	mApproachPos->init(&approachPosActionArg);
	mAnimFinished = false;
	mState = PICKUP_Walk;

	mParent->mSoundObj->startSound(PSSE_PK_VC_FIND, 0);
}

int ActPickUp::exec() {
	if (mBomb == nullptr || !mBomb->isAlive() || mBomb->getStateID() != Game::Bomb::BOMB_Wait || (mBomb->mCarrier && mBomb->mCarrier != mParent)) {
		P2ASSERT(!mParent->mBomb);
		return ACTEXEC_Fail;
	}

	switch (mState) {
	case PICKUP_Walk: {
		mApproachPos->mGoalPosition = mBomb->mPosition;
		int approachResult = mApproachPos->exec();
		if (approachResult == ACTEXEC_Success) {
			mParent->mBomb = mBomb;
			mParent->updateMatrix();
			mBomb->startCapture(&mParent->mCaptureMatrix);
			mBomb->hardConstraintOff();
			mBomb->mCarrier = mParent;

			mAnimFinished = false;
			mParent->startMotion(Game::IPikiAnims::PICK_PUT, Game::IPikiAnims::PICK_PUT, this, nullptr);
			mParent->enableMotionBlend();
			mParent->mTargetVelocity = Vector3f(0.0f);
			mParent->mSoundObj->startSound(PSSE_PK_VC_YATTA, 0);
			mState = PICKUP_Lift;
		}
		break;
	}
	case PICKUP_Lift: {
		if (mAnimFinished || !mParent->assertMotion(Game::IPikiAnims::PICK_PUT)) {
			return ACTEXEC_Success;
		}
		break;
	}
	}

	return ACTEXEC_Continue;
}

void ActPickUp::cleanup() {}

void ActPickUp::onKeyEvent(SysShape::KeyEvent const& keyEvent)
{
	switch (keyEvent.mType) {
	case KEYEVENT_LOOP_START:
		mAnimFinished = true;
		break;
	default:
		break;
	}
}

} // namespace PikiAI
