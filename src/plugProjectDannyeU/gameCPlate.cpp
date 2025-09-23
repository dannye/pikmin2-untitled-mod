#include "Game/CPlate.h"

namespace Game {

void CPlate::swapSlot(int i, int j)
{
	Slot* slot     = &mSlots[i];
	Slot* prevSlot = &mSlots[j];

	Creature* iCreature           = slot->mCreature;
	SlotChangeListener* iListener = slot->mListener;
	Creature* jCreature           = prevSlot->mCreature;
	SlotChangeListener* jListener = prevSlot->mListener;

	slot->mCreature = nullptr;
	slot->mCreature = jCreature;
	slot->mListener = jListener;
	slot->mListener->inform(i);

	prevSlot->mCreature = nullptr;
	prevSlot->mCreature = iCreature;
	prevSlot->mListener = iListener;
	prevSlot->mListener->inform(j);
}

} // namespace Game

int getPriority(int* pikiCounts, int color)
{
	for (int i = 0; i < Game::PikiColorCount + 1; i++) {
		if (color == pikiCounts[i]) {
			return i;
		}
	}

	JUT_PANICLINE(405, "col %d : sort failed !\n", color);
	return 128;
}

namespace Game {

void CPlate::sortByColor(Creature* piki, int happaType)
{
	int kind  = static_cast<Piki*>(piki)->getKind();
	int happa = static_cast<Piki*>(piki)->getHappa();

	if (static_cast<Piki*>(piki)->mBomb) {
		kind = BombPikmin;
	}

	int pikiCounts[PikiColorCount + 1];
	for (int i = 0; i < PikiColorCount + 1; i++) {
		pikiCounts[i] = (kind + i) % (PikiColorCount + 1);
	}

	int happaSlots[PikiGrowthStageCount];
	if (happaType != -1) {
		happaSlots[happaType]                              = Leaf;
		happaSlots[(happaType + 1) % PikiGrowthStageCount] = Bud;
		happaSlots[(happaType + 2) % PikiGrowthStageCount] = Flower;
	}

	for (int i = 0; i < mSlotCount; i++) {
		for (int j = 0; j < mSlotCount; j++) {
			Piki* iPiki = static_cast<Piki*>(mSlots[i].mCreature);
			Piki* jPiki = static_cast<Piki*>(mSlots[j].mCreature);
			int iKind   = iPiki->getKind();
			int jKind   = jPiki->getKind();

			if (iPiki->mBomb) {
				iKind = BombPikmin;
			}
			if (jPiki->mBomb) {
				jKind = BombPikmin;
			}

			if (iKind != jKind) {
				int iPrio = getPriority(pikiCounts, iKind);
				int jPrio = getPriority(pikiCounts, jKind);

				if (j > i && jPrio < iPrio) {
					swapSlot(j, i);
				}
				continue;
			}

			int iPrio;
			int jPrio;
			if (happaType == -1) {
				jPrio = happa != jPiki->getHappa();
				iPrio = happa != iPiki->getHappa();
			} else {
				jPrio = happaSlots[jPiki->getHappa()];
				iPrio = happaSlots[iPiki->getHappa()];
			}

			if (j > i && jPrio < iPrio) {
				swapSlot(j, i);
			}
		}
	}
}

} // namespace Game
