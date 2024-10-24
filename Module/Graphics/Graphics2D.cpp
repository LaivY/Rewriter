#include "Stdafx.h"
#include "Global.h"
#include "Graphics2D.h"

namespace Graphics::D2D
{
	struct FontCompare
	{
		bool operator()(const Font& lhs, const Font& rhs) const
		{
			if (lhs.name != rhs.name)
				return lhs.name < rhs.name;
			return lhs.size < rhs.size;
		}
	};

	struct ColorCompare
	{
		bool operator()(const Color& lhs, const Color& rhs) const
		{
			if (lhs.r != rhs.r)
				return lhs.r < rhs.r;
			if (lhs.g != rhs.g)
				return lhs.g < rhs.g;
			if (lhs.b != rhs.b)
				return lhs.b < rhs.b;
			return lhs.a < rhs.a;
		}
	};

	std::map<Font, ComPtr<IDWriteTextFormat>, FontCompare> g_textFormats;
	std::map<Color, ComPtr<ID2D1SolidColorBrush>, ColorCompare> g_colorBrushes;

	Layer::Layer(ComPtr<ID2D1BitmapRenderTarget> target) :
		m_target{ target }
	{
	}

	DLL_API void Layer::Begin()
	{
		if (!m_target)
			return;

		m_target->BeginDraw();
		g_d2dCurrentRenderTarget = m_target.Get();
	}

	DLL_API bool Layer::End()
	{
		if (!m_target)
			return false;

		if (FAILED(m_target->EndDraw()))
			return false;

		g_d2dCurrentRenderTarget = d2dContext.Get();
		return true;
	}

	DLL_API void Layer::Draw(const FLOAT2& position)
	{
		if (!m_target)
			return;

		ID2D1Bitmap* target{};
		if (FAILED(m_target->GetBitmap(&target)))
			return;

		auto size{ target->GetSize() };
		d2dContext->DrawBitmap(target, RECTF{ position.x, position.y, position.x + size.width, position.y + size.height });
	}

	DLL_API void Layer::Clear()
	{
		m_target->Clear();
	}

	ComPtr<ID2D1BitmapRenderTarget> Layer::GetTarget() const
	{
		return m_target;
	}

	DLL_API void Begin()
	{
		d3d11On12Device->AcquireWrappedResources(wrappedBackBuffers[frameIndex].GetAddressOf(), 1);
		d2dContext->SetTarget(d2dRenderTargets[frameIndex].Get());
		d2dContext->BeginDraw();
		g_d2dCurrentRenderTarget = d2dContext.Get();
	}

	DLL_API bool End()
	{
		if (FAILED(d2dContext->EndDraw()))
			return false;
		d3d11On12Device->ReleaseWrappedResources(wrappedBackBuffers[frameIndex].GetAddressOf(), 1);
		d3d11DeviceContext->Flush();
		return true;
	}

	DLL_API ComPtr<ID2D1DeviceContext2> GetContext()
	{
		return d2dContext;
	}

	DLL_API std::shared_ptr<Layer> CreateLayer(const FLOAT2& size)
	{
		ComPtr<ID2D1BitmapRenderTarget> target;
		if (FAILED(d2dContext->CreateCompatibleRenderTarget(D2D1_SIZE_F{ size.x, size.y }, &target)))
			return nullptr;
		return std::make_shared<Layer>(target);
	}

	DLL_API void SetTransform(const Matrix& transform)
	{
		g_d2dCurrentRenderTarget->SetTransform(transform);
	}

	DLL_API void PushClipRect(const RECTF& rect)
	{
		g_d2dCurrentRenderTarget->PushAxisAlignedClip(rect, D2D1_ANTIALIAS_MODE_ALIASED);
	}

	DLL_API void PopClipRect()
	{
		g_d2dCurrentRenderTarget->PopAxisAlignedClip();
	}

	DLL_API void DrawRect(const RECTF& rect, const Color& color)
	{
		ComPtr<ID2D1SolidColorBrush> brush;
		g_d2dCurrentRenderTarget->CreateSolidColorBrush(color, &brush);
		g_d2dCurrentRenderTarget->FillRectangle(rect, brush.Get());
	}

	DLL_API void DrawText(std::wstring_view text, const Font& font, const Color& color, const FLOAT2& position, Pivot pivot)
	{
		ComPtr<IDWriteTextFormat> textFormat;
		if (g_textFormats.contains(font))
		{
			textFormat = g_textFormats[font];
		}
		else
		{
			dwriteFactory->CreateTextFormat(font.name.data(), nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, font.size, L"", &textFormat);
			textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
			textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
			g_textFormats.emplace(font, textFormat);
		}

		ComPtr<ID2D1SolidColorBrush> colorBrush;
		if (g_colorBrushes.contains(color))
		{
			colorBrush = g_colorBrushes[color];
		}
		else
		{
			g_d2dCurrentRenderTarget->CreateSolidColorBrush(color, &colorBrush);
			g_colorBrushes.emplace(color, colorBrush);
		}

		ComPtr<IDWriteTextLayout> textLayout;
		dwriteFactory->CreateTextLayout(text.data(), static_cast<UINT32>(text.size()), textFormat.Get(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), &textLayout);

		DWRITE_TEXT_METRICS metrics{};
		textLayout->GetMetrics(&metrics);
		FLOAT2 offset{};
		switch (pivot)
		{
		case Pivot::LeftTop:
			break;
		case Pivot::CenterTop:
			offset.x -= metrics.width / 2.0f;
			break;
		case Pivot::RightTop:
			offset.x -= metrics.width;
			break;
		case Pivot::LeftCenter:
			offset.y -= metrics.height / 2.0f;
			break;
		case Pivot::Center:
			offset.x -= metrics.width / 2.0f;
			offset.y -= metrics.height / 2.0f;
			break;
		case Pivot::RightCenter:
			offset.x -= metrics.width;
			offset.y -= metrics.height / 2.0f;
			break;
		case Pivot::LeftBot:
			offset.y -= metrics.height;
			break;
		case Pivot::CenterBot:
			offset.x -= metrics.width / 2.0f;
			offset.y -= metrics.height;
			break;
		case Pivot::RightBot:
			offset.x -= metrics.width;
			offset.y -= metrics.height;
			break;
		default:
			break;
		}
		g_d2dCurrentRenderTarget->DrawTextLayout(position + offset, textLayout.Get(), colorBrush.Get());
	}

	DLL_API void DrawSprite(const std::shared_ptr<Resource::Sprite>& sprite, const FLOAT2& position, float opacity)
	{
		auto size{ sprite->GetSize() };
		DrawSprite(sprite, RECTF{ 0.0f, 0.0f, size.x, size.y }.Offset(position), opacity);
	}

	DLL_API void DrawSprite(const std::shared_ptr<Resource::Sprite>& sprite, const RECTF& rect, float opacity)
	{
		g_d2dCurrentRenderTarget->DrawBitmap(sprite->Get(), rect, opacity);
	}
}
