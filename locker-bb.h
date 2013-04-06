///

unsigned long bitbanded_address(unsigned long addr,int bit);

int get_bitbanded_lock(unsigned long lockaddr, int bit);
void release_bitbanded_lock(unsigned long lockaddr, int bit);

