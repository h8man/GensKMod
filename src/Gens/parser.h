#ifndef __PARSER_H__
#define __PARSER_H__


UCHAR parser_error[256];


#ifdef __cplusplus
extern "C" {
#endif

BOOL Parser_Load( HANDLE hFile );
void Parser_Unload( );

char *Parser_Line (DWORD idx );
DWORD Parser_NumLine( );

DWORD Parser_Words_Count( char *string);
char* trim( char* str, const char* t );

#ifdef __cplusplus
}
#endif

#endif