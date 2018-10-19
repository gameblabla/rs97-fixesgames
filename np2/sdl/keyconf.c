//#include "psp/psplib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "keyconf.h"
#include "keystat.h"
#include "keystat.tbl"

#define PSP_KEY_TXT "psp_key.txt"

#define KC_MAXLINE 256

#define KC_MAXTOKEN 12 // psp_key.txtの～= の～の部分の数
#define KC_MAXKEYDEF 15 // 一つのボタンに割り当てられる設定数(同時押しになる)
#define KC_MAXTOKENLEN 12 // "mm_triangle"が今のところ最長文字列

static char kc_token[KC_MAXTOKEN][KC_MAXTOKENLEN] = {
    "up", "down", "left", "right",
    "circle", "cross", "triangle", "square",
    "mm_triangle", "mm_square", "image", "comment"
};

typedef struct _fname_list {
    struct _fname_list *next;
    char fname[0]; // gcc only
} fname_list_t;

typedef struct _keyset {
    struct _keyset *next;
    char comment[KC_MAXLINE];
    UINT8 key[KC_MAXTOKEN][KC_MAXKEYDEF + 1]; // +1は終了コード(0xff)分
    fname_list_t *fname_head;
} keyset_t;

static int keyset_num = 0;
static keyset_t *keyset_head = NULL;
static int lineno = 0;

/* 文末の改行コードを削除 (Unix: LF, DOS: LF CR, Mac: CR)  */
static void chomp(char *s)
{
#define ASCII_CR 0x0d
#define ASCII_LF 0x0a

    char *p;

    p = &s[strlen(s) - 1];

    if (*p == ASCII_LF) { // Unix
        *p = '\0';
        if (strlen(s) >= 1 && *(p - 1) == ASCII_CR) { // DOS
            *(p - 1) = '\0';
        }
    } else if (*p == ASCII_CR) { // Mac
        *p = '\0';
    }
}

/* 新しいkeyset構造体を確保し、queueにつなぐ */
/* malloc()に失敗したらNULLを、成功なら確保したポインタを返す */
static keyset_t *add_keyset(void)
{
    keyset_t *p, *new;

    new = malloc(sizeof(keyset_t));
    if (new == NULL) {
        return NULL;
    }
    memset(new, 0, sizeof(keyset_t));
    memset(new->key, 0xff, sizeof(new->key));
    new->next = NULL;

    if (keyset_head == NULL) {
        keyset_head = new;
        return new;
    }

    for (p = keyset_head; p->next != NULL; p = p->next)
        ;
    p->next = new;

    return new;
}

static fname_list_t *add_fname(fname_list_t **head, char *s)
{
    fname_list_t *l, *new;
    int size;

    for (l = *head; l != NULL; l = l->next) {
        if (l->next == NULL) {
            break;
        }
    }

    size = sizeof(fname_list_t) + strlen(s) + 1;
    new = malloc(size);
    if (new == NULL) {
        return NULL;
    }

    memset(new, 0, size);
    strcpy(new->fname, s);
    if (l == NULL) {
        *head = new;
    } else {
        l->next = new;
    }

    return new;
}


/* tokenを切り出す。読み出し元のポインタも動かす。
   tokenは文頭、文末、またはtokenの前後に空白または=があること
   tokenが取得できなければ-1を返す */
static int get_token(char **s, char *d)
{
    /* 空白を飛ばす */
    while (**s == ' ' || **s == '\t') {
        (*s)++;
    }
    if (**s == '\0') {
        return -1;
    }

    if (**s == '=') {
        *d++ = *(*s)++;
        *d = '\0';
        return 0;
    }

    while (**s != ' ' && **s != '\t' && **s != '\0' && **s != '=') {
        *d++ = *(*s)++;
    }
    // [=]の対処
    if (*(d - 1) == '[' && **s == '=' && *(*s + 1) == ']') {
        *d++ = *(*s)++, *d++ = *(*s)++;
    }
    *d = '\0';

    return 0;
}

/* ""で囲まれた文字列を""をはずして取得する。*/
static int get_quoted_str(char **s, char *d)
{
    /* 空白を飛ばす */
    while (**s == ' ' || **s == '\t') {
        (*s)++;
    }
    if (**s == '\0' || **s != '"') {
        return -1;
    }
    /* 最初の"をとばす */
    (*s)++;

    while (**s != '\0' && **s != '"') {
        *d++ = *(*s)++;
    }
    if (**s != '"') {
        // " expected
        return -1;
    }
    *d = '\0';

    return 0;
}



/* nが0xffなら文字列検索, sがNULLなら数値検索 */
static KEYNAME *_search_kn(UINT8 n, char *s)
{
    KEYNAME *kn, *end;

    if (n == 0xff && s == NULL) {
        return NULL;
    }

    end = (KEYNAME *)keyname + sizeof(keyname)/sizeof(KEYNAME);


    for (kn = (KEYNAME *)keyname; kn != end; kn++) {
        if (n == 0xff && strcmp(kn->str, s) == 0) {
            return kn;
        }
        if (s == NULL && kn->num == n) {
            return kn;
        }
    }

    return NULL;
}

/* SHIFT -> 0x70のようにキー文字列からキーコードを返す。見つからなければ0xff */
static UINT8 search_keynum(char *s)
{
    KEYNAME *kn;

    if ((kn = _search_kn(0xff, s)) != NULL) {
        return kn->num;
    }

    return 0xff;
}

/* 0x70 -> SHIFTのようにキーコードからキー文字列を返す。見つからなければNULL */
static char *search_keystr(UINT8 n)
{
    KEYNAME *kn;

    if ((kn = _search_kn(n, NULL)) != NULL) {
        return kn->str;
    }

    return NULL;
}

static keyset_t *get_ks(int n)
{
    int i;
    keyset_t *p;

    for (i = 1, p = keyset_head; p != NULL; i++, p = p->next) {
        if (i == n) {
            break;
        }
    }

    return p;
}

int kc_get_ks_num(void)
{
    return keyset_num;
}
/* keysetがないなら0を返す */
int kc_is_ready(void)
{
    return (keyset_head != NULL)? 1 : 0;
}

/* keysetting番号num, token番号 tokenidの文字列を返す */
char *kc_get_keystrs(int ksnum, int tokenid)
{
    int i;
    keyset_t *p;
    char *s, *key;
    /* "RSHIFT A B C"の様な文字列が入る */
    //static char keystrs[strlen("RSHIFT ") * KC_MAXKEYDEF + 1];
    static char keystrs[255];//strlen("RSHIFT ") * KC_MAXKEYDEF + 1];

    keystrs[0] = '\0';

    p = get_ks(ksnum);
    if (p == NULL) {
        return NULL;
    }

    s = keystrs;
    for (i = 0; i < KC_MAXKEYDEF; i++) {
        key = search_keystr(p->key[tokenid][i]);
        if (key == NULL) {
            break;
        }
        strcat(s, key);
        strcat(s, " ");
        s += strlen(key) + 1;
    }

    /* 長いときは表示文字列を制限する */
    if (strlen(keystrs) > 18) {
        keystrs[16] = '.';
        keystrs[17] = '.';
        keystrs[18] = '\0';
    }

    return keystrs;
}

/* keysetting番号num, token番号 tokenidのkeycode列を返す */
void *kc_get_keycodes(int ksnum, int tokenid)
{
    keyset_t *p;

    p = get_ks(ksnum);
    if (p == NULL) {
        return NULL;
    }

    return &p->key[tokenid][0];
}

#if 0
char *kc_get_imagename(int ksnum)
{
}
#endif

char *kc_get_comment(int ksnum)
{
    keyset_t *p;

    p = get_ks(ksnum);

    return (p != NULL)? p->comment : NULL;
}


/* tokenが見つかったら: index、見つからなければ: -1 */
static int check_1st_token(char *s)
{
    int i;

    for (i = 0; i < KC_MAXTOKEN; i++) {
        if (strcmp(kc_token[i], s) == 0) {
            return i;
        }
    }

    return -1;
}

static int parse_keyconf(char *s)
{
    char token[KC_MAXLINE], tmp[KC_MAXLINE];
    static keyset_t *ks = NULL;
    int idx, i;
    UINT8 keynum;

    if (get_token(&s, token) < 0) {
        return 0;
    }

    if (strcmp(token, "[KeySetting]") == 0) {
        keyset_num++;
        ks = add_keyset();
        return (ks != NULL)? 0 : -1;
    }
    
    idx = check_1st_token(token);
    if (idx == -1) {
//        printf("parse error. token=[%s] in %d\n", token, lineno);
        return 0;
    }

    if (get_token(&s, token) < 0) {
//        printf("parse error. Not found 2nd token in %d\n", lineno);
        return 0;
    }

    if (strcmp(token, "=") != 0) {
//        printf("parse error. = expected but token is %s\n", token);
        return 0;
    }

    if (idx == kc_image) {
        if (get_quoted_str(&s, token) == 0) {
            if (add_fname(&ks->fname_head, strcpy(tmp, token)) == NULL) {
//                printf("malloc() error in add_fname()\n");
            }
        }
    } else if (idx == kc_comment) {
        if (get_quoted_str(&s, token) == 0) {
            strncpy(ks->comment, token, KC_MAXLINE - 1);
            ks->comment[KC_MAXLINE - 1] = '\0';
        }
    } else {
        for (i = 0; i < KC_MAXKEYDEF; i++) {
            if (get_token(&s, token) < 0) {
                return 0;
            }
            keynum = search_keynum(token);
            if (keynum == 0xff) {
//                printf("parse error. Unkonwn keyname [%s]\n", token);
                return 0;
            }
            ks->key[idx][i] = keynum;
        }
        if (get_token(&s, token) == 0) {
//            printf("parse error. Too much keyname [%s]\n", token);
            return 0;
        }
    }

    return 0;
}

void kc_loadfile(void)
{
    FILE *fd;
    char s[KC_MAXLINE];

    fd = fopen(PSP_KEY_TXT, "r");
    if (fd == NULL) {
        return;
    }
    while (fgets(s, KC_MAXLINE, fd)) {
        chomp(s); //改行コード変換はfgets()で処理しているかも。
        lineno++;
        if (parse_keyconf(s) < 0) {
            break;
        }
    }
    fclose(fd);
}
