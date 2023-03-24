#pragma once
#include <vector>
#include "rendering/CommonScene.h"
#include "rendering/Culling/Primitives.h"
#include "IScene.h"

namespace cdc {

class CommonRenderDevice;
class SceneEntity;
class SceneCell;
class SceneCellGroup;
class SceneEntity;

class Scene : public IScene {
	CommonRenderDevice *renderDevice; // 4
	std::vector<SceneCellGroup*> sceneCellGroups; // 98
	SceneCell *sceneCell; // B8
	std::vector<SceneEntity*> m_entities; // BC
	RenderViewport m_viewport; // E0
	CullingFrustum cullingFrustum; // 1D0
	std::vector<SceneEntity*> activeEntities; // 2F0

public:
	Scene(CommonRenderDevice *renderDevice);

	// for DoRender
	bool SetupView(/*TODO*/);
	void TraverseForView(/*TODO*/);
	void TraverseForLights(/*TODO*/);
	void CallCallbacks(/*TODO*/);
	void DrawTerrain(/*TODO*/);
	void DrawEntities(/*TODO*/);
	void DoRender();

	// called from SceneEntity ctor
	void AddEntity(SceneEntity *pEntity);

	// 43 methods
	void Render() override;
	void RenderWithoutCellTracing(RenderViewport& viewport) override;
	SceneCellGroup *GetCellGroup(uint32_t index) override;
	SceneEntity *CreateEntity() override;
};

}
