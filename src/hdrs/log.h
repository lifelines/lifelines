#ifndef log_h_included
#define log_h_included

void log_outf(const char * filepath, const char * fmt, ...) HINT_PRINTF(2,3);
void log_bytecode(const char * filepath, const char * intro, const char * bytes);

#endif /* log_h_included */
