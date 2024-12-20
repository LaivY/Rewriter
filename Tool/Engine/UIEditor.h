#pragma once

class UIEditor : public TSingleton<UIEditor>
{
private:
	struct Window
	{
		std::string fullPath;

		Int2 size;
		int32_t backgroundColor;
		Int2 backgroundRectRadius;
	};

public:
	UIEditor();
	~UIEditor() = default;

	void Render();
	void Render2D();

private:
	void DragDrop();
	void BuildWindow();
	void CalcClipRect();

private:
	static constexpr auto WINDOW_NAME{ "UI Editor" };
	bool m_isVisible;
	RectF m_clipRect;

	std::shared_ptr<Resource::Property> m_prop;
	Window m_window;
};
