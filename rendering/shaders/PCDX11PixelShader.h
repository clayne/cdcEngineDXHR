#pragma once
#include "../PCDX11AsyncCreateResource.h"
#include "../PCDX11InternalResource.h"
#include "PCDX11Shader.h"

struct ID3D11PixelShader;

namespace cdc {

class PCDX11PixelShader :
	public PCDX11AsyncCreateResource,
	public PCDX11InternalResource
{
public:
	ID3D11PixelShader *m_d3dShader;
	PCDX11ShaderBinary m_sub;

	static PCDX11PixelShader *create(
		char *blob, bool takeCopy, /*shaderManagerData,*/
		bool *isWrappedPtr, bool isWrapped)
	{
		PCDX11ShaderBinary sub(blob, /*takeCopy=*/false, isWrapped);
		// TODO
		auto *shader = new PCDX11PixelShader(blob, takeCopy, isWrapped);
		// TODO
		return shader;
	}

	PCDX11PixelShader() {}
	PCDX11PixelShader(ID3D11PixelShader *shader) {
		m_requested = true;
		m_keepWaiting = false;
		m_d3dShader = shader;
	}
	PCDX11PixelShader(char *blob, bool takeCopy, bool isWrapped) :
		PCDX11AsyncCreateResource(),
		PCDX11InternalResource(),
		m_d3dShader(nullptr),
		m_sub(blob, takeCopy, isWrapped)
	{ // copied from PCDX11VertexShader
		internalCreateIfDeviceManagerAgrees();
	}

	void requestShader() {
		m_d3dShader = 0;
		request(0);
	}

	void asyncCreate() override;

	bool internalCreate() override;
	void internalRelease() override;
};

}
