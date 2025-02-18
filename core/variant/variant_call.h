#ifndef VARIANT_CALL_H
#define VARIANT_CALL_H

#include "variant.h"
#include "binder_common.h"
#include "variant_internal.h"
#include "core/object/object.h"
#include "core/templates/oa_hash_map.h"

struct VariantBuiltInMethodInfo {
	void (*call)(Variant *base, const Variant **p_args, int p_argcount, Variant &r_ret, const Vector<Variant> &p_defvals, Callable::CallError &r_error) = nullptr;
	Variant::ValidatedBuiltInMethod validated_call = nullptr;
	Variant::PTRBuiltInMethod ptrcall = nullptr;

	Vector<Variant> default_arguments;
	Vector<String> argument_names;

	bool is_const = false;
	bool is_static = false;
	bool has_return_type = false;
	bool is_vararg = false;
	Variant::Type return_type;
	int argument_count = 0;
	Variant::Type (*get_argument_type)(int p_arg) = nullptr;

	MethodInfo get_method_info(const StringName &p_name) const {
		MethodInfo mi;
		mi.name = p_name;

		if (has_return_type) {
			mi.return_val.type = return_type;
			if (mi.return_val.type == Variant::NIL) {
				mi.return_val.usage |= PROPERTY_USAGE_NIL_IS_VARIANT;
			}
		}

		if (is_const) {
			mi.flags |= METHOD_FLAG_CONST;
		}
		if (is_vararg) {
			mi.flags |= METHOD_FLAG_VARARG;
		}
		if (is_static) {
			mi.flags |= METHOD_FLAG_STATIC;
		}

		for (int i = 0; i < argument_count; i++) {
			PropertyInfo pi;
#ifdef DEBUG_METHODS_ENABLED
			pi.name = argument_names[i];
#else
			pi.name = "arg" + itos(i + 1);
#endif
			pi.type = (*get_argument_type)(i);
			if (pi.type == Variant::NIL) {
				pi.usage |= PROPERTY_USAGE_NIL_IS_VARIANT;
			}
			mi.arguments.push_back(pi);
		}

		mi.default_arguments = default_arguments;

		return mi;
	}
};

typedef OAHashMap<StringName, VariantBuiltInMethodInfo> BuiltinMethodMap;


typedef void (*VariantFunc)(Variant &r_ret, Variant &p_self, const Variant **p_args);
typedef void (*VariantConstructFunc)(Variant &r_ret, const Variant **p_args);

template <typename R, typename... P>
static _FORCE_INLINE_ void vc_static_method_call(R (*method)(P...), const Variant **p_args, int p_argcount, Variant &r_ret, const Vector<Variant> &p_defvals, Callable::CallError &r_error) {
	call_with_variant_args_static_ret_dv(method, p_args, p_argcount, r_ret, r_error, p_defvals);
}

template <typename... P>
static _FORCE_INLINE_ void vc_static_method_call(void (*method)(P...), const Variant **p_args, int p_argcount, Variant &r_ret, const Vector<Variant> &p_defvals, Callable::CallError &r_error) {
	call_with_variant_args_static_dv(method, p_args, p_argcount, r_error, p_defvals);
}

template <typename R, typename T, typename... P>
static _FORCE_INLINE_ void vc_method_call(R (T::*method)(P...), Variant *base, const Variant **p_args, int p_argcount, Variant &r_ret, const Vector<Variant> &p_defvals, Callable::CallError &r_error) {
	call_with_variant_args_ret_dv(VariantGetInternalPtr<T>::get_ptr(base), method, p_args, p_argcount, r_ret, r_error, p_defvals);
}

template <typename R, typename T, typename... P>
static _FORCE_INLINE_ void vc_method_call(R (T::*method)(P...) const, Variant *base, const Variant **p_args, int p_argcount, Variant &r_ret, const Vector<Variant> &p_defvals, Callable::CallError &r_error) {
	call_with_variant_args_retc_dv(VariantGetInternalPtr<T>::get_ptr(base), method, p_args, p_argcount, r_ret, r_error, p_defvals);
}

template <typename T, typename... P>
static _FORCE_INLINE_ void vc_method_call(void (T::*method)(P...), Variant *base, const Variant **p_args, int p_argcount, Variant &r_ret, const Vector<Variant> &p_defvals, Callable::CallError &r_error) {
	VariantInternal::clear(&r_ret);
	call_with_variant_args_dv(VariantGetInternalPtr<T>::get_ptr(base), method, p_args, p_argcount, r_error, p_defvals);
}

template <typename T, typename... P>
static _FORCE_INLINE_ void vc_method_call(void (T::*method)(P...) const, Variant *base, const Variant **p_args, int p_argcount, Variant &r_ret, const Vector<Variant> &p_defvals, Callable::CallError &r_error) {
	VariantInternal::clear(&r_ret);
	call_with_variant_argsc_dv(VariantGetInternalPtr<T>::get_ptr(base), method, p_args, p_argcount, r_error, p_defvals);
}

template <typename From, typename R, typename T, typename... P>
static _FORCE_INLINE_ void vc_convert_method_call(R (T::*method)(P...), Variant *base, const Variant **p_args, int p_argcount, Variant &r_ret, const Vector<Variant> &p_defvals, Callable::CallError &r_error) {
	T converted(static_cast<T>(*VariantGetInternalPtr<From>::get_ptr(base)));
	call_with_variant_args_ret_dv(&converted, method, p_args, p_argcount, r_ret, r_error, p_defvals);
}

template <typename From, typename R, typename T, typename... P>
static _FORCE_INLINE_ void vc_convert_method_call(R (T::*method)(P...) const, Variant *base, const Variant **p_args, int p_argcount, Variant &r_ret, const Vector<Variant> &p_defvals, Callable::CallError &r_error) {
	T converted(static_cast<T>(*VariantGetInternalPtr<From>::get_ptr(base)));
	call_with_variant_args_retc_dv(&converted, method, p_args, p_argcount, r_ret, r_error, p_defvals);
}

template <typename From, typename T, typename... P>
static _FORCE_INLINE_ void vc_convert_method_call(void (T::*method)(P...), Variant *base, const Variant **p_args, int p_argcount, Variant &r_ret, const Vector<Variant> &p_defvals, Callable::CallError &r_error) {
	T converted(static_cast<T>(*VariantGetInternalPtr<From>::get_ptr(base)));
	call_with_variant_args_dv(&converted, method, p_args, p_argcount, r_error, p_defvals);
}

template <typename From, typename T, typename... P>
static _FORCE_INLINE_ void vc_convert_method_call(void (T::*method)(P...) const, Variant *base, const Variant **p_args, int p_argcount, Variant &r_ret, const Vector<Variant> &p_defvals, Callable::CallError &r_error) {
	T converted(static_cast<T>(*VariantGetInternalPtr<From>::get_ptr(base)));
	call_with_variant_argsc_dv(&converted, method, p_args, p_argcount, r_error, p_defvals);
}

template <typename R, typename T, typename... P>
static _FORCE_INLINE_ void vc_method_call_static(R (*method)(T *, P...), Variant *base, const Variant **p_args, int p_argcount, Variant &r_ret, const Vector<Variant> &p_defvals, Callable::CallError &r_error) {
	call_with_variant_args_retc_static_helper_dv(VariantGetInternalPtr<T>::get_ptr(base), method, p_args, p_argcount, r_ret, p_defvals, r_error);
}

template <typename T, typename... P>
static _FORCE_INLINE_ void vc_method_call_static(void (*method)(T *, P...), Variant *base, const Variant **p_args, int p_argcount, Variant &r_ret, const Vector<Variant> &p_defvals, Callable::CallError &r_error) {
	call_with_variant_args_static_helper_dv(VariantGetInternalPtr<T>::get_ptr(base), method, p_args, p_argcount, p_defvals, r_error);
}

template <typename R, typename T, typename... P>
static _FORCE_INLINE_ void vc_validated_call(R (T::*method)(P...), Variant *base, const Variant **p_args, Variant *r_ret) {
	call_with_validated_variant_args_ret(base, method, p_args, r_ret);
}

template <typename R, typename T, typename... P>
static _FORCE_INLINE_ void vc_validated_call(R (T::*method)(P...) const, Variant *base, const Variant **p_args, Variant *r_ret) {
	call_with_validated_variant_args_retc(base, method, p_args, r_ret);
}
template <typename T, typename... P>
static _FORCE_INLINE_ void vc_validated_call(void (T::*method)(P...), Variant *base, const Variant **p_args, Variant *r_ret) {
	call_with_validated_variant_args(base, method, p_args);
}

template <typename T, typename... P>
static _FORCE_INLINE_ void vc_validated_call(void (T::*method)(P...) const, Variant *base, const Variant **p_args, Variant *r_ret) {
	call_with_validated_variant_argsc(base, method, p_args);
}

template <typename From, typename R, typename T, typename... P>
static _FORCE_INLINE_ void vc_convert_validated_call(R (T::*method)(P...), Variant *base, const Variant **p_args, Variant *r_ret) {
	T converted(static_cast<T>(*VariantGetInternalPtr<From>::get_ptr(base)));
	call_with_validated_variant_args_ret_helper<T, R, P...>(&converted, method, p_args, r_ret, BuildIndexSequence<sizeof...(P)>{});
}

template <typename From, typename R, typename T, typename... P>
static _FORCE_INLINE_ void vc_convert_validated_call(R (T::*method)(P...) const, Variant *base, const Variant **p_args, Variant *r_ret) {
	T converted(static_cast<T>(*VariantGetInternalPtr<From>::get_ptr(base)));
	call_with_validated_variant_args_retc_helper<T, R, P...>(&converted, method, p_args, r_ret, BuildIndexSequence<sizeof...(P)>{});
}
template <typename From, typename T, typename... P>
static _FORCE_INLINE_ void vc_convert_validated_call(void (T::*method)(P...), Variant *base, const Variant **p_args, Variant *r_ret) {
	T converted(static_cast<T>(*VariantGetInternalPtr<From>::get_ptr(base)));
	call_with_validated_variant_args_helper<T, P...>(&converted, method, p_args, r_ret, BuildIndexSequence<sizeof...(P)>{});
}

template <typename From, typename T, typename... P>
static _FORCE_INLINE_ void vc_convert_validated_call(void (T::*method)(P...) const, Variant *base, const Variant **p_args, Variant *r_ret) {
	T converted(static_cast<T>(*VariantGetInternalPtr<From>::get_ptr(base)));
	call_with_validated_variant_argsc_helper<T, P...>(&converted, method, p_args, r_ret, BuildIndexSequence<sizeof...(P)>{});
}

template <typename R, typename T, typename... P>
static _FORCE_INLINE_ void vc_validated_call_static(R (*method)(T *, P...), Variant *base, const Variant **p_args, Variant *r_ret) {
	call_with_validated_variant_args_static_retc(base, method, p_args, r_ret);
}

template <typename T, typename... P>
static _FORCE_INLINE_ void vc_validated_call_static(void (*method)(T *, P...), Variant *base, const Variant **p_args, Variant *r_ret) {
	call_with_validated_variant_args_static(base, method, p_args);
}

template <typename R, typename... P>
static _FORCE_INLINE_ void vc_validated_static_call(R (*method)(P...), const Variant **p_args, Variant *r_ret) {
	call_with_validated_variant_args_static_method_ret(method, p_args, r_ret);
}

template <typename... P>
static _FORCE_INLINE_ void vc_validated_static_call(void (*method)(P...), const Variant **p_args, Variant *r_ret) {
	call_with_validated_variant_args_static_method(method, p_args);
}

template <typename R, typename T, typename... P>
static _FORCE_INLINE_ void vc_ptrcall(R (T::*method)(P...), void *p_base, const void **p_args, void *r_ret) {
	call_with_ptr_args_ret(reinterpret_cast<T *>(p_base), method, p_args, r_ret);
}

template <typename R, typename T, typename... P>
static _FORCE_INLINE_ void vc_ptrcall(R (T::*method)(P...) const, void *p_base, const void **p_args, void *r_ret) {
	call_with_ptr_args_retc(reinterpret_cast<T *>(p_base), method, p_args, r_ret);
}

template <typename T, typename... P>
static _FORCE_INLINE_ void vc_ptrcall(void (T::*method)(P...), void *p_base, const void **p_args, void *r_ret) {
	call_with_ptr_args(reinterpret_cast<T *>(p_base), method, p_args);
}

template <typename T, typename... P>
static _FORCE_INLINE_ void vc_ptrcall(void (T::*method)(P...) const, void *p_base, const void **p_args, void *r_ret) {
	call_with_ptr_argsc(reinterpret_cast<T *>(p_base), method, p_args);
}

template <typename From, typename R, typename T, typename... P>
static _FORCE_INLINE_ void vc_convert_ptrcall(R (T::*method)(P...), void *p_base, const void **p_args, void *r_ret) {
	T converted(*reinterpret_cast<From *>(p_base));
	call_with_ptr_args_ret(&converted, method, p_args, r_ret);
}

template <typename From, typename R, typename T, typename... P>
static _FORCE_INLINE_ void vc_convert_ptrcall(R (T::*method)(P...) const, void *p_base, const void **p_args, void *r_ret) {
	T converted(*reinterpret_cast<From *>(p_base));
	call_with_ptr_args_retc(&converted, method, p_args, r_ret);
}

template <typename From, typename T, typename... P>
static _FORCE_INLINE_ void vc_convert_ptrcall(void (T::*method)(P...), void *p_base, const void **p_args, void *r_ret) {
	T converted(*reinterpret_cast<From *>(p_base));
	call_with_ptr_args(&converted, method, p_args);
}

template <typename From, typename T, typename... P>
static _FORCE_INLINE_ void vc_convert_ptrcall(void (T::*method)(P...) const, void *p_base, const void **p_args, void *r_ret) {
	T converted(*reinterpret_cast<From *>(p_base));
	call_with_ptr_argsc(&converted, method, p_args);
}

template <typename R, typename T, typename... P>
static _FORCE_INLINE_ int vc_get_argument_count(R (T::*method)(P...)) {
	return sizeof...(P);
}
template <typename R, typename T, typename... P>
static _FORCE_INLINE_ int vc_get_argument_count(R (T::*method)(P...) const) {
	return sizeof...(P);
}

template <typename T, typename... P>
static _FORCE_INLINE_ int vc_get_argument_count(void (T::*method)(P...)) {
	return sizeof...(P);
}

template <typename T, typename... P>
static _FORCE_INLINE_ int vc_get_argument_count(void (T::*method)(P...) const) {
	return sizeof...(P);
}

template <typename R, typename T, typename... P>
static _FORCE_INLINE_ int vc_get_argument_count(R (*method)(T *, P...)) {
	return sizeof...(P);
}

template <typename R, typename... P>
static _FORCE_INLINE_ int vc_get_argument_count_static(R (*method)(P...)) {
	return sizeof...(P);
}

template <typename R, typename T, typename... P>
static _FORCE_INLINE_ Variant::Type vc_get_argument_type(R (T::*method)(P...), int p_arg) {
	return call_get_argument_type<P...>(p_arg);
}
template <typename R, typename T, typename... P>
static _FORCE_INLINE_ Variant::Type vc_get_argument_type(R (T::*method)(P...) const, int p_arg) {
	return call_get_argument_type<P...>(p_arg);
}

template <typename T, typename... P>
static _FORCE_INLINE_ Variant::Type vc_get_argument_type(void (T::*method)(P...), int p_arg) {
	return call_get_argument_type<P...>(p_arg);
}

template <typename T, typename... P>
static _FORCE_INLINE_ Variant::Type vc_get_argument_type(void (T::*method)(P...) const, int p_arg) {
	return call_get_argument_type<P...>(p_arg);
}

template <typename R, typename T, typename... P>
static _FORCE_INLINE_ Variant::Type vc_get_argument_type(R (*method)(T *, P...), int p_arg) {
	return call_get_argument_type<P...>(p_arg);
}

template <typename R, typename... P>
static _FORCE_INLINE_ Variant::Type vc_get_argument_type_static(R (*method)(P...), int p_arg) {
	return call_get_argument_type<P...>(p_arg);
}

template <typename R, typename T, typename... P>
static _FORCE_INLINE_ Variant::Type vc_get_return_type(R (T::*method)(P...)) {
	return GetTypeInfo<R>::VARIANT_TYPE;
}

template <typename R, typename T, typename... P>
static _FORCE_INLINE_ Variant::Type vc_get_return_type(R (T::*method)(P...) const) {
	return GetTypeInfo<R>::VARIANT_TYPE;
}

template <typename T, typename... P>
static _FORCE_INLINE_ Variant::Type vc_get_return_type(void (T::*method)(P...)) {
	return Variant::NIL;
}

template <typename T, typename... P>
static _FORCE_INLINE_ Variant::Type vc_get_return_type(void (T::*method)(P...) const) {
	return Variant::NIL;
}

template <typename R, typename... P>
static _FORCE_INLINE_ Variant::Type vc_get_return_type(R (*method)(P...)) {
	return GetTypeInfo<R>::VARIANT_TYPE;
}

template <typename... P>
static _FORCE_INLINE_ Variant::Type vc_get_return_type(void (*method)(P...)) {
	return Variant::NIL;
}

template <typename R, typename T, typename... P>
static _FORCE_INLINE_ bool vc_has_return_type(R (T::*method)(P...)) {
	return true;
}
template <typename R, typename T, typename... P>
static _FORCE_INLINE_ bool vc_has_return_type(R (T::*method)(P...) const) {
	return true;
}

template <typename T, typename... P>
static _FORCE_INLINE_ bool vc_has_return_type(void (T::*method)(P...)) {
	return false;
}

template <typename T, typename... P>
static _FORCE_INLINE_ bool vc_has_return_type(void (T::*method)(P...) const) {
	return false;
}

template <typename... P>
static _FORCE_INLINE_ bool vc_has_return_type_static(void (*method)(P...)) {
	return false;
}

template <typename R, typename... P>
static _FORCE_INLINE_ bool vc_has_return_type_static(R (*method)(P...)) {
	return true;
}

template <typename R, typename T, typename... P>
static _FORCE_INLINE_ bool vc_is_const(R (T::*method)(P...)) {
	return false;
}
template <typename R, typename T, typename... P>
static _FORCE_INLINE_ bool vc_is_const(R (T::*method)(P...) const) {
	return true;
}

template <typename T, typename... P>
static _FORCE_INLINE_ bool vc_is_const(void (T::*method)(P...)) {
	return false;
}

template <typename T, typename... P>
static _FORCE_INLINE_ bool vc_is_const(void (T::*method)(P...) const) {
	return true;
}

template <typename R, typename T, typename... P>
static _FORCE_INLINE_ Variant::Type vc_get_base_type(R (T::*method)(P...)) {
	return GetTypeInfo<T>::VARIANT_TYPE;
}
template <typename R, typename T, typename... P>
static _FORCE_INLINE_ Variant::Type vc_get_base_type(R (T::*method)(P...) const) {
	return GetTypeInfo<T>::VARIANT_TYPE;
}

template <typename T, typename... P>
static _FORCE_INLINE_ Variant::Type vc_get_base_type(void (T::*method)(P...)) {
	return GetTypeInfo<T>::VARIANT_TYPE;
}

template <typename T, typename... P>
static _FORCE_INLINE_ Variant::Type vc_get_base_type(void (T::*method)(P...) const) {
	return GetTypeInfo<T>::VARIANT_TYPE;
}

template <typename T>
static VariantBuiltInMethodInfo create_builtin_method(const Vector<String> &p_argnames, const Vector<Variant> &p_def_args) {
	StringName name = T::get_name();

	VariantBuiltInMethodInfo imi;

	imi.call = T::call;
	imi.validated_call = T::validated_call;
	imi.ptrcall = T::ptrcall;

	imi.default_arguments = p_def_args;
	imi.argument_names = p_argnames;

	imi.is_const = T::is_const();
	imi.is_static = T::is_static();
	imi.is_vararg = T::is_vararg();
	imi.has_return_type = T::has_return_type();
	imi.return_type = T::get_return_type();
	imi.argument_count = T::get_argument_count();
	imi.get_argument_type = T::get_argument_type;

	return imi;
}

#endif //VARIANT_CALL_H
