#ifndef KEYCONF_H
#define KEYCONF_H

enum {
    kc_up, kc_down, kc_left, kc_right,
    kc_circle, kc_cross, kc_triangle, kc_square,
    kc_mm_triangle, kc_mm_square, kc_image, kc_comment
};

int kc_get_ks_num(void);
int kc_is_ready(void);
char *kc_get_keystrs(int ksnum, int tokenid);
void *kc_get_keycodes(int ksnum, int tokenid);
char *kc_get_comment(int ksnum);
void kc_loadfile(void);

#endif
