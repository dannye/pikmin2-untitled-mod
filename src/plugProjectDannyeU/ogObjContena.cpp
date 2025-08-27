#include "og/newScreen/Contena.h"
#include "og/Screen/AlphaMgr.h"
#include "og/Screen/StickAnimMgr.h"
#include "og/Sound.h"
#include "System.h"
#include "Controller.h"
#include "stdlib.h"

namespace og {
namespace newScreen {

void ObjContena::changeMessage(u32 mesg)
{
	if ((u32)mState != mesg) {
		mAlphaMgr[mState]->out(0.5f);
		mState = mesg;
		mAlphaMgr[mState]->in(0.3f);
	}
}

void ObjContena::setStickUp()
{
	mAlphaArrow1->in(0.3f);
	mAlphaArrow2->out(0.5f);
	mStickAnimMgr->stickUp();
}

void ObjContena::setStickDown()
{
	mAlphaArrow1->out(0.5f);
	mAlphaArrow2->in(0.3f);
	mStickAnimMgr->stickDown();
}

void ObjContena::setStickUpDown()
{
	mAlphaArrow1->in(0.3f);
	mAlphaArrow2->in(0.3f);
	mStickAnimMgr->stickUpDown();
}

void ObjContena::putinPiki(bool soundType, u8 step)
{
	og::Screen::DispMemberContena* disp = mDisp;
	if (disp->mDataContena.mCurrField <= disp->mDataContena.mInOnionCount) {
		if (mState == 1) {
			if (!soundType) {
				ogSound->setError();
			}
		} else {
			ogSound->setError();
			changeMessage(1);
			setStickDown();
		}
		return;
	}

	if (disp->mDataContena.mInSquadCount == 0) {
		if (mState == 4) {
			if (!soundType) {
				ogSound->setError();
			}
		} else {
			ogSound->setError();
			changeMessage(4);
			setStickDown();
		}
	} else {
		if (step > disp->mDataContena.mInSquadCount) {
			step = disp->mDataContena.mInSquadCount;
		}
		changeMessage(0);
		disp->mDataContena.mInOnionCount += step;
		disp->mDataContena.mInSquadCount -= step;
		disp->mDataContena.mInParty2 -= step;
		disp->mDataContena.mOnMapCount -= step;
		disp->mDataContena.mResult += step;
		disp->mDataContena.mInTransfer = (u16)abs(disp->mDataContena.mResult); // should be just abs
		setStickUpDown();
		if (mTimer1 <= 0.0f) {
			mScaleArrow1->up();
			mTimer1 = msVal._38;
		}
		mScaleMgr3->up(0.1f, 30.0f, 0.8f, 0.0f);
		mScaleMgr4->down(0.05f, 35.0f, 0.8f);
		ogSound->setPlusMinus(soundType);
	}
}

void ObjContena::takeoutPiki(bool soundType, u8 step)
{
	og::Screen::DispMemberContena* disp = mDisp;
	if (disp->mDataContena.mInSquadCount < disp->mDataContena.mMaxPikiOnField) {
	} else if (mState == 2) {
		if (!soundType) {
			ogSound->setError();
		}
		return;
	} else {
		ogSound->setError();
		changeMessage(2);
		setStickUp();
		return;
	}

	if (disp->mDataContena.mInOnionCount == 0) {
		if (mState == 3) {
			if (!soundType) {
				ogSound->setError();
			}
			return;
		} else {
			ogSound->setError();
			changeMessage(3);
			setStickUp();
			return;
		}
	} else if (disp->mDataContena.mOnMapCount < disp->mDataContena.mMaxPikiCount) {
	} else if (mState == 5) {
		if (!soundType) {
			ogSound->setError();
		}
		return;
	} else {
		ogSound->setError();
		changeMessage(5);
		setStickUp();
		return;
	}

	if (step > disp->mDataContena.mInOnionCount) {
		step = disp->mDataContena.mInOnionCount;
	}
	if (step > disp->mDataContena.mMaxPikiCount - disp->mDataContena.mOnMapCount) {
		step = disp->mDataContena.mMaxPikiCount - disp->mDataContena.mOnMapCount;
	}
	changeMessage(0);
	disp->mDataContena.mInOnionCount -= step;
	disp->mDataContena.mInSquadCount += step;
	disp->mDataContena.mInParty2 += step;
	disp->mDataContena.mOnMapCount += step;
	disp->mDataContena.mResult -= step;
	disp->mDataContena.mInTransfer = (u16)abs(disp->mDataContena.mResult);
	setStickUpDown();
	if (mTimer2 <= 0.0f) {
		mScaleArrow2->up();
		mTimer2 = msVal._38;
	}
	mScaleMgr4->up(0.1f, 30.0f, 0.8f, 0.0f);
	mScaleMgr3->down(0.05f, 35.0f, 0.8f);
	ogSound->setPlusMinus(soundType);
}

bool ObjContena::moveContena()
{
	bool ret                      = false;
	og::Screen::DataContena* data = &mDisp->mDataContena;
	JUT_ASSERTLINE(603, data, "DataContena is not found!\n");
	JUT_ASSERTLINE(607, data->mOnyonID != -1, "Contena Type error!\n");

	if (mTimer1 > 0.0f) {
		mTimer1 -= sys->mDeltaTime;
	}
	if (mTimer2 > 0.0f) {
		mTimer2 -= sys->mDeltaTime;
	}

	if (!data->mState) {
		data->mState = 1;
	} else {
		if (mController->getButtonDown() & Controller::PRESS_B) {
			mDisp->mDataContena = mDataContena;
			data->mState        = 2;
			mDispState          = 3;
			data->mResult       = 0;
			data->mInTransfer   = 0;
			if ((data->mOnyonID == Game::Purple || data->mOnyonID == Game::White) && data->mExitSoundType) {
				ogSound->setCancel();
			} else {
				ogSound->setClose();
			}
			ret = true;
		} else if (mController->getButtonDown() & Controller::PRESS_A) {
			data->mState = 2;
			mDispState   = 4;
			ogSound->setDecide();
			ret = true;
		}
	}

	if (data->mState == 1) {
		u8 step = 1;
		if (mController->getButton() & Controller::PRESS_Y) {
			step = 10;
		}
		if (mController->getButton() & Controller::PRESS_UP) {
			switch (mScreenState) {
			case 0:
				mScreenState = 1;
				putinPiki(false, step);
				mTimer0 = mMoveTime;
				break;
			case 1:
				mTimer0 -= sys->mDeltaTime;
				if (mTimer0 < 0.0f)
					mScreenState = 2;
				break;
			case 2:
				putinPiki(true, step);
				break;
			default:
				mScreenState = 0;
				break;
			}
		} else if (mController->getButton() & Controller::PRESS_DOWN) {
			switch (mScreenState) {
			case 0:
				mScreenState = 3;
				takeoutPiki(false, step);
				mTimer0 = mMoveTime;
				break;
			case 3:
				mTimer0 -= sys->mDeltaTime;
				if (mTimer0 < 0.0f)
					mScreenState = 4;
				break;
			case 4:
				takeoutPiki(true, step);
				break;
			default:
				mScreenState = 0;
				break;
			}
		} else {
			mScreenState = 0;
		}
	}
	return ret;
}

} // namespace newScreen
} // namespace og
