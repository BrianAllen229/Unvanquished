/*
===========================================================================

Unvanquished GPL Source Code
Copyright (C) 2012 Unvanquished Developers

This file is part of the Unvanquished GPL Source Code (Unvanquished Source Code).

Unvanquished Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Unvanquished Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Unvanquished Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Unvanquished Source Code is also subject to certain additional terms.
You should have received a copy of these additional terms immediately following the
terms and conditions of the GNU General Public License which accompanied the Unvanquished
Source Code.  If not, please request a copy in writing from id Software at the address
below.

If you have questions concerning this license or the applicable additional terms, you
may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville,
Maryland 20850 USA.

===========================================================================
*/

#ifndef ROCKETCONDITIONALELEMENT_H
#define ROCKETCONDITIONALELEMENT_H

#include <RmlUi/Core/Core.h>
#include "../cg_local.h"

class RocketConditionalElement : public Rocket::Core::Element
{
public:
	RocketConditionalElement( const Rocket::Core::String &tag ) : Rocket::Core::Element( tag ), condition( NOT_EQUAL ), dirty_value( false ) {}

	virtual void OnAttributeChange( const Rocket::Core::ElementAttributes &changed_attributes )
	{
		Rocket::Core::Element::OnAttributeChange( changed_attributes );
		Rocket::Core::ElementAttributes::const_iterator it;
		it = changed_attributes.find( "cvar" );
		if ( it != changed_attributes.end() )
		{
			cvar = it->second.Get<Rocket::Core::String>();
			cvar_value = Cvar::GetValue( cvar.CString() ).c_str();
		}

		it = changed_attributes.find( "condition" );
		if ( it != changed_attributes.end() )
		{
			ParseCondition( it->second.Get<Rocket::Core::String>() );
		}

		it = changed_attributes.find( "value" );
		if ( it !=  changed_attributes.end() )
		{
			char *end = nullptr;
			// Check if float
			float floatVal = strtof( it->second.Get<Rocket::Core::String>().CString(), &end );

			// Is either an integer or float
			if ( end )
			{
				// is integer
				if ( static_cast< int >( floatVal ) == floatVal )
				{
					value = static_cast< int >( floatVal );
				}
				else
				{
					value = floatVal;
				}
			}

			// Is a string
			else
			{
				value = it->second;
			}

			dirty_value = true;
		}
	}

	virtual void OnUpdate()
	{
		if ( dirty_value || ( !cvar.Empty() && cvar_value != Cvar::GetValue( cvar.CString() ).c_str() ) )
		{
			if ( IsConditionValid() )
			{
				if ( !IsVisible() )
				{
					SetProperty( "display", "block" );
				}
			}
			else
			{
				if ( IsVisible() )
				{
					SetProperty( "display", "none" );
				}
			}

			cvar_value = Cvar::GetValue( cvar.CString() ).c_str();

			if ( dirty_value )
			{
				dirty_value = false;
			}
		}
	}

private:
	enum Condition
	{
		EQUALS,
		LESS,
		GREATER,
		LESS_EQUAL,
		GREATER_EQUAL,
		NOT_EQUAL
	};

	void ParseCondition( const Rocket::Core::String& str )
	{
		if ( str == "==" )
		{
			condition = EQUALS;
		}
		else if ( str == "<" )
		{
			condition = LESS;
		}
		else if ( str == ">" )
		{
			condition = GREATER;
		}
		else if ( str == "<=" )
		{
			condition = LESS_EQUAL;
		}
		else if ( str == ">=" )
		{
			condition = GREATER_EQUAL;
		}
		else
		{
			condition = NOT_EQUAL;
		}
	}

#define Compare( one, two ) switch ( condition ) { \
			case EQUALS: return one == two; \
			case LESS: return one < two; \
			case GREATER: return one > two; \
			case LESS_EQUAL: return one <= two; \
			case GREATER_EQUAL: return one >= two; \
			case NOT_EQUAL: return one != two; \
			default: return false; }

	bool IsConditionValid()
	{
		std::string str = Cvar::GetValue( cvar.CString() );
		switch ( value.GetType() )
		{
			case Rocket::Core::Variant::INT:
				Compare( ParseInt( str ), value.Get<int>() );
			case Rocket::Core::Variant::FLOAT:
			{
				float flt = 0;
				Cvar::ParseCvarValue( str, flt );
				Compare( flt, value.Get<float>() );
			}
			default:
				Compare( str, value.Get< Rocket::Core::String >().CString() );
		}
	}

	// Also takes bools
	int ParseInt(const std::string& str)
	{
		int integer;
		if ( Cvar::ParseCvarValue( str, integer ) )
		{
			return integer;
		}
		bool boolean;
		if ( Cvar::ParseCvarValue( str, boolean ) )
		{
			return +boolean;
		}
		return 0;
	}

	Rocket::Core::String cvar;
	Rocket::Core::String cvar_value;
	Condition condition;
	Rocket::Core::Variant value;
	bool dirty_value;
};
#endif
