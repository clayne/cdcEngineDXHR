#pragma once
#include <unordered_map>
#include <vector>

namespace cdc {

struct StreamDeclCacheKey {
	uint32_t hashB;
	uint32_t hashA;
	bool operator==(StreamDeclCacheKey const& other) const {
		return hashB == other.hashB && hashA == other.hashA;
	}
};

}

template<>
struct std::hash<cdc::StreamDeclCacheKey> {
	std::size_t operator()(cdc::StreamDeclCacheKey const& k) const noexcept {
		return k.hashB;
	}
};

namespace cdc {

class PCDX11ConstantBuffer;
class PCDX11RenderDevice;
class PCDX11ShaderBinary;
class PCDX11StreamDecl;
struct VertexDecl;
struct ShaderInputSpec;

struct PCDX11StreamDeclCache {

	PCDX11StreamDeclCache(PCDX11RenderDevice *renderDevice) :
		renderDevice(renderDevice)
	{}
	~PCDX11StreamDeclCache();

	PCDX11RenderDevice *renderDevice;
	std::unordered_map<StreamDeclCacheKey, PCDX11StreamDecl *> cache;
	std::vector<PCDX11ConstantBuffer*> cbs;

	PCDX11StreamDecl *buildStreamDecl(
		VertexDecl *layoutA,
		PCDX11ShaderBinary *shaderSub);

	PCDX11StreamDecl *buildStreamDecl(
		VertexDecl *layoutA,
		ShaderInputSpec *layoutB,
		bool a4,
		PCDX11ShaderBinary *shaderSub);
};

}
