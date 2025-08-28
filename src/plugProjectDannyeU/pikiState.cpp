#include "Game/PikiState.h"
#include "Game/Navi.h"
#include "Game/rumble.h"
#include "efx/TEnemyDive.h"

namespace Game {

void PikiNukareState::onKeyEvent(Piki* piki, SysShape::KeyEvent const& keyEvent)
{
	switch (keyEvent.mType) {
	case KEYEVENT_2:
		rumbleMgr->startRumble(RUMBLETYPE_PluckPiki, (int)mNavi->mNaviIndex);

		Vector3f position = piki->getPosition();
		piki->setFPFlag(FPFLAGS_PikiBeingPlucked);
		Sys::Sphere sphere(position, 10.0f);
		WaterBox* wbox = piki->checkWater(nullptr, sphere);

		if (wbox) {
			efx::TEnemyDive diveFx;
			efx::ArgScale fxArg(position, 1.2f);
			diveFx.create(&fxArg);

			if (piki->mNavi == nullptr) {
				JUT_PANICLINE(1242, "getNavi():pullW");
			}

			piki->startSound(piki->mNavi, PSSE_EV_ITEM_LAND_WATER1_S, true);
			piki->startSound(piki->mNavi, PSSE_PL_PULLOUT_PIKI, false);

		} else {
			if (piki->mNavi == nullptr) {
				JUT_PANICLINE(1246, "getNavi():Pull");
			}
			efx::createSimplePkAp(position);
			piki->startSound(piki->mNavi, PSSE_PL_PULLOUT_PIKI, false);
		}

		break;
	case KEYEVENT_END:
		mDoFinish = true;
		break;
	}
}

} // namespace Game
