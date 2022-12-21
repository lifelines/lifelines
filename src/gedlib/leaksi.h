#ifndef leaksi_h_included
#define leaksi_h_included

#define TRACK_RECORD_OPS 0
#define TRACK_NODE_OPS   0

#define TRACK_BACKTRACE  0

extern FILE* fpleaks;

#define TRACK_OP_ALLOC      1
#define TRACK_OP_FREE       2
#define TRACK_OP_REFCNT_INC 3
#define TRACK_OP_REFCNT_DEC 4

void track_record(RECORD rec, int op, char *msg, char* file, int line);
void track_record_refcnt(RECORD rec, int op, INT refcnt, char* file, int line);
void track_node(NODE node, int op, char *msg, char* file, int line);

#if TRACK_RECORD_OPS
#define TRACK_RECORD(rec,op,msg,file,line)    if (fpleaks) { track_record(rec,op,msg,file,line); }
#define TRACK_RECORD_REFCNT(rec,op,file,line) if (fpleaks) { track_record_refcnt(rec,op,rec->refcnt,file,line); }
#else
#define TRACK_RECORD(rec,op,msg,file,line)
#define TRACK_RECORD_REFCNT(rec,op,msg,file,line)
#endif

#if TRACK_NODE_OPS
#define TRACK_NODE(n,op,msg,file,line) if (fpleaks) { track_node(n,op,msg,file,line); }
#else
#define TRACK_NODE(n,op,msg,file,line)
#endif

#endif /* leaksi_h_included */

