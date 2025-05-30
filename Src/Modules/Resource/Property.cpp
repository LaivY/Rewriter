#include "Stdafx.h"
#include "Model.h"
#include "Property.h"
#include "Sprite.h"
#include "External/DirectX/WICTextureLoader12.h"

namespace Resource
{
	Property::Iterator::Iterator(std::vector<std::shared_ptr<Property>>::iterator iterator) :
		m_iterator{ iterator }
	{
	}

	DLL_API Property::Iterator::reference Property::Iterator::operator*() const
	{
		return { (*m_iterator)->GetName(), *m_iterator };
	}

	DLL_API Property::Iterator& Property::Iterator::operator++()
	{
		++m_iterator;
		return *this;
	}

	DLL_API Property::Iterator Property::Iterator::operator++(int)
	{
		Iterator temp{ *this };
		++*this;
		return temp;
	}

	DLL_API Property::Iterator& Property::Iterator::operator+=(const difference_type offset)
	{
		m_iterator += offset;
		return *this;
	}

	DLL_API Property::Iterator Property::Iterator::operator+(const difference_type offset)
	{
		Iterator temp{ *this };
		temp += offset;
		return temp;
	}

	DLL_API Property::Iterator& Property::Iterator::operator--()
	{
		--m_iterator;
		return *this;
	}

	DLL_API Property::Iterator Property::Iterator::operator--(int)
	{
		Iterator temp{ *this };
		--*this;
		return temp;
	}

	DLL_API Property::Iterator& Property::Iterator::operator-=(const difference_type offset)
	{
		m_iterator -= offset;
		return *this;
	}

	DLL_API Property::Iterator Property::Iterator::operator-(const difference_type offset)
	{
		Iterator temp{ *this };
		temp -= offset;
		return temp;
	}

	DLL_API Property::Iterator::reference Property::Iterator::operator[](const difference_type offset)
	{
		return *(*this + offset);
	}

	DLL_API bool Property::Iterator::operator==(const Iterator& rhs) const
	{
		return m_iterator == rhs.m_iterator;
	}

	DLL_API bool Property::Iterator::operator!=(const Iterator& rhs) const
	{
		return m_iterator != rhs.m_iterator;
	}

	DLL_API Property::Property() :
		m_type{ Property::Type::Folder },
		m_name{ L"" }
	{
	}

	DLL_API Property::Iterator Property::begin()
	{
		return Iterator{ m_children.begin() };
	}

	DLL_API Property::Iterator Property::end()
	{
		return Iterator{ m_children.end() };
	}

	DLL_API void Property::Add(const std::shared_ptr<Property>& child)
	{
		m_children.push_back(child);
	}

	DLL_API void Property::Delete(const std::shared_ptr<Property>& child)
	{
		std::erase(m_children, child);
	}

	DLL_API void Property::SetType(Type type)
	{
		m_type = type;
		switch (m_type)
		{
		case Type::Folder:
			break;
		case Type::Int:
			m_data = 0;
			break;
		case Type::Int2:
			m_data = Int2{ 0, 0 };
			break;
		case Type::Float:
			m_data = 0.0f;
			break;
		case Type::String:
			m_data = L"";
			break;
		case Type::Sprite:
			m_data.emplace<std::shared_ptr<Sprite>>(nullptr);
			break;
		case Type::Texture:
			// TODO
			break;
		case Type::Model:
			break;
		default:
			assert(false && "INVALID TYPE");
			break;
		}
	}

	DLL_API void Property::SetName(std::wstring_view name)
	{
		m_name = name;
	}

	DLL_API void Property::Set(int32_t value)
	{
		m_data = value;
	}

	DLL_API void Property::Set(const Int2& value)
	{
		m_data = value;
	}

	DLL_API void Property::Set(float value)
	{
		m_data = value;
	}

	DLL_API void Property::Set(std::wstring_view value)
	{
		m_data.emplace<std::wstring>(value);
	}

	DLL_API void Property::Set(const std::shared_ptr<Sprite>& value)
	{
		m_data = value;
	}

	DLL_API void Property::Set(const std::shared_ptr<Texture>& value)
	{
		m_data = value;
	}

	DLL_API void Property::Set(const std::shared_ptr<Model>& value)
	{
		m_data = value;
	}

	DLL_API Property::Type Property::GetType() const
	{
		return m_type;
	}

	DLL_API std::wstring Property::GetName() const
	{
		return m_name;
	}

	DLL_API int32_t Property::GetInt(std::wstring_view path) const
	{
		if (path.empty())
		{
			assert(m_type == Type::Int);
			return std::get<int32_t>(m_data);
		}

		std::wstring_view name{ path };
		std::wstring_view remain{};

		size_t pos{ path.find(Stringtable::DATA_PATH_SEPERATOR) };
		if (pos != std::wstring_view::npos)
		{
			name = path.substr(0, pos);
			remain = path.substr(pos + 1);
		}

		if (const auto& child{ Get(name) })
			return child->GetInt(remain);

		return 0;
	}

	DLL_API Int2 Property::GetInt2(std::wstring_view path) const
	{
		if (path.empty())
		{
			assert(m_type == Type::Int2);
			return std::get<Int2>(m_data);
		}

		std::wstring_view name{ path };
		std::wstring_view remain{};

		size_t pos{ path.find(Stringtable::DATA_PATH_SEPERATOR) };
		if (pos != std::wstring_view::npos)
		{
			name = path.substr(0, pos);
			remain = path.substr(pos + 1);
		}

		if (const auto& child{ Get(name) })
			return child->GetInt2(remain);

		return Int2{};
	}

	DLL_API float Property::GetFloat(std::wstring_view path) const
	{
		if (path.empty())
		{
			assert(m_type == Type::Float);
			return std::get<float>(m_data);
		}

		std::wstring_view name{ path };
		std::wstring_view remain{};

		size_t pos{ path.find(Stringtable::DATA_PATH_SEPERATOR) };
		if (pos != std::wstring_view::npos)
		{
			name = path.substr(0, pos);
			remain = path.substr(pos + 1);
		}

		if (const auto& child{ Get(name) })
			return child->GetFloat(remain);

		return 0.0f;
	}

	DLL_API std::wstring Property::GetString(std::wstring_view path) const
	{
		if (path.empty())
		{
			assert(m_type == Type::String);
			return std::get<std::wstring>(m_data);
		}

		std::wstring_view name{ path };
		std::wstring_view remain{};

		size_t pos{ path.find(Stringtable::DATA_PATH_SEPERATOR) };
		if (pos != std::wstring_view::npos)
		{
			name = path.substr(0, pos);
			remain = path.substr(pos + 1);
		}

		if (const auto& child{ Get(name) })
			return child->GetString(remain);

		return L"";
	}

	DLL_API std::shared_ptr<Sprite> Property::GetSprite(std::wstring_view path) const
	{
		if (path.empty())
		{
			assert(m_type == Type::Sprite);
			return std::get<std::shared_ptr<Sprite>>(m_data);
		}

		std::wstring_view name{ path };
		std::wstring_view remain{};

		size_t pos{ path.find(Stringtable::DATA_PATH_SEPERATOR) };
		if (pos != std::wstring_view::npos)
		{
			name = path.substr(0, pos);
			remain = path.substr(pos + 1);
		}

		if (const auto& child{ Get(name) })
			return child->GetSprite(remain);

		assert(m_type == Type::Sprite);
		return nullptr;
	}

	DLL_API std::shared_ptr<Texture> Property::GetTexture(std::wstring_view path) const
	{
		if (path.empty())
		{
			assert(m_type == Type::Texture);
			return std::get<std::shared_ptr<Texture>>(m_data);
		}

		std::wstring_view name{ path };
		std::wstring_view remain{};

		size_t pos{ path.find(Stringtable::DATA_PATH_SEPERATOR) };
		if (pos != std::wstring_view::npos)
		{
			name = path.substr(0, pos);
			remain = path.substr(pos + 1);
		}

		if (const auto& child{ Get(name) })
			return child->GetTexture(remain);

		assert(m_type == Type::Texture);
		return nullptr;
	}

	DLL_API std::shared_ptr<Model> Property::GetModel(std::wstring_view path) const
	{
		if (path.empty())
		{
			assert(m_type == Type::Model);
			return std::get<std::shared_ptr<Model>>(m_data);
		}

		std::wstring_view name{ path };
		std::wstring_view remain{};

		size_t pos{ path.find(Stringtable::DATA_PATH_SEPERATOR) };
		if (pos != std::wstring_view::npos)
		{
			name = path.substr(0, pos);
			remain = path.substr(pos + 1);
		}

		if (const auto& child{ Get(name) })
			return child->GetModel(remain);

		assert(m_type == Type::Model);
		return nullptr;
	}

	DLL_API std::shared_ptr<Property> Property::Get(std::wstring_view path) const
	{
		size_t pos{ path.find(Stringtable::DATA_PATH_SEPERATOR) };
		if (pos != std::wstring_view::npos)
		{
			auto childName{ path.substr(0, pos) };
			auto remain{ path.substr(pos + 1) };
			auto it{ std::ranges::find_if(m_children, [childName](const auto& p) { return p->m_name == childName; }) };
			if (it == m_children.end())
				return nullptr;
			return (*it)->Get(remain);
		}

		auto it{ std::ranges::find_if(m_children, [path](const auto& p) { return p->m_name == path; }) };
		if (it == m_children.end())
			return nullptr;
		return *it;
	}

	DLL_API const std::vector<std::shared_ptr<Property>>& Property::GetChildren() const
	{
		return m_children;
	}

	DLL_API std::vector<std::shared_ptr<Property>>& Property::GetChildren()
	{
		return m_children;
	}

#ifdef _TOOL
	DLL_API void Property::SetParent(const std::shared_ptr<Property>& prop)
	{
		m_parent = prop;
	}

	DLL_API std::shared_ptr<Property> Property::GetParent() const
	{
		return m_parent.lock();
	}
#endif
}
