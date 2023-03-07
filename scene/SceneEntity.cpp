#include "Scene.h"
#include "SceneCell.h"
#include "SceneCellGroup.h"
#include "SceneEntity.h"

namespace cdc {

SceneEntity::SceneEntity(Scene *scene) : scene(scene) {
	scene->AddEntity(this);
}

void SceneEntity::setMatrix(Matrix& newMatrix) {
	matrix = newMatrix;
	// TODO
}

Matrix& SceneEntity::getMatrix() {
	return matrix;
}

void SceneEntity::setDrawable(IDrawable *newDrawable) {
	drawable = newDrawable;
	// TODO
}

IDrawable *SceneEntity::getDrawable() {
	return drawable;
}

void SceneEntity::setCellGroup(ISceneCellGroup *newICellGroup) {
	auto newCellGroup = static_cast<SceneCellGroup*>(newICellGroup);
	if (sceneCellGroup != newCellGroup) {
		// TODO
		sceneCellGroup = newCellGroup;
		UpdateData(true);
	}
}

void SceneEntity::UpdateData(bool reInitCell) {
	// HACK
	auto cell = sceneCellGroup->cellByIndex(0);
	if (reInitCell) {
		auto subCell = &cell->subCells[0];
		bool alreadyPresent = false;
		for (auto ent : subCell->entities)
			if (ent == this)
				alreadyPresent = true;
		if (!alreadyPresent)
			subCell->entities.push_back(this);
	}
}

}
