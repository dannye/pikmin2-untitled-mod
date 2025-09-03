#include "Game/Navi.h"
#include "Game/MoviePlayer.h"

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

} // namespace Game
