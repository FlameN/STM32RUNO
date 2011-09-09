#ifndef _DBG
#define _DBG
extern void dbgmessage(char*);
extern void dbgPrintArray(int,int,unsigned char);
extern void trace( const char *format, ...);
extern void strace(const char *, const char *format, ...);
extern void dbgInit(void);
extern void dbgStackFreeSize(char*);
extern void dbgHeapFreeSize(void);
#endif
