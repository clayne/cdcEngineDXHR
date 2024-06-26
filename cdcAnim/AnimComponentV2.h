#pragma once
#include "cdcAnim/AnimPose.h"

namespace dtp { struct Model; }

class Instance;

namespace cdc {

class AnimPoseNode;
class IAnimGraphNode;
struct BoneSet;
struct Matrix;

class AnimComponentV2 {
	static BoneSet s_allBones;
public:
	IAnimGraphNode *graphOutput = nullptr; // 0
	AnimPoseNode *poseNode = nullptr; // 4
	dtp::Model *model = nullptr; // 20
	Instance *instance; // 24
	Matrix *matrices = nullptr; // 2C, assigned by G2Instance_RebuildTransforms
	float mTimeScale = 1.0f; // 4C
	IAnimGraphNode *firstNode = nullptr; // 54

	AnimComponentV2(Instance *instance);
	void Init(dtp::Model *model);
	void Update(float fFrameTime);
	AnimPoseNode *CreatePoseNode();
	void BuildTransforms();
	void PrePhysics();
	void TriggerStateTransition(uint32_t trigger, uint32_t hackAnimIndex, bool hackLoop);
	void BuildSegTransformForRoot(Matrix&, Matrix&);
	void BuildSegTransforms();
};

}
