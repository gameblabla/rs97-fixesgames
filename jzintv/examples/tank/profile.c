/* ======================================================================== */
/*  Show function-level profile data for a program.                         */
/*  Derives the function-level data from instruction-level data from        */
/*  jzIntv and the listing file from as1600.                                */
/* ======================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAXPROC (4096)

#define FL_PROC (0x0001)    /* Procedure start  */
#define FL_ENDP (0x0002)    /* Procedure end    */
#define FL_BRNC (0x0004)    /* Branch           */
#define FL_BTRG (0x0008)    /* Branch target    */
#define FL_CALL (0x0010)    /* Function call    */

typedef unsigned long long uint_64;
typedef unsigned int       uint_32;
typedef unsigned short     uint_16;
typedef unsigned char      uint_8;
typedef   signed long long sint_64;
typedef   signed int       sint_32;
typedef   signed short     sint_16;
typedef   signed char      sint_8;

typedef struct info_t
{
    char        *symb;      /* Symbol associated w/ address */
    uint_32     cycles;     /* Total cycles for address.    */
    uint_16     flags;      /* Flags.                       */
    uint_16     btarg;      /* Target of this branch.       */
    char        *code;      /* Disassembly for this line.   */
} info_t;

info_t addr[65536];

char buf[4096];

/*  When we parse the profile, glean cycle count and program structure 
 *  information as we go. */
void parse_profile(FILE *f)
{
    uint_32 c, a;
    int in_profile = 0;
    char instr[8], arg1[8], arg2[8], *s1, *s2;
    int a1, a2, r1, r2;
    int line_no = 0;

    while (fgets(buf, sizeof(buf), f) != NULL)
    {
        line_no++;
        if (strncmp(buf, "Profile", 7) == 0) 
        {
            in_profile = 1;
            continue;
        }
        if (!in_profile)
            continue;

        if (!sscanf(buf, "%d", &c))
            continue;

        if (!sscanf(buf+25, "%x", &a))
            continue;

        if ((s2 = strchr(buf, '\r')) != NULL) *s2 = 0;
        if ((s2 = strchr(buf, '\n')) != NULL) *s2 = 0;

        strncpy(instr, buf+54, 4);
        instr[5] = 0;

        a1 = a2 = -1;
        r1 = r2 = -1;

        s1 = arg1;
        s2 = buf + 63;

        while (*s2 && *s2 != ' ' && *s2 != ',')
        {
            *s1++ = *s2++;
        }
        *s1 = 0;
        s2 += *s2 == ',';

        s1 = arg2;

        while (*s2 && *s2 != ' ' && *s2 != ',')
        {
            *s1++ = *s2++;
        }
        *s1 = 0;

        s1 = arg1;
        s1 += *s1 == '#';
        
        if      (*s1 == '$') { sscanf(s1 + 1, "%x", &a1); }
        else if (*s1 == 'R') { sscanf(s1 + 1, "%d", &r1); }
        else if (*s1)        { sscanf(s1 + 1, "%d", &a1); }

        s1 = arg2;
        s1 += *s1 == '#';
        
        if      (*s1 == '$') { sscanf(s1 + 1, "%x", &a2); }
        else if (*s1 == 'R') { sscanf(s1 + 1, "%d", &r2); }
        else if (*s1)        { sscanf(s1 + 1, "%d", &a2); }



        /* Poke the data into the info structure */
        if (addr[a].code) free(addr[a].code);

        addr[a].cycles += c; 
        addr[a].code    = strdup(buf + 50);

        if (r2 == 7 ||
            (r2 == -1 && r1 == 7 && !strcmp(instr, "PULR")) ||
            (instr[0] == 'J' && instr[1] != 'S') ||
            (instr[0] == 'B'))
        {
            addr[a].flags |= FL_BRNC;
            addr[a].btarg = r2 == 7  ? a1 :
                            r1 == 7  ?  0 :
                            a2 != -1 ? a2 :
                            a1 != -1 ? a1 : 0;
        }

        if (instr[0] == 'J' || instr[1] == 'S')
        {
            addr[a].flags |= FL_CALL;
            addr[a].btarg  = a2 != -1 ? a2 : 0;
        }
    }

    return;
}

/* When we parse the listing file, look for label definitions, PROC, ENDP */
void parse_listing(FILE *f)
{
    int a = 0x0000, i, o;
    int pa = 0x0000;
    char lbl[2048], *s1, *s2;
    char *cur_proc = NULL;
    int in_body = 0;
    int lofs[2] =  { 0, 36 };
    int line_no = 0;

    while (fgets(buf, sizeof(buf), f) != NULL)
    {
        line_no++;
        

        if (buf[0] == 0x0C) 
        {
//if (in_body == 0) { printf("Found body on line %d\n", line_no); }
            in_body = 1;
        }

        if (!in_body)  /* parse symbol table at top */
        {
//printf("buf[0]=%.2X\n", buf[0] & 0xFF);

            for (i = 0; i < 2; i++)
            {
                o = lofs[i];

                if (sscanf(buf + o, "%x", &a))
                {
                    strncpy(lbl, buf + o + 9, 16);
                    lbl[16] = 0;
                    if ((s1 = strchr(lbl, ' ')) != NULL)
                        *s1 = 0;

                    if (a > 0x10000)
                    {
                        printf("BOGUS a=%.8X: buf='%s'\n", a, buf);
                    } else
                    {
//                      if (addr[a].symb) free(addr[a].symb);
//                      addr[a].symb = strdup(lbl);
                        if (!addr[a].symb) addr[a].symb = strdup(lbl);
                    }
                }
            }
            continue;
        }


        buf[sizeof(buf) - 1] = 0;
        if ((s2 = strchr(buf, '\r')) != NULL) *s2 = 0;
        if ((s2 = strchr(buf, '\n')) != NULL) *s2 = 0;

        if (strlen(buf) < 2) continue;
        pa = a;

        if ((s1 = strchr(buf, ';')) != NULL) 
        {
            while (s1 < buf + sizeof(buf))
                *s1++ = 0;
        }

        /* Lines starting w/ " 0x" are label definitions, always. */
        if (!strncmp(" 0x", buf, 3) && sscanf(buf + 3, "%x", &a) == 1)
        {
            s1 = buf + 24;
//printf("type 1, %-5d: '%s'\n", line_no, buf);

            if (!isspace(*s1))
            {
                s2 = s1;
                while (!isspace(*s2) && *s2 != ':') s2++;
                *s2 = 0;

                if (s1[0] == '@' && s1[0] == '@')
                {
                    sprintf(lbl, "%s.%s", cur_proc ? cur_proc : "", s1+2);
                } else
                {
                    strcpy(lbl, s1);
                }

                *s2 = ' ';


/*
                if (addr[a].symb) free(addr[a].symb);
                addr[a].symb = strdup(lbl);
*/
                if (!addr[a].symb) addr[a].symb = strdup(lbl);

//printf("(B) %.8X / %s\n", a, addr[a].symb);

                if (strstr(buf, "PROC"))
                {
                    addr[a].flags |= FL_PROC;
                    if (cur_proc)
                    {
                        printf("WARNING: Nested procs? line %d, %s vs. %s\n",
                                line_no, cur_proc, lbl);
                        free(cur_proc);
                    }
                    cur_proc = strdup(lbl);
//printf("(B) PROC: %s\n", cur_proc);
                }
            }
            continue;
        }

        /* Lines starting w/ "xxxx" are code lines, and maybe have labels */
        /* Labels will always be in display column 24 if present.       */
        if (strchr("0123456789abcdef", buf[0]) && 
            sscanf(buf, "%x", &a) == 1 && (s1 = strchr(buf, '\t')) != NULL)
        {
            i = s1 - buf;
            while (*s1 == '\t') 
            { 
                s1++; 
                i = (i + 8) & ~7;
            }

            if (i == 24 && !isspace(*s1))
            {
                s2 = s1;
                while (*s2 && !isspace(*s2) && *s2 != ':') s2++;
                *s2 = 0;
                
                if (s1[0] == '@' && s1[0] == '@')
                {
                    sprintf(lbl, "%s.%s", cur_proc ? cur_proc : "", s1+2);
                } else
                {
                    strcpy(lbl, s1);
                }

                if (addr[a].symb) free(addr[a].symb);
                addr[a].symb = strdup(lbl);
//printf("(C) %.8X / %s\n", a, addr[a].symb);
            }
            continue;
        }

        /* ENDP's appear on a line w/out code or addresses. */
        if (isspace(buf[0]) && strstr(buf, "ENDP"))
        {
            if (!cur_proc)
            {
                printf("WARNING: ENDP w/out PROC?  Line %d, addr %.8X\n",
                        line_no, pa);
            } else
            {
                free(cur_proc);
            }
            cur_proc = NULL;
            addr[pa].flags |= FL_ENDP;
        }
    }

    return;
}


typedef struct proc_t
{
    char     *name;
    uint_32  cycles;
    uint_16  a_lo, a_hi;
    double   pct;
} proc_t;


proc_t proc[MAXPROC];
int procs = 0;


void scan_procs(void)
{
    int a, c = 0;
    char *cur_proc = NULL;
    char fakelbl[8];
    uint_16 a_lo = 0x0000;

    for (a = 0; a < 0x10000; a++)
    {
        if (addr[a].flags & FL_PROC)
        {
            if (!addr[a].symb)
            {
                sprintf(fakelbl, "L%.4X", a & 0xFFFF);
                addr[a].symb = strdup(fakelbl);
            } 

            cur_proc = addr[a].symb;
            a_lo = a;
            c = 0;
        }

        c += addr[a].cycles;

        if ((addr[a].flags & FL_ENDP) && cur_proc)
        {
            if (procs >= MAXPROC)
            {
                printf("WARNING:  Too many procs, max %d\n", MAXPROC);
            } else
            {
                proc[procs].name = strdup(cur_proc);
                proc[procs].cycles = c;
                proc[procs].a_lo = a_lo;
                proc[procs].a_hi = a;
                procs++;
            }
            cur_proc = 0;
        }
    }
}

int comp_proc_cycles(const void *a, const void *b)
{
    const proc_t *pa = (const proc_t *)a; 
    const proc_t *pb = (const proc_t *)b;

    if (pa->cycles > pb->cycles) return -1;
    if (pa->cycles < pb->cycles) return  1;

    if (pa->a_lo   > pb->a_lo)   return  1;
    if (pa->a_lo   < pb->a_lo)   return -1;

    return 0;
}

void print_summary_report(void)
{
    int i;
    uint_32 total = 0;
    double pct;

    qsort(proc, procs, sizeof(proc_t), comp_proc_cycles);
    
    for (i = 0; i < procs; i++)
        total += proc[i].cycles;

    printf("Total profiled cycles:  %d\n\n", total);

    printf("%-30s | %-9s | %-12s | %-12s\n",
            "Function", "Range", "Cycles", "% of Tot");

    for (i = 0; i < procs; i++)
    {
        pct = 100.0 * (double)proc[i].cycles / (double)total;
        
        proc[i].pct = pct;

        if (proc[i].cycles == 0) continue;

        printf("%-30s | %.4X-%.4X | %12d | %6.2f%%\n", 
                proc[i].name, proc[i].a_lo, proc[i].a_hi,
                proc[i].cycles, pct);
    }

    printf("\n");

    return;
}

void print_loop_breakdown(void)
{
    int i, a, aa, r_lo, r_hi;
    uint_32 c;
    double pct;

    printf("Loop breakdown for each function over 2%%\n\n");

    /* look for backward branches that land within the proc. */
    for (i = 0; i < procs; i++)
    {
        if (proc[i].pct < 2.0)
            continue;

        printf("FUNCTION: %-30s | %.4X-%.4X | %12d\n", 
                proc[i].name, proc[i].a_lo, proc[i].a_hi,
                proc[i].cycles);

        for (a = proc[i].a_lo; a <= proc[i].a_hi; a++)
        {
            if ((addr[a].flags & FL_BRNC) && 
                addr[a].btarg <= a    &&
                addr[a].btarg >= proc[i].a_lo)
            {
                c = 0;
                r_hi = a;
                r_lo = addr[a].btarg;
                for (aa = r_lo; aa <= r_hi; aa++)
                    c += addr[aa].cycles;

                pct = 100.0 * (double)c / (double)proc[i].cycles;
                printf("    LOOP: %-30s | %.4X-%.4X | %12d | %6.2f%%\n",
                        addr[r_lo].symb ? addr[r_lo].symb : "",
                        r_lo, r_hi, c, pct);
            }
        }

        printf("\n");
    }
}


int main(int argc, char *argv[])
{
    FILE *f;

    memset(addr, 0, sizeof(addr));

    if (argc != 3)
    {
        printf("Usage:  profile dump.hst file.lst\n");
        exit(1);
    }

    f = fopen(argv[1], "r");
    if (!f)
    {
        printf("Could not open '%s' for reading\n", argv[1]);
        exit(1);
    }

    parse_profile(f);

    fclose(f);


    f = fopen(argv[2], "r");
    if (!f)
    {
        printf("Could not open '%s' for reading\n", argv[2]);
        exit(1);
    }

    parse_listing(f); 

    fclose(f);

    scan_procs();
    print_summary_report();
    print_loop_breakdown();

    return 0;
}

