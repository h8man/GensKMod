#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "parser.h"



struct str_parser_info
{
	char *text;
	DWORD numlines;
	DWORD *linebuffer;
	DWORD length;
}parser_info;


BOOL Parser_Load( HANDLE hFile )
{
	DWORD dwByteRead;
	DWORD i         = 0;
    DWORD linestart = 0;

	parser_info.length = GetFileSize(hFile, NULL);
	parser_info.text = (char *)LocalAlloc(LPTR, parser_info.length);
	if (parser_info.text == NULL)
	{
		wsprintf(parser_error, "Can't allocate %d bytes to read file", parser_info.length);
		return FALSE;
	}

	ReadFile(hFile, parser_info.text, parser_info.length, &dwByteRead, NULL);
	

	
	parser_info.linebuffer = (DWORD *)LocalAlloc(LPTR, parser_info.length);
	if (parser_info.linebuffer == NULL)
	{
		LocalFree(parser_info.text);
		wsprintf(parser_error, "Can't allocate %d bytes for line buffer", parser_info.length);
		return FALSE;
	}

	parser_info.numlines = 0;	
    // loop through every byte in the file
    for(i = 0; i < parser_info.length; )
    {
        if(parser_info.text[i++] == '\r')
        {
			parser_info.text[i-1] = '\0';
            // carriage-return / line-feed combination
            if(parser_info.text[i] == '\n')
			{
				parser_info.text[i] = '\0';
                i++;
			}

            // record where the line starts
            parser_info.linebuffer[parser_info.numlines++] = linestart;
            linestart = i;
        }
    }

	//save last line if no \r\n
    parser_info.linebuffer[parser_info.numlines] = linestart;


	return TRUE;
}


char *Parser_Line (DWORD idx )
{
	if (idx>parser_info.numlines)	return(NULL);

	return (parser_info.text + parser_info.linebuffer[idx]); 
}


DWORD Parser_NumLine( )
{
	return(parser_info.numlines);
}

void Parser_Unload( )
{
	LocalFree(parser_info.linebuffer);
	LocalFree(parser_info.text);
}

DWORD Parser_Words_Count( char *string)
{
	DWORD limit = strlen(string);
	DWORD i, words = 0;
	BOOL  change = TRUE;
		
	for(i = 0; i < limit; ++i)
	{ 
		if(!isspace(string[i]))
		{
			if(change)
			{
				++words;
				change = FALSE;
			}
		}
		else 
		{
			change = TRUE;
		}
	} 

	return(words);
}


//// FROM http://www.cppfrance.com/code.aspx?ID=22234

char* copy( char* dst, const char* src ) {
    char* cur = dst;
    while( *cur++ = *src++ )
        ;
    return dst;
}

char* rtrim( char* str, const char* t )
{
    char* curEnd = str, *end = str;

    char look[ 256 ] = {  1, 0 };
    while( *t )
       look[ (unsigned char)*t++ ] = 1;

    while( *end ) {
      if ( !look[ *end ] )
          curEnd = end + 1;
      ++end;
    }
    *curEnd = '\0';

    return str;
}

char* ltrim( char* str, const char* t ) {
    char* curStr = NULL;

    char look[ 256 ] = { 1, 0 };
    while( *t )
        look[ (unsigned char)*t++ ] = 1;

    curStr = str;
    while( *curStr && look[ *curStr ] )
        ++curStr;

    return copy( str, curStr );
}

char* trim( char* str, const char* t ) {
    return ltrim( rtrim( str, t ), t );
}