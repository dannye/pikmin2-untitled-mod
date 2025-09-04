#include "Game/BaseGameSection.h"
#include "Game/GameLight.h"
#include "Game/Navi.h"

namespace Game {

void BaseGameSection::directDraw(Graphics& gfx, Viewport* vp)
{
	vp->setViewport();
	vp->setProjection();
	gfx.initPrimDraw(vp->getMatrix(true));
	doDirectDraw(gfx, vp);
	if (naviMgr) {
		Navi* olimar = naviMgr->getAt(NAVIID_Olimar);
		Navi* louie = naviMgr->getAt(NAVIID_Louie);
		if (olimar) {
			olimar->doDirectDraw(gfx);
		}
		if (louie) {
			louie->doDirectDraw(gfx);
		}
	}
	if (TexCaster::Mgr::sInstance) {
		gfx.initPrimDraw(vp->getMatrix(true));
		mLightMgr->mFogMgr->set(gfx);
		TexCaster::Mgr::sInstance->draw(gfx);
	}
}

} // namespace Game
