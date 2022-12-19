#ifndef leaksi_h_included
#define leaksi_h_included

#define TRACE_RECORD_OPS 0
#define TRACE_NODE_OPS   0

extern FILE* fpleaks;

#define TRACE_OP_ALLOC      1
#define TRACE_OP_FREE       2
#define TRACE_OP_REFCNT_INC 3
#define TRACE_OP_REFCNT_DEC 4

void track_record(RECORD rec, int op, char *msg, char* file, int line);
void track_record_refcnt(RECORD rec, int op, INT refcnt, char* file, int line);
void track_node(NODE node, int op, char *msg, char* file, int line);

#if defined TRACE_RECORD_OPS
#define TRACE_RECORD(rec,op,msg,file,line)    if (fpleaks) { track_record(rec,op,msg,file,line); }
#define TRACE_RECORD_REFCNT(rec,op,file,line) if (fpleaks) { track_record_refcnt(rec,op,rec->refcnt,file,line); }
#else
#define TRACE_RECORD(rec,op,msg,file,line)
#define TRACE_RECORD_REFCNT(rec,op,msg,file,line)
#endif

#if defined TRACE_NODE_OPS
#define TRACE_NODE(n,op,msg,file,line) if (fpleaks) { track_node(n,op,msg,file,line); }
#else
#define TRACE_NODE(n,op,msg,file,line)
#endif

#endif /* leaksi_h_included */

