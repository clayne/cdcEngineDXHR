#include <unordered_set>
#include "IDrawable.h"
#include "Scene.h"
#include "SceneCell.h"
#include "SceneCellGroup.h"
#include "SceneEntity.h"

namespace cdc {

Scene::Scene(CommonRenderDevice *renderDevice) : // line 199
	renderDevice(renderDevice), sceneCell(nullptr)
{
	// TODO
	sceneCellGroups.push_back(new SceneCellGroup(this, SceneCellGroup::kGlobal));
	sceneCellGroups.back()->allocateCells();
	sceneCellGroups.push_back(new SceneCellGroup(this, SceneCellGroup::kExterior));
	sceneCellGroups.back()->allocateCells();
}

SceneCellGroup *Scene::GetCellGroup(uint32_t index) { // line 1540
	return sceneCellGroups[index];
}

SceneEntity *Scene::CreateEntity() { // line 1570
	return new SceneEntity(this/*, TODO*/);	
}

bool Scene::SetupView(/*TODO*/) { // line 524
	// TODO
	return true;
}

void Scene::TraverseForView(/*TODO*/) { // line 648
	// TODO
	
	// HACK: just draw everything
	std::unordered_set<SceneEntity*> entities;
	for (auto group : sceneCellGroups) {
		for (uint32_t i = 0; i < group->getCellCount(); i++) {
			auto *cell = group->cellByIndex(i);
			for (uint16_t j = 0; j < cell->numSubCells; j++) {
				auto *subcell = &cell->subCells[j];
				for (auto ent : subcell->entities)
					entities.insert(ent);
			}
		}
	}

	for (auto ent : entities)
		activeEntities.push_back(ent);
}

void Scene::TraverseForLights(/*TODO*/) { // line 795
	// TODO
}

void Scene::CallCallbacks(/*TODO*/) { // line 1081
	// TODO
}

void Scene::DrawTerrain(/*TODO*/) { // line 1160
	// TODO
}

void Scene::DrawEntities(/*TODO*/) { // line 1207
	// TODO
	for (auto entity: activeEntities) {
		// TODO
		entity->drawable->draw(&entity->matrix, /*TODO*/ 0.0f);
	}
}

void Scene::DoRender() { // line 467
	// TODO
	if (SetupView()) {
		TraverseForView();
		TraverseForLights();
		CallCallbacks();
		DrawTerrain();
		DrawEntities();
	}
	activeEntities.clear();
}

void Scene::Render() { // line 351
	// TODO
	DoRender();
	// TODO
}

void Scene::RenderWithoutCellTracing() { // line 1399
	// TODO
	for (SceneEntity *entity: m_entities) {
		// TODO
		if (entity->drawable)
			entity->drawable->draw(&entity->matrix, /*TODO*/ 0.0f);
	}
}

void Scene::AddEntity(SceneEntity *pEntity) { // line 2541
	pEntity->entityIndex = m_entities.size();
	m_entities.push_back(pEntity);
}

}
