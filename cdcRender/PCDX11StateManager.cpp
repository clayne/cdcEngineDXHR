#define NOMINMAX
#include <d3d11_1.h>
#undef near // can you guess why these need undefining?
#undef far
#include <cfloat>
#include <algorithm>
#include "cdcSys/Assert.h"
#include "buffers/PCDX11ConstantBuffer.h"
#include "buffers/PCDX11IndexBuffer.h"
#include "buffers/PCDX11UberConstantBuffer.h"
#include "buffers/PCDX11VertexBuffer.h"
#include "PCDX11StateManager.h"
#include "PCDX11StreamDecl.h"
#include "shaders/PCDX11ComputeShader.h"
#include "shaders/PCDX11PixelShader.h"
#include "shaders/PCDX11VertexShader.h"
#include "surfaces/PCDX11BaseTexture.h"
#include "surfaces/PCDX11DepthBuffer.h"
#include "surfaces/PCDX11RenderTarget.h"

bool operator==(D3D11_RASTERIZER_DESC const& a, D3D11_RASTERIZER_DESC const& b) {
	return
		a.FillMode == b.FillMode &&
		a.CullMode == b.CullMode &&
		a.FrontCounterClockwise == b.FrontCounterClockwise &&
		a.DepthBias == b.DepthBias &&
		a.DepthBiasClamp == b.DepthBiasClamp &&
		a.SlopeScaledDepthBias == b.SlopeScaledDepthBias &&
		a.DepthClipEnable == b.DepthClipEnable &&
		a.ScissorEnable == b.ScissorEnable &&
		a.MultisampleEnable == b.MultisampleEnable &&
		a.AntialiasedLineEnable == b.AntialiasedLineEnable;
}

bool operator==(D3D11_DEPTH_STENCILOP_DESC const& a, D3D11_DEPTH_STENCILOP_DESC const& b) {
	return
		a.StencilFailOp == b.StencilFailOp &&
		a.StencilDepthFailOp == b.StencilDepthFailOp &&
		a.StencilPassOp == b.StencilPassOp &&
		a.StencilFunc == b.StencilFunc;
}

bool operator==(D3D11_DEPTH_STENCIL_DESC const& a, D3D11_DEPTH_STENCIL_DESC const& b) {
	return
		a.DepthEnable == b.DepthEnable &&
		a.DepthWriteMask == b.DepthWriteMask &&
		a.DepthFunc == b.DepthFunc &&
		a.StencilEnable == b.StencilEnable &&
		a.StencilReadMask == b.StencilReadMask &&
		a.StencilWriteMask == b.StencilWriteMask &&
		a.FrontFace == b.FrontFace &&
		a.BackFace == b.BackFace;
}

namespace cdc {

PCDX11StateManager::PCDX11StateManager() {}

PCDX11StateManager::PCDX11StateManager(ID3D11DeviceContext *deviceContext, ID3D11Device *device) :
	m_deviceContext(deviceContext),
	m_device(device),
	m_indexBufferD3D(nullptr),
	m_pixelShader(nullptr),
	m_dirtySamplersFirst(20),
	m_dirtySamplersLast(0),
	m_dirtyShaderResourcesFirst(20),
	m_dirtyShaderResourcesLast(0)
{
	m_constantBufferVs[0] = nullptr;
	m_uberConstantBuffer[0] = new PCDX11UberConstantBuffer(12);
	m_uberConstantBuffer[1] = new PCDX11UberConstantBuffer(57);
	m_uberConstantBuffer[2] = new PCDX11UberConstantBuffer(4);
	m_uberConstantBuffer[3] = new PCDX11UberConstantBuffer(168);
	m_uberConstantBuffer[4] = new PCDX11UberConstantBuffer(8);
	m_uberConstantBuffer[5] = new PCDX11UberConstantBuffer(7);
	m_uberConstantBuffer[6] = new PCDX11UberConstantBuffer(1);

	memset(m_samplers, 0, 20 * 4);
	memset(m_textures, 0, 20 * 4);
	memset(m_textureViews, 0, 20 * 4);
	memset(m_textureResources, 0, 20 * 4);

	m_stencilParams.m_frontParams = 0xFF00000E;
	m_stencilParams.m_backParams = 0xFF00000E;
	m_stencilParams.m_writeMasks = 0;
	m_stencilParams.m_hiStencil = 0xFFFF;
	m_blendState = 0;
	m_cullMode = ~0u;

	m_dirtyViewport = true;
	m_isSkySphereModeEnabled = true;
	m_isSkySphereMode = false;

	m_projectMatrix = identity4x4;
	m_viewMatrix = identity4x4;
	m_viewProjectMatrix = identity4x4;
	m_worldMatrix = identity4x4;
	m_projectMatrixPtr = nullptr;
}

PCDX11StateManager::~PCDX11StateManager() {
	delete m_uberConstantBuffer[0];
	delete m_uberConstantBuffer[1];
	delete m_uberConstantBuffer[2];
	delete m_uberConstantBuffer[3];
	delete m_uberConstantBuffer[4];
	delete m_uberConstantBuffer[5];
	delete m_uberConstantBuffer[6];
}

void PCDX11StateManager::reset() {
	for (uint32_t i=0; i<20; i++) {
		m_samplers[i] = nullptr;
		m_textureViews[i] = nullptr;
		m_textures[i] = nullptr;
		m_textureResources[i] = nullptr;
		m_samplerFilter[i] = 0;
		m_samplerRepeat[i] = 0;
	}

	// TODO

	m_dirtyShaderResourcesFirst = 0;
	m_dirtySamplersFirst = 0;

	m_renderTarget = nullptr;
	m_depthBuffer = nullptr;
	m_renderTargetStackIndex = 0;

	m_dirtyRasterizerState = true;
	m_dirtyDepthStencilState = true;
	m_dirtyBlendState = true;
	m_dirtyConstantBuffers = true;
	m_dirtyShaderResources = true;
	m_dirtySamplers = true;

	m_topology = 0;
	m_vertexStride = 0;
	m_streamDecl = nullptr;
	m_indexBufferD3D = nullptr;
	m_vertexBufferD3D = nullptr;
	m_pixelShader = nullptr;
	m_vertexShader = nullptr;

	// TODO

	for (int i=0; i<7; i++)
		m_constantBufferPs[i] = nullptr;
	for (int i=0; i<7; i++)
		m_constantBufferVs[i] = nullptr;

	// TODO

	m_alphaThreshold = 1.0f;
	m_materialOpacity = 0.0f;

	m_fogColor[0] = 0.0f;
	m_fogColor[1] = 0.0f;
	m_fogColor[2] = 0.0f;
	m_fogColor[3] = 0.0f;

	m_blendState = 0;
	m_cullMode = ~0u;

	// TODO

	m_dirtyViewport = true;
}

void PCDX11StateManager::setIndexBuffer(PCDX11IndexBuffer *indexBuffer) {
	ID3D11Buffer *buffer = nullptr;
	if (indexBuffer)
		buffer = indexBuffer->getD3DBuffer();

	if (buffer != m_indexBufferD3D) {
		indexBuffer->method_10();
		m_deviceContext->IASetIndexBuffer(buffer, DXGI_FORMAT_R16_UINT, 0);
		m_indexBufferD3D = buffer;
	}
}

void PCDX11StateManager::setVertexBuffer(PCDX11VertexBuffer *vertexBuffer) {
	if (vertexBuffer == nullptr)
		return;

	ID3D11Buffer *buffer = vertexBuffer->getD3DBuffer();
	uint32_t stride = vertexBuffer->getStride();
	uint32_t offset = 0;
	if (buffer != m_vertexBufferD3D || stride != m_vertexStride) {
		vertexBuffer->method14();
		m_deviceContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
		m_vertexBufferD3D = buffer;
		m_vertexStride = stride;
	}
}

void PCDX11StateManager::setStreamDecl(PCDX11StreamDecl *streamDecl) {
	if (streamDecl != m_streamDecl) {
		if (!streamDecl->inputLayout) streamDecl->internalCreate(); // hack
		streamDecl->apply();
		m_streamDecl = streamDecl;
	}
}

void PCDX11StateManager::setPixelShader(PCDX11PixelShader *pixelShader) {
	if (pixelShader != m_pixelShader) {
		if (pixelShader) {
			if (!pixelShader->m_requested)
				pixelShader->requestShader();
			if (pixelShader->m_keepWaiting)
				pixelShader->awaitResource();
			m_deviceContext->PSSetShader(pixelShader->m_d3dShader, nullptr, 0);
		} else {
			m_deviceContext->PSSetShader(nullptr, nullptr, 0);
		}
		m_pixelShader = pixelShader;
	}
}

void PCDX11StateManager::setVertexShader(PCDX11VertexShader *vertexShader) {
	if (vertexShader != m_vertexShader) {
		if (vertexShader) {
			if (!vertexShader->m_requested)
				vertexShader->requestShader();
			if (vertexShader->m_keepWaiting)
				vertexShader->awaitResource();
			m_deviceContext->VSSetShader(vertexShader->m_d3dShader, nullptr, 0);
		} else {
			m_deviceContext->VSSetShader(nullptr, nullptr, 0);
		}
		m_vertexShader = vertexShader;
	}
}

void PCDX11StateManager::setPrimitiveTopology(int topology) {
	// TODO: tesselation stuff
	if (topology != m_topology) {
		m_topology = topology;
		m_deviceContext->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)topology);
	}
}

void PCDX11StateManager::setDepthLayer(bool layer) {
	if (m_isSkySphereModeEnabled && m_isSkySphereMode != layer) {
		m_isSkySphereMode = layer;

		m_dirtyViewport = true;
		updateViewport();
	}
}

void PCDX11StateManager::setCullMode(CullMode cullMode, bool frontIsCounterClockwise) {
	uint32_t compactCullMode = 2 * uint32_t(cullMode) + uint32_t(frontIsCounterClockwise);
	if (compactCullMode != m_cullMode) {
		m_cullMode = compactCullMode;
		m_rasterizerDesc.CullMode =
			cullMode == CullMode::back  /*1*/ ? D3D11_CULL_BACK /*3*/ :
			cullMode == CullMode::front /*2*/ ? D3D11_CULL_FRONT /*2*/ :
			D3D11_CULL_NONE /*0*/;
		m_rasterizerDesc.FrontCounterClockwise = frontIsCounterClockwise;
		m_dirtyRasterizerState = true;
	}
}

void PCDX11StateManager::setDepthState(D3D11_COMPARISON_FUNC comparisonFunc, bool depthWrites) {
	// untested
	D3D11_DEPTH_WRITE_MASK depthWriteMask = depthWrites
		? D3D11_DEPTH_WRITE_MASK_ALL
		: D3D11_DEPTH_WRITE_MASK_ZERO;

	auto& desc = m_depthStencilDesc;
	if (desc.DepthFunc != comparisonFunc || desc.DepthWriteMask != depthWriteMask) {
		desc.DepthFunc = comparisonFunc;
		desc.DepthWriteMask = depthWriteMask;
		m_dirtyDepthStencilState = true;
	}
}

void PCDX11StateManager::setStencil(StencilParams *stencilParams) {

	static const D3D11_COMPARISON_FUNC decodeStencilFunc[] = {
		D3D11_COMPARISON_NEVER,
		D3D11_COMPARISON_LESS,
		D3D11_COMPARISON_EQUAL,
		D3D11_COMPARISON_LESS_EQUAL,
		D3D11_COMPARISON_GREATER,
		D3D11_COMPARISON_NOT_EQUAL,
		D3D11_COMPARISON_GREATER_EQUAL,
		D3D11_COMPARISON_ALWAYS
	};

	static const D3D11_STENCIL_OP decodeStencilOp[] = {
		D3D11_STENCIL_OP_KEEP,
		D3D11_STENCIL_OP_ZERO,
		D3D11_STENCIL_OP_REPLACE,
		D3D11_STENCIL_OP_INCR,
		D3D11_STENCIL_OP_INCR_SAT,
		D3D11_STENCIL_OP_DECR,
		D3D11_STENCIL_OP_DECR_SAT,
		D3D11_STENCIL_OP_INVERT
	};

	if (memcmp(&m_stencilParams, stencilParams, 12) != 0) {
		bool stencilEnable = (stencilParams->m_frontParams & 1) || (stencilParams->m_backParams & 1);
		D3D11_DEPTH_STENCIL_DESC& desc = m_depthStencilDesc;
		desc.StencilEnable = stencilEnable;
		desc.StencilReadMask = stencilParams->m_frontParams >> 24; // byte 3
		desc.StencilWriteMask = stencilParams->m_writeMasks & 0xff; // byte 8
		if (stencilParams->m_frontParams & 1) {
			desc.FrontFace.StencilFunc = decodeStencilFunc[(stencilParams->m_frontParams >> 1) & 7];
			desc.FrontFace.StencilDepthFailOp = decodeStencilOp[(stencilParams->m_frontParams >> 12) & 0xF];
			desc.FrontFace.StencilFailOp = decodeStencilOp[(stencilParams->m_frontParams >> 8) & 0xF];
			desc.FrontFace.StencilPassOp = decodeStencilOp[(stencilParams->m_frontParams >> 4) & 0xF];
		} else {
			desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		}
		if (stencilParams->m_backParams & 1) {
			desc.BackFace.StencilFunc = decodeStencilFunc[(stencilParams->m_backParams >> 1) & 7];
			desc.BackFace.StencilDepthFailOp = decodeStencilOp[(stencilParams->m_backParams >> 12) & 0xF];
			desc.BackFace.StencilFailOp = decodeStencilOp[(stencilParams->m_backParams >> 8) & 0xF];
			desc.BackFace.StencilPassOp = decodeStencilOp[(stencilParams->m_backParams >> 4) & 0xF];
		} else {
			desc.BackFace = desc.FrontFace;
		}

		m_stencilParams = *stencilParams;
		m_dirtyDepthStencilState = true;
	}
}

void PCDX11StateManager::setDepthRange(float minDepth, float maxDepth) {
	(void)minDepth;
	(void)maxDepth;
	// TODO
}

void PCDX11StateManager::setDepthBias(int32_t bias) {
	if (bias != m_rasterizerDesc.DepthBias) {
		m_rasterizerDesc.DepthBias = bias;
		m_dirtyRasterizerState = true;
	}
}

void PCDX11StateManager::setDepthBiasClamp(float biasClamp) {
	if (biasClamp != m_rasterizerDesc.DepthBiasClamp) {
		m_rasterizerDesc.DepthBiasClamp = biasClamp;
		m_dirtyRasterizerState = true;
	}
}

void PCDX11StateManager::setSlopeScaledDepthBias(float slopeScaledDepthBias) {
	if (slopeScaledDepthBias != m_rasterizerDesc.SlopeScaledDepthBias) {
		m_rasterizerDesc.SlopeScaledDepthBias = slopeScaledDepthBias;
		m_dirtyRasterizerState = true;
	}
}

void PCDX11StateManager::setBlendStateAndBlendFactors(
	uint32_t blendState,
	uint8_t alphaThreshold,
	uint32_t blendFactors)
{
	if ((blendState & 0x7000000) == 0x7000000)
		alphaThreshold = 0;

	setAlphaThreshold(alphaThreshold);

	if (blendState != m_blendState) {
		m_blendState = blendState;
		m_dirtyBlendState = true;
		uint32_t shift = 0;
		for (uint32_t i=0; i<4; i++) {
			m_blendFactors[i] = (double)(uint8_t)(blendFactors >> shift);
			shift += 2; // BUG, fixed in Tomb Raider (2013) where shift advances by 8 bit
		}
	}
}

void PCDX11StateManager::setRenderTargetWriteMask(uint32_t mask) {
	mask &= 15;
	if (mask != m_renderTargetWriteMask) {
		m_renderTargetWriteMask = mask;
		m_dirtyBlendState = true;
	}
}

void PCDX11StateManager::setSamplerState(
	uint32_t slot,
	PCDX11BaseTexture *tex,
	uint32_t filter)
{
	// 0..15 => 0..15
	//   257 => 16
	//   258 => 17
	//   259 => 18
	//   260 => 19
	if (slot >= 0x10) slot -= 0xF1;

	uint32_t repeat = tex->wrapMode & 7;

	if (repeat != m_samplerRepeat[slot] || filter != m_samplerFilter[slot]) {
		m_samplerRepeat[slot] = repeat;
		m_samplerFilter[slot] = filter;

		uint32_t maxAnisotropy = 1;
		D3D11_FILTER d3dfilter; 
		switch (filter) {
			case 1:
				d3dfilter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
				break;
			case 2:
				d3dfilter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				break;
			case 3: // 1
			case 4: // 2
			case 6: // 4
			case 10: // 8
			case 18: // 16
				d3dfilter = D3D11_FILTER_ANISOTROPIC;
				maxAnisotropy = filter - 2;
				break;
			default:
				d3dfilter = D3D11_FILTER_MIN_MAG_MIP_POINT;
				break;
		}

		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = d3dfilter;
		samplerDesc.AddressU = (repeat & 1) ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = (repeat & 2) ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = (repeat & 4) ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MipLODBias = 0.0;
		samplerDesc.MaxAnisotropy = maxAnisotropy;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;
		samplerDesc.MinLOD = -FLT_MAX;
		samplerDesc.MaxLOD = FLT_MAX;

		if (slot == 14) {
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
			samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		}

		// TODO: cache samplers
		ID3D11SamplerState *samplerState;
		m_device->CreateSamplerState(&samplerDesc, &samplerState);

		if (samplerState != m_samplers[slot]) {
			m_dirtySamplersFirst = std::min<uint8_t>(m_dirtySamplersFirst, slot);
			m_dirtySamplersLast = std::max<uint8_t>(m_dirtySamplersLast, slot);
			m_dirtySamplers = true;
			m_samplers[slot] = samplerState;
		}
	}
}

void PCDX11StateManager::setTextureAndSampler(
	uint32_t slot,
	PCDX11BaseTexture *tex,
	uint32_t filter,
	float unknown)
{
	// 0..15 => 0..15
	//   257 => 16
	//   258 => 17
	//   259 => 18
	//   260 => 19
	uint32_t originalSlot = slot;
	if (slot >= 0x10) slot -= 0xF1;
	if (!m_device) return;

	// TODO: logic to avoid binding a texture currently used as a rendertarget
	if (tex != m_textures[slot]) {
		if (auto srv = tex ? tex->createShaderResourceView() : nullptr) {
			m_textureViews[slot] = srv;
			if (filter > tex->maxFilter)
				filter = tex->maxFilter;
			setSamplerState(originalSlot, tex, filter);
			m_textureResources[slot] = tex->getTextureResource();
		} else {
			m_textureViews[slot] = nullptr;
			m_textureResources[slot] = nullptr;
		}
		m_dirtyShaderResourcesFirst = std::min<uint8_t>(m_dirtyShaderResourcesFirst, slot);
		m_dirtyShaderResourcesLast = std::max<uint8_t>(m_dirtyShaderResourcesLast, slot);
		m_dirtyShaderResources = true;
		m_textures[slot] = tex;
	}
}

void PCDX11StateManager::setPsConstantBuffer(uint32_t slot, PCDX11ConstantBuffer *cb) {
	if (cb != m_constantBufferPs[slot]) {
		ID3D11Buffer *buffer = cb->getBuffer();
		m_deviceContext->PSSetConstantBuffers(slot, 1, &buffer);
		m_constantBufferPs[slot] = cb;
	}
}

void PCDX11StateManager::setVsConstantBuffer(uint32_t slot, PCDX11ConstantBuffer *cb) {
	if (cb != m_constantBufferVs[slot]) {
		ID3D11Buffer *buffer = cb->getBuffer();
		m_deviceContext->VSSetConstantBuffers(slot, 1, &buffer);
		m_constantBufferVs[slot] = cb;
	}
}

void PCDX11StateManager::setCommonConstantBuffers() {
	setVsConstantBuffer(0, m_uberConstantBuffer[0]); // WorldBuffer
	setVsConstantBuffer(1, m_uberConstantBuffer[3]); // SkinningBuffer
	setVsConstantBuffer(2, m_uberConstantBuffer[1]); // SceneBuffer
	// 3 StreamDeclBuffer
	// 4 MaterialBuffer
	setVsConstantBuffer(5, m_uberConstantBuffer[4]);
	setVsConstantBuffer(6, m_uberConstantBuffer[5]);

	setPsConstantBuffer(0, m_uberConstantBuffer[0]); // WorldBuffer
	setPsConstantBuffer(1, m_uberConstantBuffer[2]); // DrawableBuffer
	setPsConstantBuffer(2, m_uberConstantBuffer[1]); // SceneBuffer
	// 3 MaterialBuffer
	setPsConstantBuffer(5, m_uberConstantBuffer[4]); // LightBuffer / InstanceBuffer
	setPsConstantBuffer(6, m_uberConstantBuffer[5]);

	// setHsConstantBuffer(0, m_uberConstantBuffer[0]);
	// setHsConstantBuffer(1, m_uberConstantBuffer[6]);
	// setHsConstantBuffer(2, m_uberConstantBuffer[1]);

	// setDsConstantBuffer(0, m_uberConstantBuffer[0]);
	// setDsConstantBuffer(1, m_uberConstantBuffer[6]);
	// setDsConstantBuffer(2, m_uberConstantBuffer[1]);
}

PCDX11UberConstantBuffer& PCDX11StateManager::accessCommonCB(uint32_t i) {
	m_dirtyUberCBs[i] = true;
	m_dirtyConstantBuffers = true;
	return *m_uberConstantBuffer[i];
}

void PCDX11StateManager::setAlphaThreshold(uint8_t threshold) {
	float alphaThreshold = threshold / 255.0;
	if (m_alphaThreshold != alphaThreshold) {
		m_alphaThreshold = alphaThreshold;
		float row[4] = {alphaThreshold, 0.0f, 0.0f, 0.0f};
		accessCommonCB(2).assignRow(3, row, 1); // DrawableBuffer::AlphaThreshold
	}
}

void PCDX11StateManager::setFogColor(float *color) {
	if (m_fogColor[0] != color[0] ||
		m_fogColor[1] != color[1] ||
		m_fogColor[2] != color[2] ||
		m_fogColor[3] != color[3])
	{
		m_fogColor[0] = color[0];
		m_fogColor[1] = color[1];
		m_fogColor[2] = color[2];
		m_fogColor[3] = color[3];
		accessCommonCB(2).assignRow(0, color, 1); // DrawableBuffer::FogColor
	}
}

void PCDX11StateManager::setOpacity(float opacity) {
	if (m_materialOpacity != opacity) {
		m_materialOpacity = opacity;
		float row[4] = {opacity, 0.0f, 0.0f, 0.0f};
		accessCommonCB(2).assignRow(2, row, 1); // DrawableBuffer::MaterialOpacity
	}
}

void PCDX11StateManager::setWorldMatrix(Matrix& worldMatrix) {
	if (m_worldMatrix != worldMatrix) {
		m_worldMatrix = worldMatrix;
		m_dirtyWorldMatrix = true;
	}
}

void PCDX11StateManager::setViewMatrix(Matrix& viewMatrix) {
	if (m_viewMatrix != viewMatrix) {
		m_viewMatrix = viewMatrix;
		m_dirtyViewMatrix = true;
	}
}

void PCDX11StateManager::setProjectMatrix(Matrix& projectMatrix) {
	if (m_projectMatrix != projectMatrix) {
		m_projectMatrix = projectMatrix;
		m_dirtyProjectMatrix = true;
	}
}

void PCDX11StateManager::setProjectMatrixPtr(Matrix* projectMatrixPtr) {
	if (m_projectMatrixPtr != projectMatrixPtr) {
		m_projectMatrixPtr = projectMatrixPtr;
		m_dirtyProjectMatrix = true;
	}
}

void PCDX11StateManager::updateMatrices() {
	if (m_dirtyWorldMatrix)
		accessCommonCB(0).assignMatrix(4, m_worldMatrix); // WorldBuffer::World

	if (m_dirtyViewMatrix)
		accessCommonCB(1).assignMatrix(0, m_viewMatrix); // SceneBuffer::View

	if (m_dirtyViewMatrix || m_dirtyProjectMatrix) {

		Matrix *projectMatrix = &m_projectMatrix;
		if (m_projectMatrixPtr)
			projectMatrix = m_projectMatrixPtr;
		// TODO: build project matrix right here depending on byte5E9

		m_viewProjectMatrix = *projectMatrix * m_viewMatrix;

		accessCommonCB(0).assignMatrix(8, m_viewProjectMatrix); // WorldBuffer::ViewProject

	}

	if (m_dirtyWorldMatrix || m_dirtyViewMatrix || m_dirtyProjectMatrix) {

		Matrix worldViewProject = m_viewProjectMatrix * m_worldMatrix;

		accessCommonCB(0).assignMatrix(0, worldViewProject);

		m_dirtyWorldMatrix = false;
		m_dirtyViewMatrix = false;
		m_dirtyProjectMatrix = false;
	}
}

void PCDX11StateManager::updateRasterizerState() {
	
	if (m_dirtyRasterizerState) {
		ID3D11RasterizerState *rasterizerState;
		if (auto it = m_rasterizerStates.find(m_rasterizerDesc); it != m_rasterizerStates.end())
			rasterizerState = it->second;
		else {
			m_device->CreateRasterizerState(
				&m_rasterizerDesc,
				&rasterizerState);
			m_rasterizerStates[m_rasterizerDesc] = rasterizerState;
		}

		m_deviceContext->RSSetState(rasterizerState);

		m_dirtyRasterizerState = false;
	}
}

void PCDX11StateManager::updateDepthStencilState() {

	if (m_dirtyDepthStencilState) {
		ID3D11DepthStencilState *depthStencilState;
		if (auto it = m_depthStencilStates.find(m_depthStencilDesc); it != m_depthStencilStates.end())
			depthStencilState = it->second;
		else {
			m_device->CreateDepthStencilState(
				&m_depthStencilDesc,
				&depthStencilState);
			m_depthStencilStates[m_depthStencilDesc] = depthStencilState;
		}

		m_deviceContext->OMSetDepthStencilState(
			depthStencilState,
			(m_stencilParams.m_frontParams >> 16) & 0xff // byte 2
		);

		m_dirtyDepthStencilState = false;
	}
}

void PCDX11StateManager::updateBlendState() {

	D3D11_BLEND_OP blendOps[] = {
		D3D11_BLEND_OP_ADD,
		D3D11_BLEND_OP_SUBTRACT,
		D3D11_BLEND_OP_REV_SUBTRACT,
		D3D11_BLEND_OP_MIN,
		D3D11_BLEND_OP_MAX
	};

	D3D11_BLEND colorBlends[] = {
		D3D11_BLEND_ZERO,
		D3D11_BLEND_ONE,
		D3D11_BLEND_SRC_COLOR,
		D3D11_BLEND_INV_SRC_COLOR,
		D3D11_BLEND_SRC_ALPHA,
		D3D11_BLEND_INV_SRC_ALPHA,
		D3D11_BLEND_DEST_COLOR,
		D3D11_BLEND_INV_DEST_COLOR,
		D3D11_BLEND_DEST_ALPHA,
		D3D11_BLEND_INV_DEST_ALPHA,
		D3D11_BLEND_BLEND_FACTOR,
		D3D11_BLEND_INV_BLEND_FACTOR
	};

	D3D11_BLEND alphaBlends[] = {
		D3D11_BLEND_ZERO,
		D3D11_BLEND_ONE,
		D3D11_BLEND_SRC_ALPHA,
		D3D11_BLEND_INV_SRC_ALPHA,
		D3D11_BLEND_SRC_ALPHA,
		D3D11_BLEND_INV_SRC_ALPHA,
		D3D11_BLEND_DEST_ALPHA,
		D3D11_BLEND_INV_DEST_ALPHA,
		D3D11_BLEND_DEST_ALPHA,
		D3D11_BLEND_INV_DEST_ALPHA,
		D3D11_BLEND_BLEND_FACTOR,
		D3D11_BLEND_INV_BLEND_FACTOR
	};

	if (m_dirtyBlendState) {
		ID3D11BlendState *blendState;
		uint64_t key = ((uint64_t)m_renderTargetWriteMask) << 32 | m_blendState;
		if (auto it = m_blendStates.find(key); it != m_blendStates.end())
			blendState = it->second;
		else {
			D3D11_BLEND_DESC blendDesc;
			memset(&blendDesc, 0, sizeof(blendDesc));

			// set to d3d default
			blendDesc.AlphaToCoverageEnable = false;
			blendDesc.IndependentBlendEnable = false;
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

			//
			blendDesc.RenderTarget[0].RenderTargetWriteMask = m_renderTargetWriteMask;

			if ((m_blendState & 1) || m_renderTargetWriteMask != 15) {
				blendDesc.RenderTarget[0].BlendEnable = true;
				blendDesc.RenderTarget[0].BlendOp = blendOps[(m_blendState >> 1) & 7];
				blendDesc.RenderTarget[0].SrcBlend = colorBlends[(m_blendState >> 4) & 15];
				blendDesc.RenderTarget[0].DestBlend = colorBlends[(m_blendState >> 8) & 15];

				if (m_blendState & 0x1000) {
					blendDesc.RenderTarget[0].BlendOpAlpha = blendOps[(m_blendState >> 13) & 7];
					blendDesc.RenderTarget[0].SrcBlendAlpha = alphaBlends[(m_blendState >> 16) & 15];
					blendDesc.RenderTarget[0].DestBlendAlpha = alphaBlends[(m_blendState >> 20) & 15];

				} else {
					// copy from color settings
					blendDesc.RenderTarget[0].BlendOpAlpha = blendDesc.RenderTarget[0].BlendOp;
					blendDesc.RenderTarget[0].SrcBlendAlpha = alphaBlends[(m_blendState >> 4) & 15];
					blendDesc.RenderTarget[0].DestBlendAlpha = alphaBlends[(m_blendState >> 8) & 15];
				}

			} else {
				blendDesc.RenderTarget[0].BlendEnable = false;
			}

			HRESULT r = m_device->CreateBlendState(
				&blendDesc,
				&blendState);
			if (r != 0)
				cdc::FatalError("Couldn't create blend state %08x", m_blendState);
			m_blendStates[key] = blendState;
		}

		m_deviceContext->OMSetBlendState(
			blendState,
			m_blendFactors,
			/*sampleMask=*/ 0xffffffff);

		m_dirtyBlendState = false;
	}
}

void PCDX11StateManager::updateShaderResources() {
	unsigned begin = m_dirtyShaderResourcesFirst;
	unsigned end = m_dirtyShaderResourcesLast + 1;

	unsigned psBegin = std::clamp(begin, 0u, 16u);
	unsigned psEnd = std::clamp(end, 0u, 16u);

	if (psBegin < psEnd)
		m_deviceContext->PSSetShaderResources(
			psBegin, psEnd-psBegin, &m_textureViews[psBegin]);

	unsigned vsBegin = std::clamp(begin, 16u, 20u);
	unsigned vsEnd = std::clamp(end, 16u, 20u);

	if (vsBegin < vsEnd)
		m_deviceContext->VSSetShaderResources(
			vsBegin-16, vsEnd-vsBegin, &m_textureViews[vsBegin]);

	m_dirtyShaderResources = false;
	m_dirtyShaderResourcesFirst = 20;
	m_dirtyShaderResourcesLast = 0;
}

void PCDX11StateManager::updateSamplers() {
	unsigned begin = m_dirtySamplersFirst;
	unsigned end = m_dirtySamplersLast + 1;

	unsigned psBegin = std::clamp(begin, 0u, 16u);
	unsigned psEnd = std::clamp(end, 0u, 16u);

	if (psBegin < psEnd)
		m_deviceContext->PSSetSamplers(
			psBegin, psEnd-psBegin, &m_samplers[psBegin]);

	unsigned vsBegin = std::clamp(begin, 16u, 20u);
	unsigned vsEnd = std::clamp(end, 16u, 20u);

	if (vsBegin < vsEnd)
		m_deviceContext->VSSetSamplers(
			vsBegin-16, vsEnd-vsBegin, &m_samplers[vsBegin]);

	m_dirtySamplers = false;
	m_dirtySamplersFirst = 20;
	m_dirtySamplersLast = 0;
}

void PCDX11StateManager::updateConstantBuffers() {
	for (int i = 0; i < 7; i++)
		m_uberConstantBuffer[i]->syncBuffer(m_deviceContext);
	m_dirtyConstantBuffers = false;
}

void PCDX11StateManager::pushRenderTargets(
	PCDX11RenderTarget *renderTarget,
	PCDX11DepthBuffer *depthBuffer)
{
	uint32_t &i = m_renderTargetStackIndex;
	m_renderTargetStack[i] = m_renderTarget;
	m_depthBufferStack[i] = m_depthBuffer;
	i++;

	updateRenderTargets(renderTarget, depthBuffer);
}

void PCDX11StateManager::popRenderTargets() {
	PCDX11RenderTarget *renderTarget = nullptr;
	PCDX11DepthBuffer *depthBuffer = nullptr;
	if (uint32_t &i = m_renderTargetStackIndex; --i) {
		renderTarget = m_renderTargetStack[i];
		depthBuffer = m_depthBufferStack[i];
	}

	updateRenderTargets(renderTarget, depthBuffer);
}

void PCDX11StateManager::updateRenderTargets(
	PCDX11RenderTarget *renderTarget,
	PCDX11DepthBuffer *depthBuffer)
{
	// TODO
	m_renderTarget = renderTarget;
	m_depthBuffer = depthBuffer;
	m_dirtyViewport = true;

	ID3D11RenderTargetView *renderTargetView = renderTarget ? renderTarget->getRenderTargetView() : nullptr;
	ID3D11DepthStencilView *depthStencilView = depthBuffer ?
		depthBuffer->isLocked ? depthBuffer->getRODepthStencilView() : depthBuffer->getRWDepthStencilView() : nullptr;
	m_deviceContext->OMSetRenderTargets(
		!!renderTargetView,
		&renderTargetView,
		depthStencilView);
}

void PCDX11StateManager::updateRenderState() {
	if (m_dirtyRasterizerState)
		updateRasterizerState();
	if (m_dirtyDepthStencilState)
		updateDepthStencilState();
	if (m_dirtyBlendState)
		updateBlendState();
	if (m_dirtyShaderResources)
		updateShaderResources();
	if (m_dirtySamplers)
		updateSamplers();
	if (m_dirtyConstantBuffers)
		updateConstantBuffers();
}

void PCDX11StateManager::updateViewport() {
	// TODO: proper implementation
	if (!m_dirtyViewport)
		return;

	IRenderSurface *r = m_renderTarget ? (IRenderSurface*) m_renderTarget : (IRenderSurface*) m_depthBuffer;
	float width = r ? r->getWidth() : 320;
	float height = r ? r->getHeight() : 240;
	float minDepth = 0.f;
	float maxDepth = 1.f;

	auto& sceneBuffer = accessCommonCB(1);
	float row[4];

	row[0] = 0.0f; // offset
	row[1] = 0.0f; // offset
	row[2] = 1.0f/width; // scale
	row[3] = 1.0f/height; // scale
	sceneBuffer.assignRow(44, row, 1); // SceneBuffer::ScreenExtents

	row[0] = width;
	row[1] = height;
	row[2] = 0.0f;
	row[3] = 0.0f;
	sceneBuffer.assignRow(45, row, 1); // SceneBuffer::ScreenResolution

	if (m_isSkySphereModeEnabled) {
		float splitPoint = maxDepth - (maxDepth-minDepth) * kSkySphereRange; // * 0.02f
		if (m_isSkySphereMode)
			minDepth = splitPoint;
		else
			maxDepth = splitPoint;
	}

	row[0] = maxDepth-minDepth;
	row[1] = minDepth;
	row[2] = 0.0f;
	row[3] = 0.0f;
	sceneBuffer.assignRow(22, row, 1); // SceneBuffer::ViewportDepthScaleOffset

	D3D11_VIEWPORT viewport {
		/*.TopLeftX=*/ 0,
		/*.TopLeftY=*/ 0,
		/*.Width=*/    width,
		/*.Height=*/   height,
		/*.MinDepth=*/ minDepth,
		/*.MaxDepth=*/ maxDepth
	};

	m_deviceContext->RSSetViewports(1, &viewport);
	m_dirtyViewport = false;
}

void PCDX11StateManager::getCompressedDepthRange(float& near, float& far) {
	near = 0.0f;
	far = 1.f - kSkySphereRange;
}

void PCDX11StateManager::setComputeShader(PCDX11ComputeShader *computeShader) {
	if (!computeShader->m_d3dShader)
		computeShader->createD3DShader();
	m_deviceContext->CSSetShader(computeShader->m_d3dShader, nullptr, 0);
}

void PCDX11StateManager::setComputeShaderTexture(uint32_t slot, PCDX11BaseTexture *tex) {
	ID3D11ShaderResourceView *srv = tex ? tex->createShaderResourceView() : nullptr;
	m_deviceContext->CSSetShaderResources(slot, 1, &srv);
}

void PCDX11StateManager::setComputeShaderUAV(uint32_t slot, PCDX11BaseTexture *tex) {
	ID3D11UnorderedAccessView *uav = tex ? tex->createUnorderedAccessView() : nullptr;
	unsigned negOne = -1;
	m_deviceContext->CSSetUnorderedAccessViews(slot, 1, &uav, &negOne);
}

void PCDX11StateManager::setComputeShaderCB(uint32_t slot, PCDX11ConstantBuffer *cb) {
	ID3D11Buffer *buffer = cb ? cb->getBuffer() : nullptr;
	m_deviceContext->CSSetConstantBuffers(slot, 1, &buffer);
}

void PCDX11StateManager::dispatch(uint32_t x, uint32_t y, uint32_t z) {
	m_deviceContext->Dispatch(x, y, z);
}

bool PCDX11StateManager::internalCreate() {

	m_device = deviceManager->getD3DDevice();
	m_indexBufferD3D = nullptr;
	m_vertexBufferD3D = nullptr;
	m_streamDecl = nullptr;
	m_pixelShader = nullptr;
	m_vertexShader = nullptr;
	// TODO: hull shader
	// TODO: domain shader

	m_rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	m_rasterizerDesc.CullMode = D3D11_CULL_BACK;
	m_rasterizerDesc.FrontCounterClockwise = false;
	m_rasterizerDesc.DepthBias = 0;
	m_rasterizerDesc.DepthBiasClamp = 1.0;
	m_rasterizerDesc.SlopeScaledDepthBias = 0.0;
	m_rasterizerDesc.DepthClipEnable = true;
	m_rasterizerDesc.ScissorEnable = false;
	m_rasterizerDesc.MultisampleEnable = false;
	m_rasterizerDesc.AntialiasedLineEnable = false;

	memset((void*)&m_depthStencilDesc, 0, sizeof(m_depthStencilDesc));
	m_depthStencilDesc.DepthEnable = true;
	m_depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	m_depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	m_depthStencilDesc.StencilEnable = false;
	m_depthStencilDesc.StencilReadMask = 255;
	m_depthStencilDesc.StencilWriteMask = 255;
	m_renderTargetWriteMask = 0;

	// TODO: stencilParams

	reset();
	// TODO: clipPlanes

	m_renderTargetStackIndex = 0;

	m_dirtyUberCBs[0] = false;
	m_dirtyUberCBs[1] = false;
	m_dirtyUberCBs[2] = false;
	m_dirtyUberCBs[3] = false;
	m_dirtyUberCBs[4] = false;
	m_dirtyUberCBs[5] = false;
	m_dirtyUberCBs[6] = false;	

	return true;
}

void PCDX11StateManager::internalRelease() {}

}
