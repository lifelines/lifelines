INDEX crtindex(BTREE);
INDEX getindex(BTREE, FKEY);
void initcache(BTREE, INT);
void putheader(BTREE, BLOCK);
void putindex(BTREE, INDEX);
INDEX readindex(STRING, FKEY);
void writeindex(STRING, INDEX);
