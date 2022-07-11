#pragma once
#include "RenderMesh.h"
#include "PCDX11RenderExternalResource.h"

namespace cdc {

class PCDX11RenderDevice;
class PCDX11SimpleStaticIndexBuffer;

class PCDX11RenderModel :
	public RenderMesh,
	public PCDX11RenderExternalResource
{
public:
	uint32_t size;

	MeshHeader *meshHeader;
	Mesh       *mesh;

	uint16_t count0; // 1C
	MeshTab0 *table0; // 20

	uint16_t count1; // 24
	MeshSub *table1; // 28

	// uint32_t bonesMaybe;
	// uint32_t dword30;
	uint32_t indices;
	// PCDX11StaticIndexBuffer *indexBuffer; // 38
	PCDX11SimpleStaticIndexBuffer *indexBuffer; // 38, hack
	MeshTab0Ext16 *tab0Ext16Byte; // uses count0
	MeshTab0Ext128 *tab0Ext128Byte; // uses count0
	uint32_t vsSelect; // 4C

public:
	PCDX11RenderModel(PCDX11RenderDevice *renderDevice, uint32_t size);

	void resFree() override { /*TODO*/ }
	void resFill(void* src, uint32_t size, uint32_t offset) override;
	char *resGetBuffer() override;
	void resConstruct() override;
	// void resMethod10();
	// dtor

	void mesh18() override { /*TODO*/ }
	void mesh1C() override { /*TODO*/ }
	// void mesh20(RenderFourTuple *a2, RenderFourTuple *a3) { /*TODO*/ }
	void mesh24(float*, float*) override { /*TODO*/ }
	uint32_t getBoneCount() override { /*TODO*/ return 0; }
	void mesh2C() override { /*TODO*/ }
	MeshTab0Ext16 *getTab0Ext16() override { /*TODO*/ return tab0Ext16Byte; }
	MeshTab0Ext128 *getTab0Ext128() override { /*TODO*/ return tab0Ext128Byte; }
	Mesh *getMesh() override { /*TODO*/ return mesh; }
	uint32_t getBoneCountIfEnabled() override { /*TODO*/ return 0; }
	void getSkeleton() override { /*TODO*/ }
};

}
