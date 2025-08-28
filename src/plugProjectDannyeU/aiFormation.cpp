#include "PikiAI.h"
#include "Game/PikiState.h"
#include "Game/Navi.h"
#include "Game/gameStat.h"
#include "Game/CPlate.h"

namespace PikiAI {

void ActFormation::init(ActionArg* initArg)
{
	ActFormationInitArg* formationArg = static_cast<ActFormationInitArg*>(initArg);
	P2ASSERTLINE(267, formationArg);
	mNextAIType = 1;

	Game::Navi* currNavi = mParent->mNavi;

	mNavi = mParent->mNavi;
	Game::GameStat::formationPikis.inc(mParent);
	mInitArg.mCreature           = formationArg->mCreature;
	mInitArg.mIsDemoFollow       = formationArg->mIsDemoFollow;
	mInitArg.mDoUseTouchCooldown = formationArg->mDoUseTouchCooldown;

	if (mInitArg.mDoUseTouchCooldown) {
		mTouchingNaviCooldownTimer = 45;
	} else {
		mTouchingNaviCooldownTimer = 0;
	}

	Game::Navi* initNavi = static_cast<Game::Navi*>(formationArg->mCreature);
	bool initCheck       = formationArg->mIsDemoFollow;

	if (!initNavi) {
		mSlotID = -1;
		return;
	}

	mDistanceType         = 5;
	mOldDistanceType      = 5;
	mDistanceCounter      = 0;
	mHasLostNumbness      = false;
	mHadNumbnessLastFrame = false;

	mCPlate = initNavi->mCPlateMgr;
	mSlotID = mCPlate->getSlot(mParent, this, initCheck);
	if (mSlotID == -1 && initCheck) {
		JUT_PANICLINE(330, "slot id is -1");
	}

	if (mParent->getStateID() != Game::PIKISTATE_Nukare) {
		mParent->startMotion(Game::IPikiAnims::RUN2, Game::IPikiAnims::RUN2, nullptr, nullptr);
	}

	mHasReleasedSlot   = false;
	mUnusedVal         = 0;
	mSortState         = 0;
	mAnimationTimer    = 0;
	mTripCheckMoveDist = 0.0f;
	mIsAnimating       = 0;
	mFootmark          = nullptr;

	mParent->setPastel(false);
	mTouchingWallTimer = 0;
	mFootmarkFlags     = -1;
	mParent->setFreeLightEffect(false);
}

} // namespace PikiAI
