﻿#pragma once
#include "Scene.h"

class SceneManager : public TSingleton<SceneManager>
{
public:
	SceneManager();
	~SceneManager() = default;

	void OnDestroy();

	void Update(FLOAT deltaTime);
	void Render(const ComPtr<ID2D1HwndRenderTarget>& renderTarget) const;

	template<class T>
	requires std::is_base_of<Scene, T>::value
	void SetScene(T* scene)
	{
		if (m_scene)
			m_scene->OnDestory();
		m_scene = scene;
	}

	template<class T>
	requires std::is_base_of<Scene, T>::value
	void ChangeScene(T* scene)
	{
		m_nextScene = scene;
	}

private:
	Scene* m_scene;
	Scene* m_nextScene;
};