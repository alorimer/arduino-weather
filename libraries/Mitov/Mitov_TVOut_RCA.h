////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_TVOUT_RCA_h
#define _MITOV_TVOUT_RCA_h

#include <Mitov.h>
#include <TVout.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class TVOut_RCA_Intf;
//---------------------------------------------------------------------------
	class TVOutRCAElementBasic : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public:
		virtual void Render() = 0;
		virtual void TVBegin() = 0;

	public: // Public for the print access
		Mitov::TVOut_RCA_Intf	&FOwner;

	public:
		TVOutRCAElementBasic( Mitov::TVOut_RCA_Intf &AOwner );

	};
//---------------------------------------------------------------------------
	class TVOut_RCA_Intf
	{
	public:
		virtual void RegisterRender( TVOutRCAElementBasic *AItem ) {}
		virtual bool IsEnabled() = 0;
		virtual TVout &GetTV() = 0;

		virtual void TV_shift( bool _InVertical, int32_t _Distance ) = 0;
		virtual void TV_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, char c) = 0;
		virtual void TV_bitmap(uint8_t x, uint8_t y, const unsigned char * bmp, uint16_t i = 0, uint8_t width = 0, uint8_t lines = 0) = 0;
		virtual void TV_set_cursor(uint8_t, uint8_t) = 0;
		virtual void TV_draw_circle(uint8_t x0, uint8_t y0, uint8_t radius, char c, char fc = -1) = 0;
		virtual void TV_draw_rect(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, char c, char fc = -1) = 0;
		virtual void TV_set_pixel(uint8_t x, uint8_t y, char c) = 0;
		virtual unsigned char TV_get_pixel(uint8_t x, uint8_t y) = 0;

	};
//---------------------------------------------------------------------------
	enum TArduinoTVOutRCAElementColor { tvcBlack, tvcWhite, tvcInvert, tvcNone = -1 };
//---------------------------------------------------------------------------
	class TVOutRCAElementClocked : public Mitov::TVOutRCAElementBasic, public Mitov::ClockingSupport
	{
		typedef Mitov::TVOutRCAElementBasic inherited;

	public:
		virtual void DoClockReceive( void *_Data ) override
		{
			Render();
		}

	public:
		virtual void TVBegin() override
		{
			if( ! ClockInputPin.IsConnected() )
				Render();
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class TVOutRCAElementColored : public TVOutRCAElementClocked
	{
		typedef Mitov::TVOutRCAElementClocked inherited;

	public:
		TArduinoTVOutRCAElementColor	Color = tvcWhite;

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class TVOutRCAElementFillColored : public TVOutRCAElementClocked
	{
		typedef Mitov::TVOutRCAElementClocked inherited;

	public:
		TArduinoTVOutRCAElementColor	BorderColor = tvcWhite;
		TArduinoTVOutRCAElementColor	FillColor = tvcBlack;

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class TVOut_RCA : public OpenWire::Component, public TVOut_RCA_Intf
	{
		typedef OpenWire::Component inherited;

    public:
        bool	Enabled : 1;
		bool	IsPAL : 1;
		int32_t	Width = 128;
		int32_t	Height = 96;

	public:
		void SetEnabled( bool AVelue )
		{
			if( Enabled == AVelue )
				return;

			Enabled = AVelue;

			if( Enabled )
				TVBegin();

			else
				FTV.end();
		}

	public:
		TVout	FTV;

	protected:
		Mitov::SimpleList<TVOutRCAElementBasic *>	FElements;

	public:
		virtual void RegisterRender( TVOutRCAElementBasic *AItem ) override
		{
			FElements.push_back( AItem );
		}

		virtual void TV_shift( bool _InVertical, int32_t _Distance ) override
		{
			if( ! Enabled )
				return;

			uint8_t ADistance = abs( _Distance );
			uint8_t ADirection;
			if( _InVertical )
			{
				if( _Distance < 0 )
					ADirection = UP;

				else
					ADirection = DOWN;

			}
			else
			{
				if( _Distance < 0 )
					ADirection = LEFT;

				else
					ADirection = RIGHT;

			}

			FTV.shift( ADistance, ADirection );
		}

		virtual void TV_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, char c) override
		{
			if( Enabled )
				FTV.draw_line( x0, y0, x1, y1, c );
		}

		virtual void TV_draw_circle(uint8_t x0, uint8_t y0, uint8_t radius, char c, char fc = -1) override
		{
			if( Enabled )
				FTV.draw_circle( x0, y0, radius, c, fc );
		}

		virtual void TV_draw_rect(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, char c, char fc = -1) override
		{
			if( Enabled )
				FTV.draw_rect( x0, y0, w, h, c, fc );
		}

		virtual void TV_set_pixel(uint8_t x, uint8_t y, char c) override
		{
			if( Enabled )
				FTV.set_pixel( x, y, c );
		}

		virtual unsigned char TV_get_pixel(uint8_t x, uint8_t y) override
		{
			return FTV.get_pixel( x, y );
		}

		virtual void TV_bitmap(uint8_t x, uint8_t y, const unsigned char * bmp, uint16_t i = 0, uint8_t width = 0, uint8_t lines = 0) override
		{
			if( Enabled )
				FTV.bitmap( x, y, bmp, i, width, lines );
		}

		virtual void TV_set_cursor( uint8_t x, uint8_t y ) override
		{
			if( Enabled )
				FTV.set_cursor( x, y );
		}

	public:
		virtual bool IsEnabled() override
		{
			return Enabled;
		}

		virtual TVout &GetTV() override
		{
			return FTV;
		}

	protected:
		void TVBegin()
		{
			FTV.begin( IsPAL ? PAL : NTSC, Width, Height );

			for( int i = 0; i < FElements.size(); i ++ )
				FElements[ i ]->TVBegin();

//			FTV.select_font( font4x6 );
		}

	protected:
		virtual void SystemInit() override
		{
			if( Enabled )
				FTV.begin( IsPAL ? PAL : NTSC, Width, Height );

			inherited::SystemInit();
		}

		virtual void SystemStart() override
		{
			if( Enabled )
				for( int i = 0; i < FElements.size(); i ++ )
					FElements[ i ]->TVBegin();

			inherited::SystemStart();
		}

	public:
		TVOut_RCA() :
			IsPAL( false ),
			Enabled( true )
		{
		}

	};
//---------------------------------------------------------------------------
	class TVOutRCAElementFillScreen : public TVOutRCAElementColored
	{
		typedef Mitov::TVOutRCAElementColored inherited;

	public:
		TArduinoTVOutRCAElementColor	Color = tvcBlack;

	public:
		virtual void Render() override
		{
			if( FOwner.IsEnabled() )
				FOwner.GetTV().fill( (int)Color );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class TVOutRCAElementPlayTone : public TVOutRCAElementClocked
	{
		typedef Mitov::TVOutRCAElementClocked inherited;

	public:
		unsigned int	Frequency = 1760;
		unsigned long	Duration = 1000;

	public:
		virtual void Render() override
		{
			if( FOwner.IsEnabled() )
			{
				if( Frequency == 0 )
					FOwner.GetTV().noTone();

				else
					FOwner.GetTV().tone( Frequency, Duration );
			}
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class TVOutRCAElementDrawCircle : public TVOutRCAElementFillColored
	{
		typedef Mitov::TVOutRCAElementFillColored inherited;

	public:
		int32_t	X = 10;
		int32_t	Y = 10;
		int32_t	Radius = 10;

	public:
		virtual void Render() override
		{
			FOwner.TV_draw_circle( X, Y, Radius, (int)BorderColor, (int)FillColor );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class TVOutRCAElementDrawRectangle : public TVOutRCAElementFillColored
	{
		typedef Mitov::TVOutRCAElementFillColored inherited;

	public:
		int32_t	X = 0;
		int32_t	Y = 0;
		int32_t	Width = 10;
		int32_t	Height = 10;

	public:
		virtual void Render() override
		{
			FOwner.TV_draw_rect( X, Y, Width, Height, (int)BorderColor, (int)FillColor );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class TVOutRCAElementDrawLine : public TVOutRCAElementColored
	{
		typedef Mitov::TVOutRCAElementColored inherited;

	public:
		int32_t	X1 = 0;
		int32_t	Y1 = 0;
		int32_t	X2 = 10;
		int32_t	Y2 = 10;

	public:
		virtual void Render() override
		{
			FOwner.TV_draw_line( X1, Y1, X2, Y2, (int)Color );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class TVOutRCAElementDrawPixel : public TVOutRCAElementColored
	{
		typedef Mitov::TVOutRCAElementColored inherited;

	public:
		int32_t	X = 0;
		int32_t	Y = 0;

	public:
		virtual void Render() override
		{
			FOwner.TV_set_pixel( X, Y, (int)Color );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class TVOutRCAElementShiftScreen : public Mitov::TVOutRCAElementClocked
	{
		typedef Mitov::TVOutRCAElementClocked inherited;

	public:
		int32_t	Distance = 1;
		bool	InVertical = false;

	public:
		virtual void Render() override
		{
			FOwner.TV_shift( InVertical, Distance );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class TVOutRCAElementBasicPosition : public Mitov::TVOutRCAElementClocked
	{
		typedef Mitov::TVOutRCAElementClocked inherited;

    public:
		OpenWire::SourcePin	OutputPin;

	public:
		int32_t	X = 0;
		int32_t	Y = 0;

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class TVOutRCAElementDrawScene : public Mitov::TVOutRCAElementBasicPosition, public TVOut_RCA_Intf
	{
		typedef Mitov::TVOutRCAElementBasicPosition inherited;

	protected:
		Mitov::SimpleList<TVOutRCAElementBasic *>	FElements;

	public:
		virtual void RegisterRender( TVOutRCAElementBasic *AItem )
		{
			FElements.push_back( AItem );
		}

		virtual bool IsEnabled()
		{
			return FOwner.IsEnabled();
		}

		virtual TVout &GetTV()
		{
			return FOwner.GetTV();
		}

		virtual void TV_shift( bool _InVertical, int32_t _Distance )
		{
			FOwner.TV_shift( _InVertical, _Distance );
		}

		virtual void TV_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, char c)
		{
			FOwner.TV_draw_line( X + x0, Y + y0, X + x1, Y + y1, c );
		}

		virtual void TV_bitmap(uint8_t x, uint8_t y, const unsigned char * bmp, uint16_t i = 0, uint8_t width = 0, uint8_t lines = 0)
		{
			FOwner.TV_bitmap( X + x , Y + y, bmp, i, width, lines );
		}

		virtual void TV_set_cursor( uint8_t x, uint8_t y )
		{
			FOwner.TV_set_cursor( X + x , Y + y );
		}

		virtual void TV_draw_circle(uint8_t x0, uint8_t y0, uint8_t radius, char c, char fc = -1)
		{
			FOwner.TV_draw_circle( X + x0, Y + y0, radius, c, fc );
		}

		virtual void TV_draw_rect(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, char c, char fc = -1)
		{
			FOwner.TV_draw_rect( X + x0, Y + y0, w, h, c, fc );
		}

		virtual void TV_set_pixel(uint8_t x, uint8_t y, char c)
		{
			FOwner.TV_set_pixel( X + x , Y + y, c );
		}

		virtual unsigned char TV_get_pixel(uint8_t x, uint8_t y)
		{
			return FOwner.TV_get_pixel( X + x , Y + y );
		}

	public:
		virtual void Render() override
		{
			for( int i = 0; i < FElements.size(); i ++ )
				FElements[ i ]->Render();
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class TVOutRCAElementCheckPixel : public Mitov::TVOutRCAElementBasicPosition
	{
		typedef Mitov::TVOutRCAElementBasicPosition inherited;

    public:
		OpenWire::SourcePin	OutputPin;

	public:
		virtual void Render() override
		{
			if( FOwner.IsEnabled() )
				OutputPin.SendValue( FOwner.TV_get_pixel( X, Y ) > 0 );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class TVOutRCAElementDrawBitmap : public Mitov::TVOutRCAElementBasicPosition
	{
		typedef Mitov::TVOutRCAElementBasicPosition inherited;

    public:
		uint8_t	Width;
		uint8_t	Height;

		const unsigned char *_Bytes;

	public:
		virtual void Render() override
		{
			FOwner.TV_bitmap( X, Y, _Bytes, 0, Width, Height );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class TVOutRCAElementSetCursor : public Mitov::TVOutRCAElementBasicPosition
	{
		typedef Mitov::TVOutRCAElementBasicPosition inherited;

	public:
		virtual void Render() override
		{
			FOwner.TV_set_cursor( X, Y );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class TVOutRCAElementPrintText : public Mitov::TVOutRCAElementBasic
	{
		typedef Mitov::TVOutRCAElementBasic inherited;

	public:
		bool NewLine = true;

	protected:
		const unsigned char *FFont;

	public:
		template<typename T> void Print( T AValue )
		{
			if( FOwner.IsEnabled() )
			{
				FOwner.GetTV().select_font( FFont );
				if( NewLine )
					FOwner.GetTV().println( AValue );

				else
					FOwner.GetTV().print( AValue );
			}
		}

	public:
		TVOutRCAElementPrintText( Mitov::TVOut_RCA_Intf &AOwner, const unsigned char * AFont ) :
			inherited( AOwner ),
			FFont( AFont )
		{
		}

	};
//---------------------------------------------------------------------------
	class TVOutRCAElementTextAt : public Mitov::TVOutRCAElementClocked
	{
		typedef Mitov::TVOutRCAElementClocked inherited;

	public:
		int32_t X = 0;
		int32_t Y = 0;
		String	InitialValue;

	protected:
		const unsigned char *FFont;
		String	FValue;

	public:
		template<typename T> void Print( T AValue )
		{
			 FValue = String( AValue );
		}

	public:
		virtual void Render() override
		{
			if( FOwner.IsEnabled() )
			{
				FOwner.GetTV().select_font( FFont );
				FOwner.TV_set_cursor( X, Y );
				FOwner.GetTV().print( FValue.c_str() );
			}
		}

	protected:
		virtual void SystemInit()
		{
			FValue = InitialValue;
			inherited::SystemInit();
		}

	public:
		TVOutRCAElementTextAt( Mitov::TVOut_RCA_Intf &AOwner, const unsigned char * AFont ) :
			inherited( AOwner ),
			FFont( AFont )
		{
		}

	};
//---------------------------------------------------------------------------
	TVOutRCAElementBasic::TVOutRCAElementBasic( Mitov::TVOut_RCA_Intf &AOwner ) :
		FOwner( AOwner )
	{
		FOwner.RegisterRender( this );
	}
//---------------------------------------------------------------------------
}

#endif
