﻿#pragma once

class NytImage
{
public:
	NytImage(const ComPtr<ID2D1Bitmap>& bitmap);
	~NytImage() = default;

	void Render(const ComPtr<ID2D1HwndRenderTarget>& renderTarget) const;

private:
	ComPtr<ID2D1Bitmap> m_bitmap;
};