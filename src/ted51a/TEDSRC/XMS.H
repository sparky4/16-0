void InitXMS(void);
int XMSAllocate(long size);
unsigned long XMSTotalFree(void);	// returns KB free
void XMSFreeMem(int handle);
void XMSmove(int srchandle,long srcoff,int desthandle,long destoff,long size);

extern unsigned long XMSavail;
extern long TTLxms;
