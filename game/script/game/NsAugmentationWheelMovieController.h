#pragma once
#include <cstdint>
#include "../../ui/FakeScaleform/fakescaleform.h"
#include "NsScaleformMovieController.h"

class NsAugmentationWheelMovieController : public NsScaleformMovieController {
public:
	NsAugmentationWheelMovieController(cdc::ScriptType *ty) : NsScaleformMovieController(ty) {}

	int32_t ProcessMovieFunction(const char *name, GFxValue *args, uint32_t numArgs) override;
};
