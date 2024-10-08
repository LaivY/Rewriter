#include "Stdafx.h"
#include "App.h"
#include "Button.h"
#include "Control.h"
#include "DebugWindow.h"
#include "Modal.h"
#include "SocketManager.h"
#include "TextBlock.h"
#include "TextBox.h"
#include "WindowManager.h"

class RegisterAccountModal :
	public IModal,
	public IObserver
{
private:
	enum class ButtonID
	{
		OK, CANCLE
	};

public:
	RegisterAccountModal(const Callback& callback) :
		IModal{ callback }
	{
		m_size = { 500, 300 };
		SetPosition({ App::size.x / 2, App::size.y / 2 }, Pivot::CENTER);

		auto okButton{ std::make_shared<Button>(this) };
		okButton->OnButtonClick.Register(this, std::bind(&RegisterAccountModal::OnButtonClicked, this, ButtonID::OK));
		okButton->SetSize({ 80, 20 });
		okButton->SetPosition({ m_size.x / 2 - 50, m_size.y / 2 + 120 }, Pivot::CENTER);
		okButton->SetText(L"OK");
		Register(okButton);

		auto cancleButton{ std::make_shared<Button>(this) };
		cancleButton->OnButtonClick.Register(this, std::bind(&RegisterAccountModal::OnButtonClicked, this, ButtonID::CANCLE));
		cancleButton->SetSize({ 80, 20 });
		cancleButton->SetPosition({ m_size.x / 2 + 50, m_size.y / 2 + 120 }, Pivot::CENTER);
		cancleButton->SetText(L"CANCLE");
		Register(cancleButton);
	}

	virtual void Render() const override final
	{
		RECTI rect{ 0, 0, m_size.x, m_size.y };
		Graphics::D2D::DrawRect(rect, Graphics::D2D::Color::White);
		IWindow::Render();
	}

private:
	void OnButtonClicked(ButtonID id)
	{
		if (id == ButtonID::CANCLE)
			Destroy();
	}
};

DebugWindow::DebugWindow()
{
	App::OnPacket.Register(this, std::bind_front(&DebugWindow::OnPacket, this));
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
	//auto b = Resource::Get(L"UI.dat/LoginUI/Background");	
	//Renderer2D::DrawImage(b->GetImage(), FLOAT2{});

	RECTI rect{ 0, 0, m_size.x, m_size.y };
	//RECTI outline{ rect };
	//outline.left -= 5;
	//outline.top -= 5;
	//outline.right += 5;
	//outline.bottom += 5;
	//Graphics::D2D::DrawRect(outline, D2D1::ColorF::Black);
	Graphics::D2D::DrawRect(rect, D2D1::ColorF::White);

	using namespace Graphics::D2D;
	Font font{ L"", 16.0f };
	DrawText(L"안녕하세요", { 50, 50 }, font, Color::Black);

	IWindow::Render();
}

void DebugWindow::OnPacket(Packet& packet)
{
}
