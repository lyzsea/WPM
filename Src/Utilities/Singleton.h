#pragma once

#include <memory>
using namespace std;
#include "Interlocked.h"


template <class T>
class Singleton
{
public:
	static inline T* instance();

private:
	Singleton(void){}
	~Singleton(void){}
	Singleton(const Singleton&){}
	Singleton & operator= (const Singleton &){}

	static auto_ptr<T> _instance;
	static CResGuard _rs;
};

template <class T>
auto_ptr<T> Singleton<T>::_instance;

template <class T>
CResGuard Singleton<T>::_rs;

template <class T>
inline T* Singleton<T>::instance()
{
	if( 0 == _instance.get() )
	{
		CResGuard::CGuard gd(_rs);
		if( 0== _instance.get())
		{
			_instance.reset ( new T);
		}
	}
	return _instance.get();
}

#define DECLARE_SINGLETON_CLASS( type ) \
	friend class auto_ptr< type >;\
	friend class Singleton< type >;
