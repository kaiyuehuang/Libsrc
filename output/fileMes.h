#ifndef _FILEMES_H
#define _FILEMES_H


/**************************************************
»ñÈ¡ÎÄ¼þ³¤¶È
***************************************************/
extern int GetFileLen(FILE *fp);
  
extern char *STRSTR(const char *str, char needle);

extern int ScanDir(char *basePath,void callfilename(char *filepath,char *name));

#endif
