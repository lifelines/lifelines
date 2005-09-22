#ifndef log_h_included
#define log_h_included

void log_outf(const char * filepath, const char * fmt, ...);
void log_bytecode(const char * filepath, const char * intro, const char * bytes);

#endif /* log_h_included */
