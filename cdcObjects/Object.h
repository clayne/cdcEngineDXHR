#pragma once
#include "cdcWorld/cdcWorldTypes.h"

namespace cdc {

class AnimGraphRequest;
class RenderMesh;
class ObjectBlobAnim;

struct ObjectBlobSub_DTPData {
	uint32_t dword0;
	uint32_t dword4;
	uint32_t dword8;
	uint32_t dwordC;
	float float10_lod1_distance_maybe;
	float float14_lod2_distance_maybe;
	uint8_t byte18_lod1_rmi_selector_maybe;
	uint8_t byte19_lod2_rmi_selector_maybe;
	uint8_t f1A[6];
	float float20;
	uint16_t word24;
	uint8_t byte26;
	uint8_t f27;
	float fl28;
	uint8_t f2C[4];
	uint16_t word30;
	uint8_t f32[6];
	uint16_t word38;
	uint8_t f3A[2];
	uint32_t dword3C;
	uint16_t word40;
	uint16_t word42;
	uint32_t dword44;
	uint32_t dword48;
	AnimGraphRequest **pAnimGraphReq4C;
	uint32_t numAnimGraphs54;
	struct { AnimGraphRequest *req; uint32_t unk4; } *animgraphs54;
	uint32_t dword58;
	uint32_t dword5C;
	uint32_t dword60;
	int field_64_IK;
	uint8_t gap68[4];
	int field_6C;
	uint8_t gap70[8];
	int field_78_cloth;
	int field_7C_softBody;
	uint32_t dword80;
	uint32_t hasSoundMaybe;
	uint32_t dword88;
	uint32_t dword8C;
	uint32_t dword90;
	uint32_t dword94_softAccessory;
	uint32_t dword98;
	uint32_t dword9C;
	uint32_t dwordA0;
	uint32_t dwordA4;
	uint32_t dwordA8;
	uint32_t dwordAC;
	uint32_t dwordB0;
	uint32_t dwordB4;
	uint32_t dwordB8;
	uint32_t dwordBC; // important for drms storing general purpose data
	uint32_t dwordC0;
	ObjectBlobAnim **animationsC4;
	uint32_t dwordC8;
	uint32_t dwordCC;
	uint32_t dwordD0;
	uint32_t dwordD4;
	uint32_t dwordD8;
	uint32_t dwordDC;
	uint32_t dwordE0;
	uint32_t dwordE4;
	uint32_t dwordE8;
	uint32_t dwordEC;
	uint32_t dwordF0;
	uint32_t dwordF4;
	uint32_t dwordF8;
	uint16_t numModels; // FC
	uint8_t fFE[2];
	dtp::Model **models;
	uint32_t dword104;
	uint32_t dword108;
	uint32_t dword10C;
	uint32_t dword110;
	uint32_t dword114;
	uint16_t baseObject_objectListIndex;
	uint16_t dword11A;
	uint32_t dword11C;
	uint32_t dword120;
	uint32_t dword124;
	uint32_t dword128;
	uint32_t hasDeferredLightMaybe;
};

struct ObjectBlob {
	ObjectBlobSub_DTPData *dtpData;
	uint16_t *punsigned4;
	uint32_t dword8;
	uint32_t dwordC;
	uint32_t dword10;
	uint32_t dword14; // own object list id
	uint32_t dword18;
	void **dword1C;
	uint16_t word20;
	uint8_t f22[10];
	uint32_t dword2C;
	uint32_t dword30;
	uint16_t word34;
	uint16_t numModels;
	uint16_t uberFake1;
	uint16_t uberFake2;
	uint16_t word3C;
	uint8_t f3E[2];
	dtp::Model **models;
	uint32_t dword44;
	uint32_t dword48;
	uint32_t dword4C;
	uint32_t dword50;
	uint32_t dword54;
	uint32_t dword58;
	uint32_t name;
	uint32_t dword60;
	uint16_t word64;
	uint16_t word66;
	uint16_t word68;
	uint16_t word6A;
	uint32_t dword6C_lod1_distance_maybe;
	uint32_t dword70_lod2_distance_maybe;
	uint8_t byte74_lod1_model_selector_maybe;
	uint8_t byte75_lod2_model_selector_maybe;
	uint16_t word76;
	uint32_t dword78;
	uint32_t dword7C;
	uint32_t dword80;
	uint32_t dword84;
	uint32_t dword88;
};

}
