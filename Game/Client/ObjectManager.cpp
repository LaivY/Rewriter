﻿#include "Stdafx.h"
#include "Camera.h"
#include "EventManager.h"
#include "GameObject.h"
#include "Map.h"
#include "ObjectManager.h"
#include "Player.h"

ObjectManager::ObjectManager()
{
	if (auto em{ EventManager::GetInstance() })
		em->OnSceneChange.Add(std::bind_front(&ObjectManager::OnSceneChange, this));
}

void ObjectManager::Update(float deltaTime)
{
	if (m_camera)
		m_camera->Update(deltaTime);
	if (m_map)
		m_map->Update(deltaTime);
	if (m_localPlayer)
		m_localPlayer->Update(deltaTime);
	for (const auto& [_, p] : m_remotePlayers)
		p->Update(deltaTime);
	for (const auto& o : m_gameObjects)
		o->Update(deltaTime);
	RemoveInvalidObjects();
}

void ObjectManager::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	if (m_camera)
		m_camera->SetShaderVariable(commandList);
	if (m_map)
		m_map->Render(commandList);
	if (m_localPlayer)
		m_localPlayer->Render(commandList);
	for (const auto& [_, p] : m_remotePlayers)
		p->Render(commandList);
	for (const auto& o : m_gameObjects)
		o->Render(commandList);
}

void ObjectManager::AddObject(const std::shared_ptr<IGameObject>& object)
{
	m_gameObjects.push_back(object);
}

void ObjectManager::SetCamera(const std::shared_ptr<Camera>& camera)
{
	m_camera = camera;
}

void ObjectManager::SetMap(const std::shared_ptr<Map>& map)
{
	m_map = map;
}

void ObjectManager::SetLocalPlayer(const std::shared_ptr<LocalPlayer>& player)
{
	m_localPlayer = player;
}

void ObjectManager::AddRemotePlayer(const std::shared_ptr<RemotePlayer>& player)
{
	m_remotePlayers.insert(std::make_pair(player->GetCharacterID(), player));
}

std::weak_ptr<IGameObject> ObjectManager::GetGameObject(IGameObject* object) const
{
	auto it{ std::ranges::find_if(m_gameObjects, [&](const auto& o) { return o.get() == object; }) };
	if (it != m_gameObjects.end())
		return *it;
	return {};
}

std::weak_ptr<Camera> ObjectManager::GetCamera() const
{
	return m_camera;
}

std::weak_ptr<Map> ObjectManager::GetMap() const
{
	return m_map;
}

std::weak_ptr<LocalPlayer> ObjectManager::GetLocalPlayer() const
{
	return m_localPlayer;
}

std::weak_ptr<RemotePlayer> ObjectManager::GetRemotePlayer(CharacterID characterID) const
{
	if (m_remotePlayers.contains(characterID))
		return m_remotePlayers.at(characterID);
	return {};
}

bool ObjectManager::OnSceneChange(IScene* scene)
{
	auto om{ ObjectManager::GetInstance() };
	if (!om)
		return true;

	// 관리하는 모든 게임오브젝트들 삭제
	om->m_gameObjects.clear();
	om->m_camera.reset();
	om->m_localPlayer.reset();
	om->m_remotePlayers.clear();
	return false;
}

void ObjectManager::RemoveInvalidObjects()
{
	std::erase_if(m_gameObjects, [](const auto& o) { return !o->IsValid(); });
}
