#ifndef _GAME_ENTITIES_ITEMSTONEGATE_H
#define _GAME_ENTITIES_ITEMSTONEGATE_H

#include "types.h"
#include "Game/Entities/ItemGate.h"

namespace Game {
namespace ItemStoneGate {
struct Mgr : public BaseItemMgr {
	Mgr();

	virtual u32 generatorGetID() { return 'sgat'; }
	virtual u32 generatorLocalVersion() { return '0002'; }
	virtual void doAnimation() { mNodeObjectMgr.doAnimation(); }
	virtual void doEntry() { mNodeObjectMgr.doEntry(); }
	virtual void doSetView(int viewportNumber) { mNodeObjectMgr.doSetView(viewportNumber); }
	virtual void doViewCalc() { mNodeObjectMgr.doViewCalc(); }
	virtual void doSimulation(f32 rate) { mNodeObjectMgr.doSimulation(rate); }
	virtual void doDirectDraw(Graphics& gfx) { mNodeObjectMgr.doDirectDraw(gfx); }
	virtual void initDependency();
	virtual BaseItem* generatorBirth(Vector3f&, Vector3f&, GenItemParm*);
	virtual void generatorWrite(Stream&, GenItemParm*);
	virtual void generatorRead(Stream&, GenItemParm*, u32);
	virtual GenItemParm* generatorNewItemParm();
	virtual char* getCaveName(int);
	virtual int getCaveID(char*);

	void setupGate(ItemGate*);
	void setupPlatform(ItemGate*);
	BaseItem* birth();

	NodeObjectMgr<ItemGate> mNodeObjectMgr;
	Platform* mCentrePlatform;
	Platform* mSidePlatform;
	Sys::MatTevRegAnimation mMatTevRegAnim;
};

extern Mgr* mgr;

} // namespace ItemStoneGate
} // namespace Game

#endif
