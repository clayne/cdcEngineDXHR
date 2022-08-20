#include <cstdio>
#include "config.h"
#include "stream.h"
#include "StreamUnit.h"
#include "../drm/ResolveObject.h"
#include "../filesystem/FileHelpers.h"
#include "../filesystem/FileSystem.h"
#include "../game/Main.h" // for buildDRMPath
#include "../object/ObjectManager.h" // for readAndParseObjectList

#if ENABLE_IMGUI
#include "../imgui/imgui.h"
#endif

using namespace cdc;

StreamUnit StreamTracker[80];

extern char buildType[16];

struct ObjListEntry {
	char name[128];
	uint16_t *data;
};

struct ObjList {
	uint32_t count;
	ObjListEntry entries[];
};

ObjList *objList;

void buildUnitsUI() {
#if ENABLE_IMGUI
	for (uint32_t i = 0; i < objList->count; i++) {
		ImGui::Text("%s", objList->entries[i].name);
	}
#endif
}

void STREAM_Init() { // 582
	// TODO

	// read object names
	readAndParseObjectList();

	// read unit names
	char objListPath[300];
	sprintf(objListPath, "%s\\objlist.dat", buildType);
	if (getDefaultFileSystem()->getSize(objListPath)) {
		objList = (ObjList*) FSHelper_ReadFile(objListPath);
		uint32_t count = objList->count;
		ObjListEntry *entry = objList->entries;
		uint16_t *data = (uint16_t*)&objList->entries[objList->count];
		while (count--) {
			if (entry->name[0] != '\0') {
				entry->data = data;
				data += 1 + *data;
			}
			entry++;
		}
	}

	// TODO
}

StreamUnit *STREAM_GetStreamUnitWithID(int32_t id) { // 1170
	// HACK
	return &StreamTracker[0];
}

StreamUnit *STREAM_GetAndInitStreamUnitWithID(int32_t id) { // couldn't confirm real name
	auto *unit = STREAM_GetStreamUnitWithID(id);
	// TODO
	return unit;
}

void STREAM_FinishLoad(StreamUnit *unit) { // 1658
	printf("unit->level=%p\n", unit->level);
}

void STREAM_LoadLevelReturn(void *loadData, void *data, void *data2, ResolveObject *resolveObj) { // 1922
	StreamUnit *unit = (StreamUnit*)data2;
	// TODO
	STREAM_FinishLoad(unit);
	// TODO
}

void STREAM_StreamLoadLevelReturn(void *loadData, void *data, void *data2, ResolveObject *resolveObj) { // 1971
	StreamUnit *unit = (StreamUnit*)data2;
	// TODO
	STREAM_FinishLoad(unit);
	// TODO
}

void STREAM_LoadLevel(const char *baseAreaName, StreamUnitPortal *streamPortal, bool loadObjects) { // 2085
	// TODO
	if (true) {
		// TODO
		int32_t i = -1;
		if (streamPortal)
			i = streamPortal->word84;
		StreamUnit *unit = STREAM_GetAndInitStreamUnitWithID(i);
		if (unit) {
			char filename[256];
			buildDRMPath(filename, baseAreaName);
			ResolveObject *resolveObject;
			if (streamPortal) {
				resolveObject = ResolveObject::create(
					filename,
					&STREAM_StreamLoadLevelReturn,
					0,
					unit,
					(void**)&unit->level, // put pointer to blob here
					0,
					0,
					0,
					3 // normal priority
				);
			} else {
				resolveObject = ResolveObject::create(
					filename,
					&STREAM_LoadLevelReturn,
					0,
					unit,
					(void**)&unit->level, // put pointer to blob here
					0,
					0,
					0,
					3 // normal priority
				);
			}
		}
	}
}

// only called from scripting system
void STREAM_LoadLevel(const char *baseAreaName) { // 2171
	// TODO
}

StreamUnit *STREAM_LevelLoadAndInit(const char *baseAreaName) { // 2855
	// TODO
	STREAM_LoadLevel(baseAreaName, nullptr, true);
	// TODO
	return nullptr;
}
