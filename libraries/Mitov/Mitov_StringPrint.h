////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_STRING_PRINT_h
#define _MITOV_STRING_PRINT_h

#include <Mitov.h>

namespace Mitov
{
	class StringPrint : public Print
	{
		typedef Print inherited;

	public:
		String Value;

	public:
	    virtual size_t write( uint8_t AChar )
		{
			Value += (char)AChar;
			return 1;
		}
	};
//---------------------------------------------------------------------------
	class BufferPrint : public Print
	{
		typedef Print inherited;

	public:
		Mitov::SimpleList<uint8_t> Value;

	public:
	    virtual size_t write( uint8_t AChar )
		{
			Value.push_back( AChar );
			return 1;
		}
	};
}

#endif
