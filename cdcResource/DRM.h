#pragma once
#include <stdint.h>

namespace cdc {

class ResolveObject;

enum class ContentType : uint8_t {
	Generic = 0,
	Empty = 1,
	Animation = 2,
	RenderResource = 5,
	FMODSoundBank = 6,
	DTPData = 7,
	Script = 8,
	ShaderLib = 9,
	Material = 10,
	Object = 11,
	RenderMesh = 12,
	CollisionMesh = 13,
	StreamGroupList = 14,
	AnyType = 15
};

struct SectionRecord {
	struct Entry { // a mini section header
		uint32_t domainID;
		uint16_t contentType;
		uint32_t sectionID;
	};

	Entry *m_pEntry;
	uint32_t m_numEntries;

	void Release(ResolveObject *pObject);
	void ReleaseEntry(uint32_t index, ResolveObject *pObject);
};

struct DRMSectionHeader { // cdc::SectionInfo
	uint32_t payloadSize;
	uint8_t type;
	uint8_t  unknown05;
	uint16_t unknown06; // versionID
	struct {
		uint32_t singleFlag : 1; // hasDebugInfo
		uint32_t allocFlags : 7; // resourceType
		uint32_t relocSize : 24;
	};
	uint32_t id;
	uint32_t languageBits;
};

static_assert(sizeof(void*) == 4, "only 32-bit build is supported");
static_assert(sizeof(DRMSectionHeader) == 20);

struct DRMHeader {
	uint32_t version;
	uint32_t dependencyDrmListSize;
	uint32_t dependencyObjListSize;
	uint32_t unknown0C;
	uint32_t unknown10;
	uint32_t flags;
	uint32_t sectionCount;
	uint32_t rootSection;
};

}
