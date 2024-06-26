﻿#include "Stdafx.h"
#include "Control.h"
#include "DebugWindow.h"
#include "Renderer2D.h"
#include "TextBox.h"

DebugWindow::DebugWindow()
{
	m_size = { 800, 600 };
	
	auto textBox{ std::make_unique<TextBox>(this) };
	textBox->SetSize({ 150, 25 });
	textBox->SetPosition({ m_size.x / 2, m_size.y / 2 }, Pivot::CENTER);
	m_controls.push_back(std::move(textBox));
}

void DebugWindow::OnMouseEvent(UINT message, int x, int y)
{
	IWindow::OnMouseEvent(message, x, y);
}

void DebugWindow::OnKeyboardEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
	IWindow::OnKeyboardEvent(message, wParam, lParam);
}

void DebugWindow::Update(float deltaTime)
{
	IWindow::Update(deltaTime);
}

void DebugWindow::Render() const
{
	RECTI rect{ 0, 0, m_size.x, m_size.y };
	RECTI outline{ rect };
	outline.left -= 5;
	outline.top -= 5;
	outline.right += 5;
	outline.bottom += 5;
	Renderer2D::DrawRect(outline, D2D1::ColorF::Black);
	Renderer2D::DrawRect(rect, D2D1::ColorF::White);

	for (const auto& control : m_controls)
		control->Render();
}
