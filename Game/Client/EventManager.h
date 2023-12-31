﻿#pragma once

class IScene;
class IGameObject;

class EventManager : public TSingleton<EventManager>
{
private:
	template <class... Args>
	class Event
	{
	private:
		using Caller = std::weak_ptr<IGameObject>;
		using Callback = std::function<bool(Args...)>;

		struct Data
		{
			bool doValidCheck{ true };
			Caller caller{};
			Callback callback{};
		};

	public:
		Event() = default;
		~Event() = default;

		void operator()(const Args&... args)
		{
			std::erase_if(m_data, 
				[&](const auto& data)
				{
					if (data.doValidCheck)
					{
						if (auto caller{ data.caller.lock() })
							return data.callback(args...);
						return true;
					}
					return data.callback(args...);
				});
		}

		void Add(const Callback& callback)
		{
			m_data.emplace_back(false, Caller{}, callback);
		}

		void Add(const Caller& caller, const Callback& callback)
		{
			m_data.emplace_back(true, caller, callback);
		}

		void Clear()
		{
			m_data.clear();
		}

	private:
		std::vector<Data> m_data;
	};

public:
	EventManager() = default;
	~EventManager() = default;

	void Clear();

public:
	Event<IScene*> OnSceneChange;
};