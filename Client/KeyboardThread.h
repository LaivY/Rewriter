﻿#pragma once

template<class T>
class TSingleton;

class KeyboardThread : public TSingleton<KeyboardThread>
{
public:
	KeyboardThread();
	~KeyboardThread();

	void OnKeyboardEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	void Run();

private:
	std::thread m_thread;
	BOOL m_isRunning;
	BOOL m_doProcess;

	HWND m_hWnd;
	UINT m_message;
	WPARAM m_wParam;
	LPARAM m_lParam;
};