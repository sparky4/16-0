///////////////////////////////////////
//
// TED5 Map Header for HP1
//
///////////////////////////////////////

//
// Map Names
//
typedef enum {
		A_MAP,                   // 0
		LASTMAP
	     } mapnames;

//
// TILEINFO offsets
//
#define 1		402
#define 2		(1+NUMTILE16)
#define 3		(2+NUMTILE16)
#define 4		(3+NUMTILE16)

//
// TILEINFOM offsets
//
#define 1		(4+NUMTILE16)
#define 2		(1+NUMTILE16M)
#define 3		(2+NUMTILE16M)
#define 4		(3+NUMTILE16M)
