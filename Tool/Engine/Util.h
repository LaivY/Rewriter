﻿#pragma once

namespace Util
{
	std::string wstou8s(const std::wstring& wstr);
	std::string wstombs(const std::wstring& wstr);
	std::string u8stou8s(const std::u8string& u8str);
	std::string u8stombs(const std::u8string& u8str);
	std::wstring u8stows(const std::string& u8str);
}