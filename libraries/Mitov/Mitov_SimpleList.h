// SimpleList.h

#ifndef _MITOV_SIMPLELIST_h
#define _MITOV_SIMPLELIST_h

//#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
//#else
//#include "WProgram.h"
//#endif

#ifndef NULL
#define NULL 0
#endif

namespace Mitov
{

template<typename T>
class SimpleList
{
public:
    ~SimpleList()
    {
        delete[] _list;
        _list = NULL;
        _size = 0;
    }

	void AddCount( int ACount )
	{
		for( int i = 0; i < ACount; ++i )
			push_back( T() );
	}

	void SetCount( int ACount )
	{
		while( _size < ACount )
			push_back( T() );

		while( _size > ACount )
			pop_back();
	}

	void SetCount( int ACount, T AValue )
	{
		while( _size < ACount )
			push_back( AValue );

		while( _size > ACount )
			pop_back();
	}

    void push_back(T obj)
    {
        ++_size;

        T* newBuffer = new T[_size];

        if( _list )
        {
            for (unsigned int i = 0; i < _size - 1; ++i)
                newBuffer[i] = _list[i];

            delete[] _list;
        }

        newBuffer[_size - 1] = obj;
        _list = newBuffer;
    }

    void append(T *obj, int ACount )
    {
		if( ! obj )
			return;

		if( !ACount )
			return;

		int AOldSize = _size;
        _size += ACount;

        T* newBuffer = new T[_size];
        if( _list )
        {
    		memcpy( newBuffer, _list, AOldSize * sizeof( T ));
    		delete[] _list;
        }

		memcpy( newBuffer + AOldSize, obj, ACount * sizeof( T ));

        _list = newBuffer;
    }

    void push_front(T obj)
    {
        ++_size;
        T* newBuffer = new T[_size];

        if( _list )
        {
            for (unsigned int i = _size - 1; i > 0; --i)
                newBuffer[i] = _list[i - 1];

    		delete[] _list;
        }

        newBuffer[0] = obj;
        _list = newBuffer;
    }

    void pop_back()
    {
        if (empty())
            return;

        --_size;
        if (empty())
        {
			delete[] _list;
            _list = NULL;
            return;
        }

        T* newBuffer = new T[_size];
		for (unsigned int i = 0; i < _size; ++i)
			newBuffer[i] = _list[i];

        delete[] _list;
        _list = newBuffer;
    }

    void pop_front( int ACount = 1 )
    {
        if (empty())
            return;

		int AOldSize = _size;
        _size -= ACount;
        if (empty())
        {
			delete[] _list;
            _list = NULL;
            return;
        }

        T* newBuffer = new T[_size];

		memmove( newBuffer, _list + ACount, AOldSize * sizeof( T ));

        delete[] _list;
        _list = newBuffer;
    }

/*
    void reserve(unsigned int size)
    {
        if (size <= _size)
            return;

        _preAlloc = size;

        T* newBuffer = new T[_preAlloc];

        for (unsigned int i = 0; i < _size; ++i)
            newBuffer[i] = _list[i];

        delete[] _list;
        _list = newBuffer;
    }
*/
    typedef T* iterator;

	iterator erase(iterator &itr)
	{
        if (empty())
            return NULL;

        --_size;
        if (empty())
        {
			delete[] _list;
            _list = NULL;

            return NULL;
        }

        T* newBuffer = new T[_size];

        bool sum = false;
        unsigned int pos = 0;
        for (unsigned int i = 0; i < _size; ++i)
        {
            if (_list + i == itr)
            {
                sum = true;
                pos = i;
            }

            if (sum)
                newBuffer[i] = _list[i + 1];

            else
                newBuffer[i] = _list[i];
        }

		delete[] _list;
        _list = newBuffer;

        itr = _list + pos;

        return itr;
    }

    inline iterator begin() { return (empty() ? NULL : _list); }
    inline iterator end() { return (empty() ? NULL : _list + _size); }

    void clear()
    {
        if (_list)
        {
            delete[] _list;
            _list = NULL;
        }

        _size = 0;
    }

    inline bool empty() const { return !_size; }
    inline unsigned int size() const { return _size; }
    inline operator T*() const { return _list; }

	SimpleList<T> & operator = ( const SimpleList<T> &other )
	{
		if( &other == this )
			return *this;

		clear();
		append( other, other.size() );

		return *this;
	}

protected:
    T* _list = nullptr;
    unsigned int _size = 0;
};

template<typename T>
class SimpleObjectList : public SimpleList<T>
{
	typedef SimpleList<T>	inherited;

public:
    ~SimpleObjectList()
    {
		for( int i = 0; i < inherited::_size; ++i )
			delete inherited::_list[ i ];
	}
};

} // Mitov

#endif
