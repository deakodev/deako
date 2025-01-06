#pragma once

namespace Deako {

	/// Inheritor allows for compile-time polymorphism (vs runtime virtual inheritance) 
	/// and templates of common methods via the curiously recurring template pattern
	template <class BaseClass, class DerivedClass>
	class Inheritor : public BaseClass
	{
	public:
		template<typename... Args>
		Inheritor(Args&&... args)
			: BaseClass(std::forward<Args>(args)...)
		{
		}

		template<typename... Args>
		static Ref<DerivedClass> Create(Args&&... args)
		{
			return CreateRef<DerivedClass>(std::forward<Args>(args)...);
		}

		template<typename... Args>
		static Ref<DerivedClass> CreateIf(bool condition, Args&&... args)
		{
			if (condition)
			{
				return CreateRef<DerivedClass>(std::forward<Args>(args)...);
			}

			return nullptr;
		}

		std::size_t GetSizeOf() const { return sizeof(DerivedClass); }
		const char* GetTypeName() const { return TypeName<DerivedClass>(); }

	};

	template<typename T>
	constexpr const char* TypeName() { return typeid(T).name(); }

	/// Define helper to get the type name of a Deako:: type
#define DK_TYPE_NAME(T) \
        template<> constexpr const char* TypeName<T>() { return #T; } \
        template<> constexpr const char* TypeName<const T>() { return "const "#T; }


}