#include "AnimFragmentNode.h"
// #include "cdcAnim/AnimPose.h"
// #include "cdcWorld/cdcWorldTypes.h"

namespace cdc {

AnimFragmentNode::AnimFragmentNode(AnimComponentV2 *ac, AnimFragment *fragment) :
	AnimPipelineNode(ac, /*numInputs=*/ 0, /*addToChain=*/ true),
	fragment(fragment)
{
	// TODO
}

void AnimFragmentNode::SetAnimData(uint16_t anim) {
	// TODO
}

void AnimFragmentNode::Activate(Instance*, float) {
	// TODO
}

void AnimFragmentNode::Deactivate(Instance*) {
	// TODO
}

float AnimFragmentNode::GetNextSyncPoint(void*) {
	// TODO
	return 0.0f;
}

void AnimFragmentNode::GetSyncInfo(void*, float *elapsedOut, float *durationOut) {
	// TODO
}

void AnimFragmentNode::Update(void*) {
	// TODO
}

void AnimFragmentNode::PrePhysics(AnimContextData *data) {
	if (data->weight >= 0.00001) {
		if (data->weight != 0.0) {
			if (fragment) { // AnimFragment
				if (fragment->mKeyCount == 1)
					DecompressPose(data);
				else
					DecompressFrame(data);
			} else {
				// TODO
			}
		}
	}
}

void AnimFragmentNode::EnsureIDMap(dtp::Model *model) {
	// TODO: keep these maps in a hashmap
	if (boneMap &&
		animID == boneMap->animID &&
		boneMapHash == boneMap->boneMapHash)
		return;

	if (boneMap)
		delete boneMap;
	boneMap = GenerateIDMaps(fragment, model);
	
}

void AnimFragmentNode::DecompressPose(AnimContextData *data) {
	EnsureIDMap(data->model);
	// TODO
}

void AnimFragmentNode::DecompressFrame(AnimContextData *data) {
	EnsureIDMap(data->model);
	// TODO
}

}
