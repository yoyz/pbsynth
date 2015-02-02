#ifndef _GP2X_H_
#define _GP2X_H_

    #ifdef GP2X
        #include <fcntl.h>
        #include <unistd.h>
        #include <sys/ioctl.h>
        #include <sys/soundcard.h>
        #include <sys/mman.h>
    #endif

// pad
#define GP2X_BUTTON_UP              (0)
#define GP2X_BUTTON_DOWN            (4)
#define GP2X_BUTTON_LEFT            (2)
#define GP2X_BUTTON_RIGHT           (6)
#define GP2X_BUTTON_UPLEFT          (1)
#define GP2X_BUTTON_UPRIGHT         (7)
#define GP2X_BUTTON_DOWNLEFT        (3)
#define GP2X_BUTTON_DOWNRIGHT       (5)
#define GP2X_BUTTON_CLICK           (18)
#define GP2X_BUTTON_A               (12)
#define GP2X_BUTTON_B               (13)
#define GP2X_BUTTON_X               (14)
#define GP2X_BUTTON_Y               (15)
#define GP2X_BUTTON_L               (10)
#define GP2X_BUTTON_R               (11)
#define GP2X_BUTTON_START           (8)
#define GP2X_BUTTON_SELECT          (9)
#define GP2X_BUTTON_VOLUP           (16)
#define GP2X_BUTTON_VOLDOWN         (17)

    void ramHack()
    {
        #ifdef GP2X
        unsigned long gp2x_mem;
        unsigned short *gp2x_memregs;
        volatile unsigned short *MEM_REG;

        // open MEM_REG
        gp2x_mem = open("/dev/mem", O_RDWR);
        gp2x_memregs=(unsigned short *)mmap(0, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, gp2x_mem, 0xc0000000);
        MEM_REG=&gp2x_memregs[0];

        unsigned long interrupt_flags = MEM_REG[0x808>>2],

        CAS=0,tRC=6-1,tRAS=4-1,tWR=1-1,tMRD=1-1,tRFC=1-1,tRP=2-1,tRCD=2-1; //craigix's timings

          MEM_REG[0x0808>>2] = 0xFF8FFFE7;        // Mask interrupts

          //gp2x_memregs[0x0910>>1] = s;                 // Set clock and wait

          //while(gp2x_memregs[0x0902>>1] & 1);

          gp2x_memregs[0x3802>>1] = ((tMRD<<12)|(tRFC<<8)|(tRP<<4)|(tRCD)); //set RAM tweaks
          gp2x_memregs[0x3804>>1] = ((CAS<<12)|(tRC<<8)|(tRAS<<4)|(tWR));

          gp2x_memregs[0x0924>>1] = 0x8900 + ((1)<<8); // Set upll timing prescaler to 1 (0x5A00 for fw 1.0.x)

          MEM_REG[0x0808>>2] = interrupt_flags;   // Turn on interrupts

        close(gp2x_mem);
        #endif
    }

#endif
