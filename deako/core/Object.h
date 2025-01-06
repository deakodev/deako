#pragma once

namespace Deako {

	/// 
	class Object
	{
	public:
		Object();

		void Hello() { return DK_CORE_INFO("Hello"); }

	private:

	};

	DK_TYPE_NAME(Object);

}