#ifndef CLASS_H_
#define CLASS_H_

#define DEFINE_CLASS(name, type, exit, init, init_args...)   \
	typedef type       class_##name##_t;                     \
	static inline void class_##name##_destructor(type *p)    \
	{                                                        \
		type _T = *p;                                        \
		exit;                                                \
	}                                                        \
	static inline type class_##name##_constructor(init_args) \
	{                                                        \
		type t = init;                                       \
		return t;                                            \
	}

#define CLASS(name, var) \
	class_##name##_t var __cleanup(class_##name##_destructor) = class_##name##_constructor

#define DEFINE_GUARD(name, type, lock, unlock) \
	DEFINE_CLASS(name, type, unlock, ({        \
		lock;                                  \
		_T;                                    \
	}),                                        \
	  type _T)
#define guard(name) CLASS(name, __UNIQUE_ID(guard))

#endif // CLASS_H_
