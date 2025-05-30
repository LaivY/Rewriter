#include "Stdafx.h"
#include "Global.h"
#include "Graphics.h"
#include "Graphics2D.h"
#include "Graphics3D.h"
#include "SwapChain.h"

namespace Graphics
{
	DLL_API bool Initialize(HWND hWnd)
	{
		g_hWnd = hWnd;
		if (!D3D::Initialize())
			return false;
#ifdef _DIRECT2D
		if (!D2D::Initialize())
			return false;
#endif
		return true;
	}

	DLL_API void CleanUp()
	{
		D3D::CleanUp();
#ifdef _DIRECT2D
		D2D::CleanUp();
#endif
	}

	DLL_API void OnResize(int width, int height)
	{
		g_swapChain->OnResize(width, height);
	}
}
