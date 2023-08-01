#pragma once
#include <cstdint>
#include "cdcMath/Math.h"
#include "cdcSound/SoundOwner.h"

namespace dtp {

struct SoundPlex;

struct sfxmarker {
	cdc::Vector3 position; // 0
	// cdc::Euler rotation; // 10
	cdc::Vector rotation; // 10
	uint32_t dword20; // m_unit or UniqueID or m_streamLayer
	cdc::SoundHandle *soundHandles;    // 24
	SoundPlex **soundData; // 28
	uint32_t numSounds;    // 2C
	uint32_t numTriggers;  // 30
	SoundPlex *sounds;     // 34
	void *triggers;        // 38
};

static_assert(sizeof(sfxmarker) == 0x3C);

}
