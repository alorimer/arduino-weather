////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _OPENWIRE_h
#define _OPENWIRE_h

#include <Mitov_SimpleList.h>

namespace OpenWire
{
	class Object
	{
	};
//---------------------------------------------------------------------------	
	class VirtualObject : public Object
	{
	public:
		virtual ~VirtualObject() {}
	};
//---------------------------------------------------------------------------
	class Component : public VirtualObject
	{
	public:
		static void _SystemInit();
		static void _SystemLoop();

	protected:
		virtual void CalculateFields() {}
		virtual void SystemInit()
		{
			CalculateFields();
		}

		virtual void SystemStart() {}
		virtual void SystemLoopBegin( unsigned long currentMicros ) {}
		virtual void SystemLoopEnd() {}
		virtual void SystemLoopUpdateHardware() {}

	public:
		Component();
		virtual ~Component();

	};
//---------------------------------------------------------------------------
	typedef void (Object::*TOnPinReceive) ( void *_Data );
//---------------------------------------------------------------------------
	typedef void (Object::*TOnPinIndexedReceive) ( int AIndex, void *_Data );
//---------------------------------------------------------------------------
	class Pin : public Object
	{
	public:
		virtual void InternalConnect( Pin &_other ) {}

	public:
		virtual void Connect( Pin &_other )
		{
			InternalConnect( _other );
			_other.InternalConnect( *this );
		}

		virtual void Receive( void *_Data ) {}
	};
//---------------------------------------------------------------------------
	class CallbackPin : public Pin
	{
	protected:
		Object       *OnReceiveObject = nullptr;
		TOnPinReceive OnReceive = nullptr;

	public:
		void SetCallback( Object *AOnReceiveObject, TOnPinReceive AOnReceive )
		{
			OnReceiveObject = AOnReceiveObject;
			OnReceive = AOnReceive;
		}

	public:
		virtual void InternalConnect( Pin &_other ) {}

	public:
		virtual void Receive( void *_Data ) override
		{
			if( OnReceive )
				( OnReceiveObject->*OnReceive )( _Data );

		}

	};
//---------------------------------------------------------------------------
	class StreamPin : public Pin
	{
	public:
		Mitov::SimpleList<OpenWire::Pin*> Pins;

	public:
		template<typename T> void SendValue( T AValue )
		{
			Notify( &AValue );
		}

		void SendValue( String AValue )
		{
			Notify( (char *)AValue.c_str() );
		}

	public:
		virtual void InternalConnect( Pin &_other ) override
		{
			Pins.push_back( &_other );
		}

	public:
		virtual bool IsConnected()
		{
			return ( Pins.size() > 0 );
		}

	public:
		virtual void Notify( void *_data )
		{
			for( Mitov::SimpleList<OpenWire::Pin*>::iterator Iter = Pins.begin(); Iter != Pins.end(); ++Iter )
				( *Iter )->Receive( _data );

		}
	};
//---------------------------------------------------------------------------
	class SourcePin : public StreamPin
	{
	};
//---------------------------------------------------------------------------
	class SinkPin : public CallbackPin
	{
	};
//---------------------------------------------------------------------------
	class ConnectSinkPin : public CallbackPin
	{
		typedef CallbackPin inherited;

	protected:
		bool	FIsConnected = false;

	public:
		virtual void InternalConnect( Pin &_other ) 
		{
			inherited::InternalConnect( _other );
			FIsConnected = true;
		}

		virtual bool IsConnected()
		{
			return FIsConnected;
		}

	};
//---------------------------------------------------------------------------
	template<typename T> class TypedSourcePin : public SourcePin
	{
		typedef SourcePin inherited;

	protected:
		T	FValue;

	public:
		void SetValue( T AValue, bool AChangeOnly )
		{
			if( AChangeOnly )
				if( AValue == FValue )
					return;

			FValue = AValue;
			Notify( &FValue );
		}

	};
//---------------------------------------------------------------------------
	template<typename T> class VlaueSinkPin : public SinkPin
	{
		typedef SinkPin inherited;

	public:
		T Value = T( 0 );

	public:
		virtual void Receive( void *_Data ) override
		{
			Value = *(T*)_Data;
			inherited::Receive( _Data );
		}
	};
//---------------------------------------------------------------------------
	template<> class VlaueSinkPin<String> : public SinkPin
	{
		typedef SinkPin inherited;

	public:
		String Value;

	public:
		virtual void Receive( void *_Data ) override
		{
			Value = (char *)_Data;
			inherited::Receive( _Data );
		}
	};
//---------------------------------------------------------------------------
	template<> class VlaueSinkPin<char *> : public SinkPin
	{
		typedef SinkPin inherited;

	public:
		String Value;

	public:
		virtual void Receive( void *_Data ) override
		{
			Value = (char*)_Data;
			inherited::Receive( _Data );
		}

	};
//---------------------------------------------------------------------------
	template<typename T> class VlaueChangeSinkPin : public SinkPin
	{
		typedef SinkPin inherited;

	public:
		T Value = 0;
		T OldValue = 0;

	public:
		virtual void Receive( void *_Data ) override
		{
			Value = *(T*)_Data;
			inherited::Receive( _Data );
		}

	};
//---------------------------------------------------------------------------
	template<> class VlaueChangeSinkPin<char *> : public SinkPin
	{
		typedef SinkPin inherited;

	public:
		String Value;
		String OldValue;

	public:
		virtual void Receive( void *_Data ) override
		{
			Value = (char *)_Data;
			inherited::Receive( _Data );
		}

	};
//---------------------------------------------------------------------------
	template<> class VlaueChangeSinkPin<String> : public SinkPin
	{
		typedef SinkPin inherited;

	public:
		String Value;
		String OldValue;

	public:
		virtual void Receive( void *_Data ) override
		{
			Value = (char *)_Data;
			inherited::Receive( _Data );
		}

	};
//---------------------------------------------------------------------------
	class IndexedSinkPin : public SinkPin
	{
		typedef SinkPin inherited;

	public:
		int Index = 0;

	public:
		TOnPinIndexedReceive OnIndexedReceive = nullptr;

	protected:
		virtual void Receive( void *_Data ) override
		{
			if( OnIndexedReceive )
				( OnReceiveObject->*OnIndexedReceive )( Index, _Data );
			
			inherited::Receive( _Data );
		}

	};
//---------------------------------------------------------------------------
	template<typename T> class LiveBindingSink : public OpenWire::Pin
	{
	protected:
		void (*FFunc)( T AData );

	protected:
		virtual void Receive( void *_Data ) override
		{
			FFunc(*(T*)_Data );
		}

	public:
		LiveBindingSink( void (*AFunc)( T AData ) ) :
			FFunc( AFunc )
		{
		}
	};
//---------------------------------------------------------------------------
	static Mitov::SimpleList<OpenWire::Component*> _Components;
//---------------------------------------------------------------------------
	Component::Component()
	{
		_Components.push_back(this);
	}
//---------------------------------------------------------------------------
	Component::~Component()
	{
		for( Mitov::SimpleList<OpenWire::Component *>::iterator Iter = OpenWire::_Components.begin(); Iter != OpenWire::_Components.end(); ++Iter )
			if( *Iter == this )
			{
				_Components.erase(Iter);
				break;
			}

	}
//---------------------------------------------------------------------------
	void Component::_SystemInit()
	{
		for( Mitov::SimpleList<OpenWire::Component *>::iterator Iter = OpenWire::_Components.begin(); Iter != OpenWire::_Components.end(); ++Iter )
			( *Iter )->SystemInit();

		for( Mitov::SimpleList<OpenWire::Component *>::iterator Iter = OpenWire::_Components.begin(); Iter != OpenWire::_Components.end(); ++Iter )
			( *Iter )->SystemStart();

	}
//---------------------------------------------------------------------------
	void Component::_SystemLoop()
	{
		unsigned long currentMicros = micros();
		for( Mitov::SimpleList<OpenWire::Component *>::iterator Iter = OpenWire::_Components.begin(); Iter != OpenWire::_Components.end(); ++Iter )
			( *Iter )->SystemLoopBegin( currentMicros );

		for( Mitov::SimpleList<OpenWire::Component *>::iterator Iter = OpenWire::_Components.begin(); Iter != OpenWire::_Components.end(); ++Iter )
			( *Iter )->SystemLoopEnd();

		for( Mitov::SimpleList<OpenWire::Component *>::iterator Iter = OpenWire::_Components.begin(); Iter != OpenWire::_Components.end(); ++Iter )
			( *Iter )->SystemLoopUpdateHardware();

	}
//---------------------------------------------------------------------------
}

#endif
