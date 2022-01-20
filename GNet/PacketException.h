#pragma once
#include <iostream>

namespace GNet
{
	class PacketException
	{
	public:
		PacketException(std::string exception) 
			: m_exception(exception) 
		{}

		const char* what()
		{
			return m_exception.c_str();
		}

		std::string toString()
		{
			return m_exception;
		}

	private:
		std::string m_exception;
	};
}
