#include <algorithm>
#include <cstring>
#include <string>
#include <vector>
#include "IRenderDrawable.h"
#include "LinearAllocator.h"
#include "PIXTracker.h"
#include "RenderPasses.h"

namespace cdc {

static void defaultDrawer(uint32_t funcSetIndex, IRenderDrawable *drawable, IRenderDrawable *prevDrawable) {
	drawable->draw(funcSetIndex, prevDrawable);
}

static bool defaultComparator(uint32_t funcSetIndex, IRenderDrawable *drawable, IRenderDrawable *prevDrawable) {
	return drawable->compare(funcSetIndex, prevDrawable);
}

RenderPasses::RenderPasses() {
	for (uint32_t i=0; i<32; i++) {
		passes[i].active = false;
		requestedPassesScene[i] = ~0u;
		requestedPassesShadow[i] = ~0u;
	}
	activeFuncBitfield = 0;
}

uint32_t RenderPasses::addRenderPass(RenderPassType passType, uint32_t order, uint32_t sortMode, uint32_t funcSetIndex, uint32_t firstPassId) {
	uint32_t passId = firstPassId;
	while (passId < 32 && passes[passId].active)
		passId++;

	if (passId >= 32)
		return -1;

	passes[passId].active = true;
	passes[passId].order = order;
	passes[passId].sortMode = sortMode;
	passes[passId].passType = passType;
	passes[passId].funcSetIndex = funcSetIndex;
	passes[passId].callbacks = nullptr;

	// find insertion point by order
	uint32_t *r = passType == kShadowPass
		? requestedPassesShadow
		: requestedPassesScene;
	for (; *r != ~0u; r++)
		if (passes[*r].order > order)
			break;

	// shift later elements
	uint32_t insertedItem = passId;
	do {
		uint32_t displacedItem = *r;
		*r = insertedItem;
		r++;
		insertedItem = displacedItem;
	} while (insertedItem != ~0u);

	dword408[(int)passType] |= 1 << passId;
	if (sortMode == 0)
		depthSortedPasses |= 1 << passId;

	return passId;
}

void RenderPasses::setRenderPassDebugName(uint32_t passId, const char *name) {
	strncpy(passes[passId].name, name, 64);
}

uint32_t RenderPasses::allocFuncIndex(const char *name) {
	// find zero bit
	uint32_t i = 0, active = activeFuncBitfield;
	while (active & 1) {
		active >>= 1;
		i++;
	}
	if (i == 32)
		return 0;

	// assign defaults on that column across function sets
	for (uint32_t funcSet=0; funcSet<20; funcSet++) {
		drawers[funcSet].func[i] = &defaultDrawer;
		comparators[funcSet].func[i] = &defaultComparator;
	}

	// mark in use
	activeFuncBitfield |= 1 << i;
	return i;
}

void RenderPasses::sort(DrawableList *list, int passId) {
	RenderPass& pass = passes[passId];
	if (pass.sortMode == 0) {
		list->sortSimple();
	} else if (pass.sortMode == 1) {
		list->sortWithFunc(&comparators[pass.funcSetIndex], pass.funcSetIndex);
	}
}

void RenderPasses::draw(DrawableList *list, int passId) {
	RenderPass& pass = passes[passId];

	cdc::PIXTracker::StartMarker(pass.name);
	list->draw(&drawers[pass.funcSetIndex], pass.funcSetIndex);
	cdc::PIXTracker::EndMarker();
}

void RenderPasses::sortAndDraw(
	RenderPassType passType,
	DrawableListsAndMasks *lists,
	CommonRenderDevice *renderDevice,
	uint32_t mask)
{
	uint32_t *reqPass;
	if (passType == kShadowPass)
		reqPass = requestedPassesShadow;
	else
		reqPass = requestedPassesScene;

	uint32_t priorPasses = 0;
	for (uint32_t passId; (passId = *reqPass) != -1; reqPass++) {
		if (mask & (1 << passId)) {
			RenderPass *pass = &passes[passId];
			DrawableListsAndMasks *activeLists =
				pass->passType == kLightPass ? lists->lightPasses : lists;
			if (activeLists) {
				IRenderPassCallback *callbacks = pass->callbacks;
				DrawableList *list = &lists->drawableLists[lists->compactIndices[passId]];
				uint32_t drawableCount = list->itemCount;
				if (!callbacks || callbacks->pre(renderDevice, passId, drawableCount, priorPasses)) {
					priorPasses |= 1 << passId;
					if (drawableCount) {
						sort(list, passId);
						draw(list, passId);
					}
					if (callbacks)
						callbacks->post(renderDevice, passId);
				} else {
					std::string name = pass->name;
					name += " (empty)";
					cdc::PIXTracker::StartMarker(name.c_str());
					cdc::PIXTracker::EndMarker();
				}
			}
		}
		// renderDevice->sub_51EB20(passId);
	}
	// renderDevice->sub_51EB20(32);
}

DrawableListsAndMasks *RenderPasses::createDrawableLists(RenderPassType passType, uint32_t mask, LinearAllocator *linear) {
	if (mask /* & ... */)
		return new (linear, 0, true) DrawableListsAndMasks(this, passType, mask, linear);
	return nullptr;
}

void DrawableList::add(IRenderDrawable *drawable) {
	itemCount++;
	auto item = new (linear, 2, true) DrawableItem();
	item->drawable = drawable;
	item->next = nullptr;
	if (last)
		last->next = item;
	last = item;
	if (!first)
		first = item;
}

void DrawableList::sortSimple() {
	// I don't see a point trying to sort them while in a linked-list
	std::vector<DrawableItem*> items;
	for (DrawableItem *item = first; item; item = item->next)
		items.push_back(item);

	// sort the vector
	std::sort(items.begin(), items.end(), [](DrawableItem *a, DrawableItem *b) {
		return a->drawable->sortZ > b->drawable->sortZ;
	});

	// relink the list
	DrawableItem **tracer = &first;
	for (auto item : items) {
		*tracer = item;
		tracer = &item->next;
	}
	*tracer = nullptr;
}

void DrawableList::sortWithFunc(CompareFunctionSet *funcSet, uint32_t funcSetIndex) {
	// TODO
}

void DrawableList::draw(RenderFunctionSet *funcSet, uint32_t funcSetIndex) {
	IRenderDrawable *drawable;
	DrawableItem *item = first;

	for (IRenderDrawable *prev = nullptr; item; prev = drawable) {
		drawable = item->drawable;
		bool matchPrev = prev &&
			prev->typeID == drawable->typeID &&
			prev->getVtable() == drawable->getVtable();
		funcSet->func[drawable->typeID](funcSetIndex, drawable, matchPrev ? prev : nullptr);
		item = item->next;
	}
}

void DrawableList::absorbToBack(DrawableList& other) {
	if (this == &other || other.itemCount == 0)
		return;

	if (last)
		last->next = other.first;
	else
		first = other.first;
	last = other.last;
	itemCount += other.itemCount;
	other.itemCount = 0;
	other.first = 0;
	other.last = 0;
}

size_t DrawableList::size() const {
	size_t n = 0;
	for (DrawableItem *item = first; item; item = item->next)
		n++;
	return n;
}

DrawableListsAndMasks::DrawableListsAndMasks(
	RenderPasses *renderPasses,
	RenderPassType passType,
	uint32_t passMask,
	LinearAllocator *linear)
:
	renderPasses(renderPasses),
	passType(passType)
{
	// TODO: passMask &= renderPasses->dword408[(int)passType];
	passMask8 = passMask;
	passMaskC = passMask;
	lightPasses = nullptr;

	uint32_t listCount = 0;
	if (passMask) {
		uint8_t *compactPtr = compactIndices;
		do {
			*compactPtr++ = listCount;
			if (passMask & 1)
				listCount++;
			passMask >>= 1;
		} while (passMask);
	}

	for (uint32_t i = 0; i < 32; i++)

	if (listCount)
		drawableLists = new (linear, 0, true) DrawableList[listCount];
	else
		drawableLists = nullptr;

	for (uint32_t i = 0; i<listCount; i++)
		drawableLists[i] = { linear, 0, 0, 0 };
}

void DrawableListsAndMasks::add(IRenderDrawable *drawable, uint32_t insertMask) {
	DrawableList *list = drawableLists;
	uint32_t advanceMask = passMask8;
	insertMask &= passMaskC;
	while (insertMask) {
		if (insertMask & 1)
			list->add(drawable);

		if (advanceMask & 1)
			list++;

		insertMask >>= 1;
		advanceMask >>= 1;
	}
}

DrawableList *DrawableListsAndMasks::listForPass(uint32_t passId) {
	// TODO
	return nullptr;
}

void DrawableListsAndMasks::absorbToBack(DrawableListsAndMasks& other) {
	uint32_t thisMask = this->passMask8;
	uint32_t otherMask = other.passMask8;
	DrawableList *thisList = this->drawableLists;
	DrawableList *otherList = other.drawableLists;

	while (thisMask || otherMask) {

		if (thisMask & otherMask & 1)
			thisList->absorbToBack(*otherList);

		if (thisMask & 1)
			thisList++;

		if (otherMask & 1)
			otherList++;

		thisMask >>= 1;
		otherMask >>= 1;
	}
}

}
