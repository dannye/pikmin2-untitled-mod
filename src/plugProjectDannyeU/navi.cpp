#include "Game/Navi.h"
#include "Game/NaviState.h"
#include "Game/NaviParms.h"
#include "Game/MoviePlayer.h"
#include "Game/CameraMgr.h"
#include "P2JME/P2JME.h"

#define GO_HERE_NAVI_DEBUG (false)

namespace Game {

void Navi::doSimulation(f32 timeStep)
{
	if (gameSystem->mIsFrozen && !gameSystem->isMultiplayerMode() && this != naviMgr->getActiveNavi()) {
		return;
	}

	if (moviePlayer->isFlag(MVP_IsActive)) {
		mVelocity       = Vector3f(0.0f);
		mTargetVelocity = Vector3f(0.0f);
		mAcceleration   = Vector3f(0.0f);
	}

	FakePiki::doSimulation(timeStep);
}

void Navi::doDirectDraw(Graphics& gfx)
{
	if (gfx.mCurrentViewport->mCamera != mCamera) {
		return;
	}
	if (moviePlayer && moviePlayer->mDemoState != DEMOSTATE_Inactive) {
		return;
	}

	int stateID = getStateID();
	if (stateID == NSID_Nuku || stateID == NSID_NukuAdjust) {
		bool display;
		if (stateID == NSID_Nuku) {
			Game::NaviNukuState* state = (Game::NaviNukuState*)getCurrState();
			display = state->mDidPressB == 0;
		} else {
			display = true;
		}
		if (display) {
			GXSetZMode(GX_FALSE, GX_LESS, GX_FALSE);
			Vector3f pos(mPosition.x, mPosition.y + 35.0f, mPosition.z);

			PerspPrintfInfo info(0.4f);
			info.mFont = gP2JMEMgr->mFont;
			info.mPerspectiveOffsetX = -55;
			info.mColorA = Color4(0xFF, 0x44, 0x44, 0xFF);
			info.mColorB = Color4(0xFF, 0x22, 0x22, 0xFF);
			gfx.perspPrintf(info, pos, "B");

			info.mPerspectiveOffsetX = 15;
			info.mColorA = Color4(0xFF, 0xFF, 0xFF, 0xFF);
			info.mColorB = Color4(0x99, 0x99, 0xFF, 0xFF);
			gfx.perspPrintf(info, pos, "Cancel");
		}
		return;
	}

	if (stateID == NSID_Walk || stateID == NSID_Throw || stateID == NSID_ThrowWait) {
		Piki* piki = nullptr;
		if (stateID == NSID_Walk) {
			piki = mNextThrowPiki;
		} else if (stateID == NSID_Throw) {
			Game::NaviThrowState* state = (Game::NaviThrowState*)getCurrState();
			piki = state->mPiki;
		} else if (stateID == NSID_ThrowWait) {
			Game::NaviThrowWaitState* state = (Game::NaviThrowWaitState*)getCurrState();
			if (state->mHasHeldPiki)
				piki = state->mHeldPiki;
			else
				piki = state->mNextPiki;
		}
		if (piki) {
			Vector3f start = getPosition() + Vector3f(-15.0f * sinf(mFaceDir), 10.0f, -15.0f * cosf(mFaceDir));
			Vector3f end(mWhistle->mPosition.x, start.y, mWhistle->mPosition.z);
			Vector3f throwDir = end - start; throwDir.normalise();

			float height, distance;
			switch (piki->mPikiKind) {
			case Yellow:
				end -= throwDir * 10.0f;
				height = C_NAVIPARMS.mThrowHeightYellow.mValue;
				distance = 1.0f;
				break;
			case Purple:
				end += throwDir * 5.0f;
				height = C_NAVIPARMS.mThrowBlackHeight.mValue + 10.0f;
				distance = 2.0f;
				break;
			case White:
				end -= throwDir * 7.5f;
				height = C_NAVIPARMS.mThrowWhiteHeight.mValue;
				distance = 1.0f;
				break;
			default:
				end -= throwDir * 2.5f;
				height = C_NAVIPARMS.mThrowHeightMin.mValue;
				distance = 1.0f;
			}

			GXSetLineWidth(stateID == NSID_ThrowWait ? 10 : 6, GX_TO_ZERO);
			gfx.mDrawColor = piki->mDefaultColor;
			Vector3f prev = start;
			for (int i = 1; i <= 31; ++i) {
				float t = i / 32.0f;
				Vector3f next(
					start.x * (1.0f - t) + end.x * t,
					start.y - 4.0f / (distance * distance) * height * t * (t - distance),
					start.z * (1.0f - t) + end.z * t
				);
				if (i > 2) gfx.drawLine(prev, next);
				prev = next;
			}
			gfx.drawLine(prev, end);
		}
		return;
	}

#if GO_HERE_NAVI_DEBUG
	if (getStateID() != NSID_GoHere) {
		return;
	}

	Game::NaviGoHereState* state = (Game::NaviGoHereState*)getCurrState();

	PerspPrintfInfo info;
	Vector3f pos(mPosition.x, 15.0f + mPosition.y, mPosition.z);

	info.mColorA = Color4(0xC8, 0xC8, 0xFF, 0xC8);
	info.mColorB = Color4(0x64, 0x64, 0xFF, 0xC8);
	gfx.perspPrintf(info, pos, "[%d/%d] t[%1.1f]", state->mActiveRouteNodeIndex, state->mPath.mLength, state->mTimeoutTimer);
#endif
}

} // namespace Game
