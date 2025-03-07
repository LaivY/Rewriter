#pragma once

namespace Protocol
{
	enum class Type : uint8_t
	{
		None,
		Initialize,
		Login,
		Register,
	};

	enum class Login : uint8_t
	{
		Request,
		Result,
	};

	enum class Register : uint8_t
	{
		Check,
		CheckResult,
		Request,
		Result,
	};
}
