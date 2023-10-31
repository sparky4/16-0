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
#define A		402
#define B		(A+NUMTILE16)
#define C		(B+NUMTILE16)
#define D		(C+NUMTILE16)

//
// TILEINFOM offsets
//
#define A		(D+NUMTILE16)
#define B		(A+NUMTILE16M)
#define C		(B+NUMTILE16M)
#define D		(C+NUMTILE16M)
