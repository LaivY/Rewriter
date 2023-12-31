﻿#include "Stdafx.h"
#include "Include/Image.h"

namespace Resource
{
	Image::Image(BYTE* binary, DWORD binarySize) : m_binarySize{ binarySize }
	{
		m_binary.reset(binary);
	}

	Image::operator ID2D1Bitmap*() const
	{
		return m_d2dBitmap.Get();
	}

	void Image::SetD2DBitmap(ID2D1Bitmap* bitmap)
	{
		m_binary.reset();
		m_binarySize = 0;
		m_d2dBitmap = bitmap;
	}

	std::pair<BYTE*, DWORD> Image::GetBinary() const
	{
		return std::make_pair(m_binary.get(), m_binarySize);
	}

	ID2D1Bitmap* Image::GetD2DBitmap() const
	{
		return m_d2dBitmap.Get();
	}

	INT2 Image::GetSize() const
	{
		if (m_d2dBitmap)
		{
			auto [w, h] { m_d2dBitmap->GetSize() };
			return INT2{ static_cast<int>(w), static_cast<int>(h) };
		}

		if (m_d3dResource)
		{
			auto desc{ m_d3dResource->GetDesc() };
			return INT2{ static_cast<int>(desc.Width), static_cast<int>(desc.Height) };
		}

		return INT2{};
	}

	INT2 GetSize(const std::shared_ptr<Resource::Image>& image)
	{
		if (image)
			return image->GetSize();
		return INT2{ 0, 0 };
	}

	void UseAsD2D(const ComPtr<ID2D1DeviceContext2>& ctx, const std::shared_ptr<Resource::Image>& image)
	{
		if (!ctx || !image)
			return;

		auto bitmap{ image->GetD2DBitmap() };
		if (bitmap)
			return;

		auto [binary, binarySize] { image->GetBinary() };

		ComPtr<IWICImagingFactory> factory;
		ComPtr<IWICBitmapDecoder> decoder;
		ComPtr<IWICFormatConverter> converter;
		ComPtr<IWICBitmapFrameDecode> frameDecode;
		ComPtr<IWICStream> stream;

		HRESULT hr{ E_FAIL };
		hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
		hr = factory->CreateStream(&stream);
		hr = stream->InitializeFromMemory(binary, binarySize);
		hr = factory->CreateDecoderFromStream(stream.Get(), NULL, WICDecodeMetadataCacheOnLoad, &decoder);
		hr = factory->CreateFormatConverter(&converter);
		hr = decoder->GetFrame(0, &frameDecode);
		hr = converter->Initialize(frameDecode.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeMedianCut);
		hr = ctx->CreateBitmapFromWicBitmap(converter.Get(), &bitmap);
		assert(SUCCEEDED(hr));

		image->SetD2DBitmap(bitmap);
	}
}