/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include <ctype.h>
#include "utils/stl_utils.h"
#include "utils/stringext.h"
#include "Util/UTF8.hpp"
#include "Poco/Latin1Encoding.h"
#include "Poco/UTF8Encoding.h"
#include "Poco/TextConverter.h"
#include "Util/Clamp.hpp"
#include "Util/TruncateString.hpp"

void PExtractParameter(TCHAR *Source, TCHAR *Destination, size_t dest_size, int DesiredFieldNumber)
{
  size_t index = 0;
  size_t dest_index = 0;
  int CurrentFieldNumber = 0;

  size_t StringLength = _tcslen(Source);

  while( (CurrentFieldNumber < DesiredFieldNumber) && (index < StringLength) ) {
    if ( Source[ index ] == ',' ) {
      CurrentFieldNumber++;
	}
    index++;
  }

  if ( CurrentFieldNumber == DesiredFieldNumber ) {
    while( (dest_index < dest_size) && (index < StringLength) &&
            (Source[ index ] != ',') && (Source[ index ] != 0x00) )
	{
	  Destination[dest_index] = Source[ index ];
	  index++; dest_index++;
	}
    assert(dest_index < dest_size);
    Destination[std::min(dest_index, dest_size-1)] = '\0';
  }
}

int HexStrToInt(TCHAR *Source){

	static const size_t digit_table_symbol_count = 256;
	static const unsigned char digit_table[digit_table_symbol_count] = {
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x00 - 0x07
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x08 - 0x0F
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x10 - 0x17
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x18 - 0x1F
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x20 - 0x27
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x28 - 0x2F
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // 0x30 - 0x37
		0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x38 - 0x3F
		0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, // 0x40 - 0x47
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x48 - 0x4F
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x50 - 0x57
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x58 - 0x5F
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x60 - 0x67
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x68 - 0x6F
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x70 - 0x77
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x78 - 0x7F
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x80 - 0x87
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x88 - 0x8F
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0A, // 0x90 - 0x97
		0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, // 0x98 - 0x9F
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xA0 - 0xA7
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xA8 - 0xAF
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xB0 - 0xB7
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xB8 - 0xBF
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xC0 - 0xC7
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xC8 - 0xCF
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xD0 - 0xD7
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xD8 - 0xDF
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xE0 - 0xE7
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xE8 - 0xEF
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0xF0 - 0xF7
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  // 0xF8 - 0xFF
	};

	int nOut=0;
	while(1){
		const unsigned int digit = static_cast<unsigned int>(digit_table[static_cast<unsigned char>(*Source++)]);
		if (0xFF == digit)
			break;
		nOut = ((nOut << 4)&0xFFFFFFF0)|digit;
	}
	return nOut;
}


/*
 * WARNING: if you are using Stop pointer, remember that you MUST set it equal to *Source before
   calling this function, because in case of null result, it will not be set!!
 */
double StrToDouble(TCHAR *Source, TCHAR **Stop)
{
  int index = 0;
  int StringLength        = 0;
  double Sum = 0;
  double Divisor = 10;
  int neg = 0;

  if (Source==NULL) return 0.0;
  StringLength = _tcslen(Source);

  while(((Source[index] == ' ')||(Source[index]=='+')||(Source[index]==9))
        && (index<StringLength))
    // JMW added skip for tab stop
    // JMW added skip for "+"
    {
      index ++;
    }
  if (index>= StringLength) {
	// WARNING> we are not setting Stop here!!
    return 0.0; // Set 0.0 as error, probably not the best thing to do. TOFIX 110307
  }
  if (Source[index]=='-') {
    neg=1;
    index++;
  }

  while( (index < StringLength)
	 &&
	 (
	  (Source[index]>= '0') && (Source [index] <= '9')
          )
	 )
    {
      Sum = (Sum*10) + (Source[ index ] - '0');
      index ++;
    }

  if (index >= StringLength) goto _strtodouble_return;
  if(Source[index] == '.')
    {
      index ++;
      while( (index < StringLength)
	     &&
	     (
	      (Source[index]>= '0') && (Source [index] <= '9')
	      )
	     )
	{
	  Sum = (Sum) + (double)(Source[ index ] - '0')/Divisor;
	  index ++;Divisor = Divisor * 10;
	}
    }

_strtodouble_return:
  if(Stop != NULL) {
    if (index < StringLength) {
        *Stop = &Source[index];
    } else {
        *Stop = &Source[StringLength];
    }
  }


  if (neg) {
    return -Sum;
  } else {
    return Sum;
  }
}


// RMN: Volkslogger outputs data in hex-strings.  Function copied from StrToDouble
// Note: Decimal-point and decimals disregarded.  Assuming integer amounts only.
double HexStrToDouble(TCHAR *Source, TCHAR **Stop)
{
  int index = 0;
  int StringLength        = 0;
  double Sum = 0;
  int neg = 0;

  if (Source==NULL) return 0.0;
  StringLength = _tcslen(Source);

  while((index < StringLength) && ((Source[index] == ' ')||(Source[index]==9)))
    // JMW added skip for tab stop
    {
      index ++;
    }
  if (index >= StringLength) {
      goto _hexstrtodouble_return;
  }
  if (Source[index]=='-') {
    neg=1;
    index++;
  }

  while(
  (index < StringLength)	 &&
	(	( (Source[index]>= '0') && (Source [index] <= '9')  ) ||
		( (Source[index]>= 'A') && (Source [index] <= 'F')  ) ||
		( (Source[index]>= 'a') && (Source [index] <= 'f')  )
		)
	)
    {
      if((Source[index]>= '0') && (Source [index] <= '9'))	  {
		Sum = (Sum*16) + (Source[ index ] - '0');
		index ++;
	  }
	  if (index >= StringLength) goto _hexstrtodouble_return;
	  if((Source[index]>= 'A') && (Source [index] <= 'F'))	  {
		Sum = (Sum*16) + (Source[ index ] - 'A' + 10);
		index ++;
	  }
	  if (index >= StringLength) goto _hexstrtodouble_return;
	  if((Source[index]>= 'a') && (Source [index] <= 'f'))	  {
		Sum = (Sum*16) + (Source[ index ] - 'a' + 10);
		index ++;
	  }
    }

_hexstrtodouble_return:
  if(Stop != NULL) {
      if (index < StringLength) {
          *Stop = &Source[index];
      } else {
          *Stop = &Source[StringLength];
      }
  }

  if (neg) {
    return -Sum;
  } else {
    return Sum;
  }
}




TCHAR *_tcstok_r(TCHAR *s, const TCHAR *delim, TCHAR **lasts){
// "s" MUST be a pointer to an array, not to a string!!!
// (ARM92, Win emulator cause access violation if not)

  const TCHAR *spanp;
	int   c, sc;
	TCHAR *tok;


	if (s == NULL && (s = *lasts) == NULL)
		return (NULL);

	/*
	 * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
	 */

cont:
	c = *s++;
	for (spanp = delim; (sc = *spanp++) != 0;) {
		if (c == sc)
			goto cont;
	}

	if (c == 0) {		/* no non-delimiter characters */
		*lasts = NULL;
		return (NULL);
	}
	tok = s - 1;

	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;  // causes access violation in some configs if s is a pointer instead of an array
				*lasts = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}

// Same As strtok_r but not skeep leading delimiter ...
TCHAR *strsep_r(TCHAR *s, const TCHAR *delim, TCHAR **lasts){
// "s" MUST be a pointer to an array, not to a string!!!
// (ARM92, Win emulator cause access violation if not)

	const TCHAR *spanp;
	int   c, sc;
	TCHAR *tok = NULL;


	if (s == NULL && (s = *lasts) == NULL)
		return (NULL);

	// return empty string if s start with delimiter
	c = *s++;
	for (spanp = delim; (sc = *spanp++) != 0;) {
		if (c == sc) {
			*lasts = s;
			s[-1] = 0;
			return (s - 1);
		}
	}

	if (c == 0) {		/* no non-delimiter characters */
		*lasts = NULL;
		return (NULL);
	}
	tok = s - 1;

	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;  // causes access violation in some configs if s is a pointer instead of an array
				*lasts = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}

// Trim trailing space
void TrimRight(TCHAR* str) {
    TCHAR * end = str + _tcslen(str);
    while(end > str && _istspace(*(end-1))) end--;
    // Write new null terminator
    *(end) = 0;
}

TCHAR* StringMallocParse(const TCHAR* old_string) {
  TCHAR buffer[2048];	// Note - max size of any string we cope with here !
  TCHAR* new_string;
  unsigned int used = 0;
  unsigned int i;

  // Transcode special characters
  for (i = 0; i < _tcslen(old_string); i++) {
    if (used < 2045) {
      if (old_string[i] == '\\' ) {
        if (old_string[i + 1] == 'r') {
          buffer[used++] = '\r';
          i++;
        } else if (old_string[i + 1] == 'n') {
          buffer[used++] = '\n';
          i++;
        } else if (old_string[i + 1] == '\\') {
          buffer[used++] = '\\';
          i++;
        } else {
          buffer[used++] = old_string[i];
        }
      } else {
        buffer[used++] = old_string[i];
      }
    }
  };
  // right trim string
  while(used > 0 && buffer[used-1] == _T(' ')) {
    --used;
  }
  buffer[used++] =_T('\0');

  TCHAR *pstart = _tcsstr(buffer, _T("_@M"));
  if (pstart==NULL) {
	goto _notoken;
  }

  // find end of token
  TCHAR *pnext;
  pnext = std::find(pstart+3, &buffer[used], _T('_'));
  if(pnext == &buffer[used]) {
    StartupStore(_T("...... Menu Label incorrect, invalid token: <%s>" NEWLINE),buffer);
    goto _notoken; // invalid token
  }
  if(std::distance(pstart, pnext) > 9) {
	StartupStore(_T("...... Menu Label incorrect, token too big: <%s>" NEWLINE),buffer);
	goto _notoken;
  }
  ++pnext;

  TCHAR lktoken[10];
  *std::copy(pstart, pnext, lktoken) = _T('\0');
  *pstart = _T('\0');

  const TCHAR *tokentext;
  tokentext = LKGetText(lktoken);
  size_t new_len;
  new_len = _tcslen(buffer) + _tcslen(tokentext) + _tcslen(pnext);
  new_string = (TCHAR *)malloc((new_len+1)*sizeof(TCHAR));
  if (new_string==NULL) {
	OutOfMemory(_T(__FILE__),__LINE__);
	return NULL;
  }
  _stprintf(new_string,_T("%s%s%s"), buffer, tokentext, pnext);
  return new_string;

_notoken:
  new_string = (TCHAR *)malloc((_tcslen(buffer)+1)*sizeof(TCHAR));
  if (new_string==NULL) {
	OutOfMemory(_T(__FILE__),__LINE__);
	return NULL;
  }
  _tcscpy(new_string, buffer);
  return new_string;
}

/*
 * Implementation of the _splitpath runtime library function with wide character strings
 * Copyright 2000, 2004 Martin Fuchs -- GPL licensed - WINE project
 */
void LK_tsplitpath(const TCHAR* path, TCHAR* drv, TCHAR* dir, TCHAR* name, TCHAR* ext)
{
	const TCHAR* end; /* end of processed string */
	const TCHAR* p;	  /* search pointer */
	const TCHAR* s;	  /* copy pointer */

	/* extract drive name */
	if (path[0] && path[1]==':') {
		if (drv) {
			*drv++ = *path++;
			*drv++ = *path++;
			*drv = '\0';
		} else path+=2;
	} else if (drv)
		*drv = '\0';

	/* search for end of string or stream separator */
	for(end=path; *end && *end!=':'; )
		end++;

	/* search for begin of file extension */
	for(p=end; p>path && *--p!='\\' && *p!='/'; )
		if (*p == '.') {
			end = p;
			break;
		}

	if (ext)
		for(s=end; (*ext=*s++); )
			ext++;

	/* search for end of directory name */
	for(p=end; p>path; )
		if (*--p=='\\' || *p=='/') {
			p++;
			break;
		}

	if (name) {
		for(s=p; s<end; )
			*name++ = *s++;

		*name = '\0';
	}

	if (dir) {
		for(s=path; s<p; )
			*dir++ = *s++;

		*dir = '\0';
	}
}

//
// v5 NOTE about LK_tcsncpy
// Remember: the destination must ALWAYS be sized numchars+1 at least!!
// We set a safe 0 in dest[numchars] position. Careful!
//

#if USELKASSERT
void LK_tcsncpy_internal(TCHAR *dest, const TCHAR *src, const unsigned int numofchars, const unsigned int sizedest, const int line, const TCHAR *filename)
#else
void LK_tcsncpy_internal(TCHAR *dest, const TCHAR *src, const unsigned int numofchars)
#endif
{

  #if USELKASSERT
  if (dest == NULL || sizedest==0) {
	StartupStore(_T("[ASSERT FAILURE (LK_tcsncpy dest)] in %s line %d, sizedest=%d") NEWLINE, filename, line,sizedest);
	return;
	//LKASSERT(false); // does not work during startup!
  }
  // We need space for 0 termination, otherwise we are in trobles
  // So numofchars must be < than the destination string.
  // Notice> we cannot check sizeof of a pointer, so anything with an address size instead of an array size
  // will be excluded. Better than nothing.
  if (  sizedest>sizeof(dest) && numofchars >= sizedest ) {
	StartupStore(_T("[ASSERT FAILURE (LK_tcsncpy dest)] in %s line %d dstsize=%d srcsize=%d") NEWLINE, filename, line,sizedest,numofchars);
	_tcsncpy(dest,src,sizedest-1);
	dest[sizedest-1] = '\0';
	return;
	//LKASSERT(false);
  }
  #endif

  // if source is null we safely ignore it, and give an empty string as result
  if (src == NULL || src[0]=='\0') {
	dest[0] = '\0';
	return;
  }

  _tcsncpy(dest,src,numofchars);
  dest[numofchars]='\0';

}





const TCHAR *WindAngleToText(double angle) {

 // Valid index values: 0 - 16,  17 is for Err
 static const TCHAR *const windrose[17]= {TEXT("N"),TEXT("NNE"),TEXT("NE"),TEXT("ENE"),TEXT("E"),TEXT("ESE"),
			TEXT("SE"),TEXT("SSE"),TEXT("S"),TEXT("SSW"),TEXT("SW"),TEXT("WSW"),
			TEXT("W"),TEXT("WNW"),TEXT("NW"),TEXT("NNW"),TEXT("---")};

 // We need 32 slots of 11.25 degrees for a full 360
 static unsigned short angleslot[32]={0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13,14,14,15,15,0};

 if (angle<0) return(windrose[16]); // ---
 if (angle>=360) angle-=360;

 unsigned short direction= (unsigned short)(angle/11.25);
 LKASSERT(direction<32);
 return (windrose[angleslot[direction]]);

}

///////////////////////////////////////////////////////////////////////
// Extract H, M, S from string like "HH:MM:SS"
//   Sec output parameter is optional
void StrToTime(LPCTSTR szString, int *Hour, int *Min, int *Sec) {
    LKASSERT(Hour && Min);
    TCHAR* sz = NULL;
    if (szString) {
        *Hour = Clamp<int>(_tcstol(szString, &sz, 10), 0, 23);
        if (*sz == _T(':')) {
            *Min = Clamp<int>(_tcstol(sz + 1, &sz, 10), 0, 59);

            if (Sec && (*sz == _T(':'))) {
                *Sec = Clamp<int>(_tcstol(sz + 1, &sz, 10), 0, 59);
            }
        }
    }
}
