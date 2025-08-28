#include "Game/NaviState.h"
#include "Game/NaviParms.h"
#include "Game/MoviePlayer.h"
#include "Dolphin/rand.h"
#include "PikiAI.h"
#include "Game/Entities/ItemPikihead.h"
#include "Game/rumble.h"
#include "PSM/Navi.h"
#include "Game/PikiState.h"
#include "Game/CPlate.h"

namespace Game {

void NaviWalkState::execAI(Navi* navi)
{
	switch (mAIState) {
	case WALKAI_Wait:
		if (execAI_wait(navi))
			checkAI(navi);
		break;

	case WALKAI_Animation:
		execAI_animation(navi);
		checkAI(navi);
		break;

	case WALKAI_Escape:
		execAI_escape(navi);
		break;

	case WALKAI_Attack:
		execAI_attack(navi);
		break;
	}
}

bool NaviWalkState::execAI_wait(Navi* navi)
{
	blendVelocity(navi, Vector3f::zero);
	mIdleTimer -= sys->mDeltaTime;

	if (mIdleTimer <= 0.0f) {
		ItemPikihead::Item* targetSprout = nullptr;
		if (navi != naviMgr->getActiveNavi()) {
			f32 actionRadius = naviMgr->mNaviParms->mNaviParms.mAutopluckDistance.mValue;
			f32 minDist = actionRadius * actionRadius;

			Iterator<ItemPikihead::Item> iter(ItemPikihead::mgr);

			CI_LOOP(iter)
			{
				ItemPikihead::Item* sprout = *iter;
				Vector3f sproutPos = sprout->getPosition();
				Vector3f naviPos   = navi->getPosition();
				f32 heightDiff     = FABS(sproutPos.y - naviPos.y);
				f32 sqrXZ          = sqrDistanceXZ(sproutPos, naviPos);

				if (sprout->canPullout() && sqrXZ < minDist && heightDiff < 25.0f
				    && (!gameSystem->isVersusMode() || sprout->mColor == (1 - navi->mNaviIndex))) {
					minDist      = sqrXZ;
					targetSprout = sprout;
				}
			}
		}

		if (targetSprout) {
			NaviNukuAdjustStateArg nukuAdjustArg;
			navi->setupNukuAdjustArg(targetSprout, nukuAdjustArg);
			navi->mFsm->transit(navi, NSID_NukuAdjust, &nukuAdjustArg);
			return false;
		} else {
			initAI_animation(navi);
			mIdleTimer = 2.0f + randFloat();
		}
		return true;
	}

	if (mTarget) {
		Vector3f naviPos   = navi->getPosition();
		Vector3f targetPos = mTarget->getPosition();
		Vector3f sep       = naviPos - targetPos;
		f32 rad            = 100.0f;
		if (sep.sqrMagnitude2D() > SQUARE(rad)) {
			mTarget = nullptr;
			return;
		}

		Vector3f::getFlatDirectionFromTo(naviPos, targetPos);

		navi->mFaceDir += 0.2f * angDist(roundAng(JMAAtan2Radian(targetPos.x, targetPos.z)), navi->mFaceDir);
		navi->mFaceDir = roundAng(navi->mFaceDir);
	}
	return true;
}

void NaviNukuState::exec(Navi* navi)
{
	if (moviePlayer && moviePlayer->mDemoState != DEMOSTATE_Inactive) {
		if (mIsFollower) {
			NaviFollowArg followArg(false); // not new to party
			transit(navi, NSID_Follow, &followArg);
			return;
		}
		transit(navi, NSID_Walk, nullptr);
		return;
	}
	navi->mVelocity       = 0.0f;
	navi->mTargetVelocity = 0.0f;
	if (!navi->assertMotion(mAnimID)) {
		if (mIsFollower != 0) {
			NaviFollowArg followArg(false); // not new to party
			transit(navi, NSID_Follow, &followArg);
		} else {
			transit(navi, NSID_Walk, nullptr);
		}
		navi->mPluckingCounter = 0;
	} else if (mIsFollower == 0) {
		if (mDidPressB == 0 && navi->mController1 && navi->mController1->isButtonDown(JUTGamePad::PRESS_B)) {
			mDidPressB = 1;
			mIsActive  = 0;
		}
		if (mDidPressB == 0 && mIsActive == 0) {
			mIsActive = 1;
			navi->mPluckingCounter++;
		}
	}
}

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

	if (navi->mController1->getButtonDown() & Controller::PRESS_B) {
		transit(navi, NSID_Walk, nullptr);
		return;
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

void NaviThrowState::onKeyEvent(SysShape::KeyEvent const& key)
{
	switch (key.mType) {
	case 2:
		if (!mPiki->isThrowable()) {
			mHasThrown = true;
		} else {
			Vector3f pos = mNavi->mWhistle->getPosition();
			mNavi->throwPiki(mPiki, pos);
			mPiki->mBrain->start(PikiAI::ACT_Free, nullptr);
			mPiki->mFsm->transit(mPiki, PIKISTATE_Flying, nullptr);
			mPiki->setFreeLightEffect(false);
			mHasThrown = true;
		}
		break;
	case KEYEVENT_END:
		transit(mNavi, NSID_Walk, nullptr);
		break;
	}
}

} // namespace Game
