#include "Game/NaviState.h"
#include "Game/NaviParms.h"
#include "Game/MoviePlayer.h"
#include "Game/rumble.h"
#include "PSM/Navi.h"
#include "Game/PikiState.h"
#include "Game/CPlate.h"

namespace Game {

void NaviThrowWaitState::exec(Navi* navi)
{
	if (moviePlayer && moviePlayer->mDemoState != DEMOSTATE_Inactive) {
		transit(navi, NSID_Walk, nullptr);
		return;
	}

	if (!navi->mController1) {
		return;
	}

	navi->control();

	if (!mHeldPiki) {
		if (mNextPiki) {
			mNextPikiTimeLimit -= sys->mDeltaTime;
			if (mNextPikiTimeLimit < 0.0f) {
				transit(navi, NSID_Walk, nullptr);
				return;
			}

			if (navi->mController1->getButtonDown() & Controller::PRESS_B) {
				transit(navi, NSID_Walk, nullptr);
				return;
			}
			CollPart* part   = navi->mCollTree->getCollPart('rhnd');
			Vector3f handPos = part->mPosition;
			Vector3f pikiPos = mNextPiki->getPosition();
			f32 dist         = handPos.distance(pikiPos);
			if (!(dist <= 32.5f))
				return;

			navi->mAnimSpeed = 30.0f;
			navi->startMotion(IPikiAnims::THROWWWAIT, IPikiAnims::THROWWWAIT, this, nullptr);
			navi->enableMotionBlend();
			mHeldPiki = mNextPiki;
			mNextPiki = nullptr;
			rumbleMgr->startRumble(RUMBLETYPE_Nudge, mNavi->mNaviIndex);
			mHeldPiki->mFsm->transit(mHeldPiki, PIKISTATE_Hanged, nullptr);
			mHasHeldPiki = true;
		} else {
			transit(navi, NSID_Punch, nullptr);
			return;
		}
	}

	navi->mNextThrowPiki = mHeldPiki;

	f32 min               = CG_NAVIPARMS(navi).mThrowDistanceMin();
	f32 max               = CG_NAVIPARMS(navi).mThrowDistanceMax();
	navi->mHoldPikiCharge = mHoldChargeLevel / 3.0f * (max - min) + min;

	max                    = CG_NAVIPARMS(navi).mThrowHeightMax();
	min                    = CG_NAVIPARMS(navi).mThrowHeightMin();
	navi->mHoldPikiCharge2 = mHoldChargeLevel / 3.0f * (max - min) + min;

	if (mHeldPiki && mHasHeldPiki) {
		int stateID = mHeldPiki->getStateID();
		if (stateID != PIKISTATE_Hanged && stateID != PIKISTATE_GoHang) {
			transit(navi, NSID_Walk, nullptr);
			return;
		}
	}

	if (navi->mController1->getButtonDown() & Controller::PRESS_DPAD_RIGHT) {
		mCurrHappa    = -1;
		int currColor = mHeldPiki->getKind();
		int pikisNext[(PikiColorCount - 1)];
		for (int i = 0; i < (PikiColorCount - 1); i++) {
			pikisNext[i] = ((currColor + i + 1) % PikiColorCount);
		}

		Piki* newPiki = nullptr;
		for (int i = 0; i < (PikiColorCount - 1); i++) {
			Piki* p = findNearestColorPiki(navi, pikisNext[i]);
			if (p) {
				newPiki = p;
				break;
			}
		}

		if (newPiki) {
			Piki* held = mHeldPiki;
			if (held->mNavi) {
				if (currColor == Bulbmin) {
					held->mNavi->mSoundObj->stopSound(PSSE_PK_HAPPA_THROW_WAIT, 0);
				} else {
					held->mNavi->mSoundObj->stopSound(PSSE_PK_VC_THROW_WAIT, 0);
				}
			}
			held->mFsm->transit(held, PIKISTATE_Walk, nullptr);
			mHeldPiki = newPiki;
			newPiki->mFsm->transit(newPiki, PIKISTATE_Hanged, nullptr);
			sortPikis(navi);
			PSSystem::spSysIF->playSystemSe(PSSE_SY_THROW_PIKI_CHANGE, 0);
			rumbleMgr->startRumble(RUMBLETYPE_Nudge, navi->mNaviIndex);
			return;
		}

	} else if (navi->mController1->getButtonDown() & Controller::PRESS_DPAD_LEFT) {
		mCurrHappa    = -1;
		int currColor = mHeldPiki->getKind();
		int pikisNext[(PikiColorCount - 1)];
		for (int i = 0; i < (PikiColorCount - 1); i++) {
			pikisNext[i] = ((currColor + ((PikiColorCount - 2) - i) + 1) % PikiColorCount);
		}

		Piki* newPiki = nullptr;
		for (int i = 0; i < (PikiColorCount - 1); i++) {
			Piki* p = findNearestColorPiki(navi, pikisNext[i]);
			if (p) {
				newPiki = p;
				break;
			}
		}
		if (newPiki) {
			Piki* held = mHeldPiki;
			if (held->mNavi) {
				if (currColor == Bulbmin) {
					held->mNavi->mSoundObj->stopSound(PSSE_PK_HAPPA_THROW_WAIT, 0);
				} else {
					held->mNavi->mSoundObj->stopSound(PSSE_PK_VC_THROW_WAIT, 0);
				}
			}
			held->mFsm->transit(held, PIKISTATE_Walk, nullptr);
			mHeldPiki = newPiki;
			newPiki->mFsm->transit(newPiki, PIKISTATE_Hanged, nullptr);
			sortPikis(navi);
			PSSystem::spSysIF->playSystemSe(PSSE_SY_THROW_PIKI_CHANGE, 0);
			rumbleMgr->startRumble(RUMBLETYPE_Nudge, navi->mNaviIndex);
			return;
		}

	} else if (navi->mController1->getButtonDown() & Controller::PRESS_DPAD_UP
	           || navi->mController1->getButtonDown() & Controller::PRESS_DPAD_DOWN) {
		bool isButton = navi->mController1->isButtonDown(Controller::PRESS_DPAD_DOWN);
		int currColor = mHeldPiki->mPikiKind;
		int currHappa = mHeldPiki->mHappaKind;
		Piki* newPiki;
		for (int i = 0; i < MaxHappaStage; i++) {
			if (isButton) {
				mCurrHappa = (mCurrHappa + (PikiGrowthStageCount - 1)) % PikiGrowthStageCount; // leaf->flower, flower->bud, bud->leaf
			} else {
				mCurrHappa = (mCurrHappa + 1) % PikiGrowthStageCount; // leaf->bud, bud->flower, flower->leaf
			}
			newPiki = findNearestColorPiki(navi, currColor);
			if (newPiki) {
				if (newPiki->getHappa() != currHappa) {
					break;
				}
			}
			newPiki = nullptr;
		}
		if (newPiki) {
			Piki* held = mHeldPiki;
			if (held->mNavi) {
				if (currColor == Bulbmin) {
					held->mNavi->mSoundObj->stopSound(PSSE_PK_HAPPA_THROW_WAIT, 0);
				} else {
					held->mNavi->mSoundObj->stopSound(PSSE_PK_VC_THROW_WAIT, 0);
				}
			}

			held->mFsm->transit(held, PIKISTATE_Walk, nullptr);
			mHeldPiki = newPiki;
			newPiki->mFsm->transit(newPiki, PIKISTATE_Hanged, nullptr);
			sortPikis(navi);
			PSSystem::spSysIF->playSystemSe(PSSE_SY_THROW_PIKI_CHANGE, 0);
			rumbleMgr->startRumble(RUMBLETYPE_Nudge, navi->mNaviIndex);
			return;
		}
	}

	if (!(navi->mController1->getButton() & Controller::PRESS_A)) {
		sortPikis(navi);
		navi->mHoldPikiTimer = mHoldChargeLevel / 3.0f * CG_NAVIPARMS(navi).mTimeLimitForThrowing();
		NaviThrowInitArg arg(mHeldPiki);
		transit(navi, NSID_Throw, &arg);
		return;
	}

	navi->mHoldPikiTimer += sys->mDeltaTime;

	if (navi->mHoldPikiTimer > CG_NAVIPARMS(navi).mTimeLimitForThrowing()) {
		navi->mHoldPikiTimer = CG_NAVIPARMS(navi).mTimeLimitForThrowing();
	}
	if (mInitialSortDelayTimer > 0.0f) {
		mInitialSortDelayTimer -= sys->mDeltaTime;
		if (mInitialSortDelayTimer <= 0.0f) {
			sortPikis(navi);
		}
		return;
	}

	if (navi->mCPlateMgr->mActiveGroupSize > 0) {
		Vector3f slotPos = navi->mCPlateMgr->mSlots->mPosition;
		Vector3f naviPos = navi->getPosition();
		if (slotPos.distance(naviPos) > 30.0f) {
			Vector3f naviPos = navi->getPosition();
			Vector3f naviVel = navi->getVelocity();
			navi->mCPlateMgr->setPos(naviPos, navi->mFaceDir + PI, naviVel, 1.0f);
			sortPikis(navi);
		}
	}
}

} // namespace Game
