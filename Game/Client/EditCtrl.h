﻿#pragma once
#include "UI.h"
#include "Font.h"

class EditCtrl : public IUserInterface
{
private:
	static constexpr float MARGIN = 1.0f;
	static constexpr int CARET_THICKNESS = 2;
	static constexpr float CARET_BLINK_SECOND = 0.5f;

public:
	EditCtrl(const INT2& size);
	~EditCtrl() = default;

	virtual void OnKeyboardEvent(UINT message, WPARAM wParam, LPARAM lParam) final;

	virtual void Update(FLOAT deltaTime) final;
	virtual void Render(const ComPtr<ID2D1DeviceContext2>& d2dContext) const final;

	void SetFont(const std::shared_ptr<Font>& font);
	void SetText(const std::string& text);

private:
	void EraseText(size_t count);
	void InsertText(const std::wstring& text);
	void MoveCaret(int distance);
	void CreateTextLayout();

private:
	std::shared_ptr<Font> m_font;
	ComPtr<IDWriteTextLayout> m_textLayout;

	std::wstring m_text;
	bool m_isCompositing;

	// 캐럿
	int m_caretPosition;
	RECTF m_caretRect;
	float m_caretTimer;
	float m_xOffset;
};