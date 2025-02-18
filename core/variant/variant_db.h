#ifndef VARIANT_REGISTRY_H
#define VARIANT_REGISTRY_H

#include <core/templates/hash_map.h>

#include "variant.h"
#include "variant_call.h"
#include "variant_construct.h"
#include "core/templates/local_vector.h"

class VariantExtensionType {
protected:
	StringName _name;
	bool _is_trivial = false;
	LocalVector<VariantConstructData> _constructors;
	BuiltinMethodMap _builtin_methods;

public:
	virtual ~VariantExtensionType() = default;

	_FORCE_INLINE_ bool is_trivial() const { return _is_trivial; }
	_FORCE_INLINE_ const StringName &get_name() const { return _name; }
	LocalVector<VariantConstructData> &get_constructors() { return _constructors; }
	const LocalVector<VariantConstructData> &get_constructors() const { return _constructors; }
	BuiltinMethodMap &get_builtin_methods() { return _builtin_methods; }
	const BuiltinMethodMap &get_builtin_methods() const { return _builtin_methods; }

	virtual void reference_init(Variant &p_variant, const Variant &p_arg) const {}
	virtual void destruct(Variant &p_variant) const {}
	virtual String stringify(const Variant &p_variant, int p_recursion_count) const {
		return "<" + Variant::get_type_name(p_variant.type) + ">";
	}
};

class VariantDB {
	static LocalVector<VariantExtensionType*> extensions;
	static HashMap<StringName, Variant::Type> type_by_name;

public:
	_FORCE_INLINE_ static bool type_exists(Variant::Type type) {
		return type > 0 && (type - Variant::VARIANT_MAX) < extensions.size();
	}

	_FORCE_INLINE_ static bool is_custom_type(Variant::Type type) {
		return type >= Variant::VARIANT_MAX && type - Variant::VARIANT_MAX < extensions.size();
	}

	_FORCE_INLINE_ static Variant::Type type_count() {
		return Variant::VARIANT_MAX + extensions.size();
	}

	static Variant::Type add_type(VariantExtensionType &type) {
		CRASH_COND_MSG(type.get_name().is_empty(), "Variant types must set _name");

		const Variant::Type new_id = static_cast<Variant::Type>(Variant::VARIANT_MAX + extensions.size());

		extensions.push_back(&type);
		type_by_name[type.get_name()] = new_id;

		return new_id;
	}

	static VariantExtensionType &get(Variant::Type type) {
		return *extensions[type - Variant::VARIANT_MAX];
	}

	static const Variant::Type *id_for_name(const StringName &name) {
		return type_by_name.getptr(name);
	}
};

#endif //VARIANT_REGISTRY_H
