/**************************************************************************/
/*  register_types.cpp                                                    */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "register_types.h"

#include "text_server_adv.h"

#include <core/variant/variant_db.h>

template <typename T, Variant::Type& VARIANT_TYPE>
class SharedIntVariantTypeConstructor;

struct SharedInt {
	static Variant::Type type_id;

	std::shared_ptr<int> ptr;

	SharedInt() {
		ptr = std::make_shared<int>(0);
	}
	SharedInt(int p_value) {
		ptr = std::make_shared<int>(p_value);
	}
	SharedInt(const SharedInt &p_other) {
		ptr = p_other.ptr;
	}

	_ALWAYS_INLINE_ int& operator*() const { return *ptr; }

	int get() const { return *ptr; }
	void update(int value) { *ptr = value; }
};

template <>
struct VariantGetInternalPtr<SharedInt> {
	static SharedInt *get_ptr(Variant *v) { return reinterpret_cast<SharedInt*>(v->_data._mem); }
	static const SharedInt *get_ptr(const Variant *v) { return reinterpret_cast<const SharedInt*>(v->_data._mem); }
};

#ifdef DEBUG_METHODS_ENABLED
#define bind_method(m_type, m_method, m_arg_names, m_default_args) \
	METHOD_CLASS(m_type, m_method, &m_type::m_method);             \
	_builtin_methods.insert(Method_##m_type##_##m_method::get_name(), create_builtin_method<Method_##m_type##_##m_method>(m_arg_names, m_default_args))
#else
#define bind_method(m_type, m_method, m_arg_names, m_default_args) \
	METHOD_CLASS(m_type, m_method, &m_type ::m_method);            \
	register_builtin_method<Method_##m_type##_##m_method>(sarray(), m_default_args);
#endif

#undef METHOD_CLASS
#define METHOD_CLASS(m_class, m_method_name, m_method_ptr)                                                                                                        \
	struct Method_##m_class##_##m_method_name {                                                                                                                   \
		static void call(Variant *base, const Variant **p_args, int p_argcount, Variant &r_ret, const Vector<Variant> &p_defvals, Callable::CallError &r_error) { \
			vc_method_call(m_method_ptr, base, p_args, p_argcount, r_ret, p_defvals, r_error);                                                                    \
		}                                                                                                                                                         \
		static void validated_call(Variant *base, const Variant **p_args, int p_argcount, Variant *r_ret) {                                                       \
			vc_validated_call(m_method_ptr, base, p_args, r_ret);                                                                                                 \
		}                                                                                                                                                         \
		static void ptrcall(void *p_base, const void **p_args, void *r_ret, int p_argcount) {                                                                     \
			vc_ptrcall(m_method_ptr, p_base, p_args, r_ret);                                                                                                      \
		}                                                                                                                                                         \
		static int get_argument_count() {                                                                                                                         \
			return vc_get_argument_count(m_method_ptr);                                                                                                           \
		}                                                                                                                                                         \
		static Variant::Type get_argument_type(int p_arg) {                                                                                                       \
			return vc_get_argument_type(m_method_ptr, p_arg);                                                                                                     \
		}                                                                                                                                                         \
		static Variant::Type get_return_type() {                                                                                                                  \
			return vc_get_return_type(m_method_ptr);                                                                                                              \
		}                                                                                                                                                         \
		static bool has_return_type() {                                                                                                                           \
			return vc_has_return_type(m_method_ptr);                                                                                                              \
		}                                                                                                                                                         \
		static bool is_const() {                                                                                                                                  \
			return vc_is_const(m_method_ptr);                                                                                                                     \
		}                                                                                                                                                         \
		static bool is_static() {                                                                                                                                 \
			return false;                                                                                                                                         \
		}                                                                                                                                                         \
		static bool is_vararg() {                                                                                                                                 \
			return false;                                                                                                                                         \
		}                                                                                                                                                         \
		static Variant::Type get_base_type() {                                                                                                                    \
			return SharedInt::type_id;                                                                                                                \
		}                                                                                                                                                         \
		static StringName get_name() {                                                                                                                            \
			return #m_method_name;                                                                                                                                \
		}                                                                                                                                                         \
	};

class SharedIntVariantType: public VariantExtensionType {
public:
	SharedIntVariantType() {
		_name = "SharedInt";
		_constructors.push_back(make_constructor<SharedIntVariantTypeConstructor<SharedIntVariantType, SharedInt::type_id>>(sarray()));

		bind_method(SharedInt, update, sarray("value"), varray());
		bind_method(SharedInt, get, sarray(), varray());
	}

	_FORCE_INLINE_ static SharedInt &as_ref_unsafe(Variant &p_variant) {
		return *VariantGetInternalPtr<SharedInt>::get_ptr(&p_variant);
	}

	_FORCE_INLINE_ static const SharedInt &as_ref_unsafe(const Variant &p_variant) {
		return *VariantGetInternalPtr<SharedInt>::get_ptr(&p_variant);
	}

	void reference_init(Variant &p_variant, const Variant &p_arg) const override {
		memnew_placement(p_variant._data._mem, SharedInt(as_ref_unsafe(p_arg)));
	}

	void destruct(Variant &p_variant) const override {
		reinterpret_cast<SharedInt*>(p_variant._data._mem)->~SharedInt();
	}

	String stringify(const Variant &p_variant, int p_recursion_count) const override {
		return itos(*as_ref_unsafe(p_variant));
	}
};

template <typename T, Variant::Type& VARIANT_TYPE>
class SharedIntVariantTypeConstructor {
public:
	static void construct(Variant &r_ret, const Variant **p_args, Callable::CallError &r_error) {
		memnew_placement(r_ret._data._mem, SharedInt);
		r_ret.type = SharedInt::type_id;

		r_error.error = Callable::CallError::CALL_OK;
	}

	static inline void validated_construct(Variant *r_ret, const Variant **p_args) {
		r_ret->type = SharedInt::type_id;
		memnew_placement(r_ret->_data._mem, SharedInt);
	}
	static void ptr_construct(void *base, const void **p_args) {
		memnew_placement(base, SharedInt);
	}

	static int get_argument_count() {
		return 0;
	}

	static Variant::Type get_argument_type(int p_arg) {
		return Variant::NIL;
	}

	static Variant::Type get_base_type() {
		return VARIANT_TYPE;
	}
};

Variant::Type SharedInt::type_id;

struct TinyUInt8Array {
	static Variant::Type type_id;

	std::array<uint8_t, 16> data;
};

template <typename T, Variant::Type& VARIANT_TYPE>
class TinyUInt8ArrayConstructor;

template <>
struct VariantGetInternalPtr<TinyUInt8Array> {
	static TinyUInt8Array *get_ptr(Variant *v) { return reinterpret_cast<TinyUInt8Array*>(v->_data._mem); }
	static const TinyUInt8Array *get_ptr(const Variant *v) { return reinterpret_cast<const TinyUInt8Array*>(v->_data._mem); }
};

class TinyUInt8ArrayVariantType: public VariantExtensionType {
public:
	TinyUInt8ArrayVariantType() {
		_is_trivial = true;
		_name = "TinyUInt8Array";
		_constructors.push_back(make_constructor<TinyUInt8ArrayConstructor<TinyUInt8ArrayVariantType, SharedInt::type_id>>(sarray()));
	}

	_FORCE_INLINE_ static TinyUInt8Array &as_ref_unsafe(Variant &p_variant) {
		return *VariantGetInternalPtr<TinyUInt8Array>::get_ptr(&p_variant);
	}

	_FORCE_INLINE_ static const TinyUInt8Array &as_ref_unsafe(const Variant &p_variant) {
		return *VariantGetInternalPtr<TinyUInt8Array>::get_ptr(&p_variant);
	}
};


template <typename T, Variant::Type& VARIANT_TYPE>
class TinyUInt8ArrayConstructor {
public:
	static void construct(Variant &r_ret, const Variant **p_args, Callable::CallError &r_error) {
		memnew_placement(r_ret._data._mem, TinyUInt8Array);
		r_ret.type = TinyUInt8Array::type_id;

		r_error.error = Callable::CallError::CALL_OK;
	}

	static inline void validated_construct(Variant *r_ret, const Variant **p_args) {
		r_ret->type = TinyUInt8Array::type_id;
		memnew_placement(r_ret->_data._mem, TinyUInt8Array);
	}
	static void ptr_construct(void *base, const void **p_args) {
		memnew_placement(base, TinyUInt8Array);
	}

	static int get_argument_count() {
		return 0;
	}

	static Variant::Type get_argument_type(int p_arg) {
		return Variant::NIL;
	}

	static Variant::Type get_base_type() {
		return VARIANT_TYPE;
	}
};

Variant::Type TinyUInt8Array::type_id;

class SharedIntObject: public RefCounted {
	GDCLASS(SharedIntObject, RefCounted);

	public:
	SharedInt value;
};

void initialize_text_server_adv_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SERVERS) {
		return;
	}

	GDREGISTER_CLASS(TextServerAdvanced);
	TextServerManager *tsman = TextServerManager::get_singleton();
	if (tsman) {
		Ref<TextServerAdvanced> ts;
		ts.instantiate();
		tsman->add_interface(ts);
	}

	static SharedIntVariantType shared_int_type = SharedIntVariantType();
	SharedInt::type_id = VariantDB::add_type(shared_int_type);
	CRASH_COND(sizeof(SharedInt) > sizeof(Variant::_data));

	static TinyUInt8ArrayVariantType tiny_uint8_array_type = TinyUInt8ArrayVariantType();
	TinyUInt8Array::type_id = VariantDB::add_type(tiny_uint8_array_type);
	CRASH_COND(sizeof(TinyUInt8Array) > sizeof(Variant::_data));

	GDREGISTER_CLASS(SharedIntObject);
}

void uninitialize_text_server_adv_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SERVERS) {
		return;
	}
}

#ifdef GDEXTENSION

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/memory.hpp>

using namespace godot;

extern "C" {

GDExtensionBool GDE_EXPORT textserver_advanced_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(&initialize_text_server_adv_module);
	init_obj.register_terminator(&uninitialize_text_server_adv_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SERVERS);

	return init_obj.init();
}

} // ! extern "C"

#endif // ! GDEXTENSION
