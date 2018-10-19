#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#define SPEED B115200 
/* #define SPEED B19200  */
/* #define SPEED B9600  */

int main(int argc, char *argv[])
{
    struct termios tio;
    int fd;
    int total, dropped, prev;
    unsigned char co, ci, cp, cq, cr, cs, ct, cu, cv, cw, cx;
    int to, tn, i, j, w, r;
    int skip = 0;

    srand(time(0));

    if (argc != 2)
    {
        fprintf(stderr, "usage: looptest /dev/ttySn\n");
        exit(1);
    }

    fd = open(argv[1], O_RDWR | O_NONBLOCK);

    if (fd < 0)
    {
        perror("open()");
        fprintf(stderr, "Could not open serial device \"%s\".\n", argv[1]);
        exit(1);
    }

    tcgetattr(fd, &tio);
    cfmakeraw(&tio);

    tio.c_cflag &= ~( CSIZE | PARENB );
    tio.c_cflag |= ( CLOCAL | CRTSCTS | CS8 | HUPCL );

    tio.c_oflag &= ~( OLCUC | ONLCR | OCRNL | ONOCR | ONLRET | OFILL | 
                      NLDLY | CRDLY | TABDLY | BSDLY | VTDLY | FFDLY );
    tio.c_oflag |= NL0 | CR0 | TAB0 | BS0 | VT0 | FF0;

    tio.c_iflag |= IGNBRK;

    tio.c_lflag &= ~( ECHO | ECHOE | ECHOK | ECHONL | ECHOCTL | ECHOPRT );

    tio.c_cc[ VTIME ] = 5;
    tio.c_cc[ VMIN  ] = 1;

    if (tcsetattr(fd, TCSANOW, &tio))
    {
        perror("tcsetattr()");
        fprintf(stderr, "Could not control serial device \"%s\".\n", argv[1]);
        exit(1);
    }

    if (cfsetispeed(&tio, SPEED))
    {
        perror("cfsetispeed()");
        fprintf(stderr, "Could not control serial device \"%s\".\n", argv[1]);
        exit(1);
    }

    if (cfsetospeed(&tio, SPEED))
    {
        perror("cfsetispeed()");
        fprintf(stderr, "Could not control serial device \"%s\".\n", argv[1]);
        exit(1);
    }

    if (tcsetattr(fd, TCSANOW, &tio))
    {
        perror("tcsetattr()");
        fprintf(stderr, "Could not control serial device \"%s\".\n", argv[1]);
        exit(1);
    }

    tcflush(fd, TCIOFLUSH);
    tcflow(fd,  TCOON);
    tcflow(fd,  TCION);

    
    printf("Testing loopback.  Press Ctrl-C to quit.\n");
    prev    = 0;
    total   = 0;
    dropped = 0;

    printf("Synchronizing...\n");
    fflush(stdout);

    /* Make sure we're in sync with the target, by sending a ramp of values
       from 0..255 to the target, and seeing that ramp on our side here. */
    i = j = 0;
    while (j <= 100)
    {
        co = (i ^ 0xAA) & 0xFF;
        ci = -i;
//printf("writing %.2X\n", co & 0xFF);        
        while (( w = write(fd, &co, 1) ) != 1)
        {
            if (w == -1)
            {
                perror("write");
                exit(1);
            }
        }

//printf("wrote %.2X\n", co & 0xFF); fflush(stdout);
        while (( r = read(fd, &ci, 1) ) != 1)
        {
            if (r == -1 && !(errno == EAGAIN || errno == EINTR))
            {
                perror("read");
                exit(1);
            }
        }

//printf("read  %.2X\n", ci & 0xFF); fflush(stdout);
        if ((0xFF & (ci ^ co)) == 0)
        {
            j++;
            break;
        } else
        {
            while (read(fd, &ci, 1) == 1)
                /* drain */;

            skip++;
            j = 0;
            break;
        }

        i++;
    }

    printf("Synchronized!  Skips: %d\n", skip);
    fflush(stdout);

    skip = 0;

    i  = 0x00;
    ci = 0;
    cq = i++;
    cr = i++;
    cs = i++;
    ct = i++;
    cu = i++;
    cv = i++;
    cw = i++;
    co = i++;
    while (write(fd, &cq, 1) != 1) putchar('.');
    while (write(fd, &cr, 1) != 1) putchar('.');
    while (write(fd, &cs, 1) != 1) putchar('.');
    while (write(fd, &ct, 1) != 1) putchar('.');
    while (write(fd, &cu, 1) != 1) putchar('.');
    while (write(fd, &cv, 1) != 1) putchar('.');
    while (write(fd, &cw, 1) != 1) putchar('.');
    while (write(fd, &co, 1) != 1) putchar('.');
    
    printf("Synchronized.  Exercising loopback.  Last write: %.2X\n", co);
    fflush(stdout);

    to = time(0);

    while (1)
    {
        total++;

        cp = cq;
        cq = cr;
        cr = cs;
        cs = ct;
        ct = cu;
        cu = cv;
        cv = cw;
        cw = co;
        co = i++;
        while (( w = write(fd, &co, 1) ) != 1)
        {
            if (w == -1 && !(errno == EAGAIN || errno == EINTR))
            {
                perror("write");
                exit(1);
            }
        }

        if (!skip)
        {
            int r;

            while ((r = read(fd, &ci, 1)) != 1)
            {
                if (r == -1 && !(errno == EAGAIN || errno == EINTR))
                {
                    perror("read()");
                    fprintf(stderr, "\nError reading serial device \"%s\".\n", argv[1]);
                    exit(1);
                }
            }
        } else
        {
            skip--;
        }

//printf("read  %.2X\n", ci & 0xFF); fflush(stdout);
        if (ci != cp)
        {
            if (ci == cq)
            {
printf("\ndropped %.2X, got %.2X\n", cp, ci); fflush(stdout);
                dropped++;
                skip++;
//              write(fd, &co, 1);
            } else
            {
//              fprintf(stderr, 
//                      "\nOops:  Wrote %.2X, %.2X, %.2X, %.2X, %.2X "
//                      "and got %.2X\n",
//                      0xFF & cp, 0xFF & cq, 0xFF & cr, 0xFF & cs, 0xFF & co, 
//                      0xFF & ci);
//              exit(1);
            }
        }

        tn = time(0);
        if (tn != to)
        {
            printf("%10d sent  %10d dropped  %10d cps\r", total, dropped,
                    total - prev);
            prev = total;
            to = tn;
            fflush(stdout);
        }
    }

    return 0;
}
