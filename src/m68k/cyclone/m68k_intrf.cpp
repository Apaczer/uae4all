#include "sysconfig.h"
#include "sysdeps.h"
#include "memory.h"
#include "custom.h"
#include "autoconf.h"
#include "uae.h"
#include "ersatz.h"
#include "m68k/m68k_intrf.h"

#include <stdio.h>

#define dprintf(...)
//#define dprintf(f,...) printf("%05i: %s: " f "\n",M68KCONTEXT.cycles_counter,__FUNCTION__,##__VA_ARGS__)
#define mdprintf(...)
//#define mdprintf(f,...) \
if (M68KCONTEXT.cycles_counter >= 14000000) \
printf("%05i|%03i: %06x: %s: " f "\n",M68KCONTEXT.cycles_counter,m68k_context.cycles,m68k_context.pc-m68k_context.membase,__FUNCTION__,##__VA_ARGS__)
#define dprintf_al(f,...) printf("%05i: %s: " f "\n",M68KCONTEXT.cycles_counter,__FUNCTION__,##__VA_ARGS__)

struct Cyclone m68k_context;
M68KCONTEXT_t M68KCONTEXT;

static unsigned char  read8 (unsigned int a);
static unsigned short read16(unsigned int a);
static unsigned int   read32(unsigned int a);
static void write8 (unsigned int a,unsigned char  d);
static void write16(unsigned int a,unsigned short d);
static void write32(unsigned int a,unsigned int   d);

static unsigned int check_pc(unsigned int pc);
static void irq_ack(int level);
static int  unrecognized_callback(void);


void init_m68k(void)
{
	CycloneInit();
	memset(&m68k_context, 0, sizeof(m68k_context));
	m68k_context.checkpc = check_pc;
	m68k_context.read8   = m68k_context.fetch8  = read8;
	m68k_context.read16  = m68k_context.fetch16 = read16;
	m68k_context.read32  = m68k_context.fetch32 = read32;
	m68k_context.write8  = write8;
	m68k_context.write16 = write16;
	m68k_context.write32 = write32;
	m68k_context.IrqCallback = irq_ack;
	m68k_context.UnrecognizedCallback = unrecognized_callback;
}


/* FAME interface */
void m68k_release_timeslice(void)
{
	m68k_context.cycles = 0;
	dprintf(" ");
}


int m68k_raise_irq(int level, int vector)
{
	M68KCONTEXT.interrupts[0] |= 1 << level;
	m68k_irq_update();
	dprintf("lvl: %i, line: %i, irqs: %02x", level, m68k_context.irq, M68KCONTEXT.interrupts[0]);

	return 0;
}


int m68k_lower_irq(int level)
{
	M68KCONTEXT.interrupts[0] &= ~(1 << level);
	m68k_irq_update();
	dprintf("lvl: %i, line: %i, irqs: %02x", level, m68k_context.irq, M68KCONTEXT.interrupts[0]);

	return 0;
}


int m68k_reset(void)
{
	dprintf("");
	m68k_context.stopped = 0;
	m68k_context.osp = 0;
	m68k_context.srh = 0x27; // Supervisor mode
	m68k_context.flags = 0;
//	m68k_context.irq = 0;
//	m68k_context.a[7] = m68k_context.read32(0); // Stack Pointer, set by caller
//	m68k_context.membase=0;
//	m68k_context.pc = m68k_context.checkpc(PicoCpu.read32(4)); // Program Counter, set by caller

	return 0;
}


int m68k_emulate(int cycles)
{
	cycles -= 1;
	m68k_context.cycles = cycles;
	m68k_context.stopped = (M68KCONTEXT.execinfo & 0x80) ? 1 : 0;
	//dprintf("c = %i, stoped: %i", cycles, m68k_context.stopped);

	if (m68k_context.stopped /*|| M68KCONTEXT.interrupts[0]*/)
		dprintf("c = %i, stopped: %i, irq: %02x", cycles, m68k_context.stopped, M68KCONTEXT.interrupts[0]);

	CycloneRun(&m68k_context);

	M68KCONTEXT.cycles_counter += cycles - m68k_context.cycles;
	if (m68k_context.stopped)
	     M68KCONTEXT.execinfo |=  0x80;
	else M68KCONTEXT.execinfo &= ~0x80;

	return 0;
}


/* helper funcs */

// must call this whenever M68KCONTEXT.interrupts[0] gets changed
void m68k_irq_update(void)
{
	int level, ints = M68KCONTEXT.interrupts[0];

	for (level = 7; level && !(ints & (1 << level)); level--);

	m68k_context.irq = level;
}


static unsigned int check_pc(unsigned int pc)
{
	pc -= m68k_context.membase;
	pc &= ~1;

	// do we really need to do this? I've seen some cores never clearing the upper bits
	// and pushing them, etc. But in that case kickstart hangs because cmp @ fc090e fails.
	pc &= ~0xff000000;

	uae_u8 *p = baseaddr[pc >> 16];

	if ((int)p & 1)
	{
		printf("Cyclone problem: branched to unknown memory location: %06x\n", pc);
		p = baseaddr[0xf8];
	}

	//dprintf("newpc=%06x, base=%p, result=%08x, oldpc=%06x", pc, p, (unsigned)p + pc, m68k_context.pc - m68k_context.membase);
	m68k_context.membase = (unsigned)p;
	return (unsigned)p + pc;
}


static void irq_ack(int level)
{
	M68KCONTEXT.interrupts[0] &= ~(1 << level);
	m68k_irq_update();

	dprintf("level: %i, irqs: %02x", level, M68KCONTEXT.interrupts[0]);
}


static int unrecognized_callback(void)
{
	unsigned pc = m68k_context.pc - m68k_context.membase;
	unsigned opcode = *(uae_u16 *)m68k_context.pc;

	dprintf("op 0x%04x @ 0x%06x", opcode, pc);

//	if (opcode != 0x4E7B) *(int*)0 = 1;

	if (cloanto_rom && (opcode & 0xF100) == 0x7100) {
		dprintf("  cloanto"); // whatever it is
		_68k_dreg((opcode >> 9) & 7) = (uae_s8)(opcode & 0xFF);
		m68k_context.pc += 2;
		return 1;
	}
	if (opcode == 0x4E7B && get_long(0x10) == 0 && (pc & 0xF80000) == 0xF80000) {
		dprintf("  68020");
		write_log("Your Kickstart requires a 68020 CPU. Giving up.\n");
		set_special (SPCFLAG_BRK);
		quit_program = 1;
		m68k_context.cycles = 0;
		return 1;
	}
	if (opcode == 0xFF0D) {
		if ((pc & 0xF80000) == 0xF80000) {
			dprintf("  dummy");
			// This is from the dummy Kickstart replacement
			uae_u16 arg = *(uae_u16 *)(pc+2);
			m68k_context.pc += 4;
			ersatz_perform(arg);
		} else if ((pc & 0xFFFF0000) == RTAREA_BASE) {
			dprintf("  stop");
			// User-mode STOP replacement 
			mispcflags|=SPCFLAG_STOP;
			m68k_context.stopped = 1;
			m68k_context.cycles = 0;
		}
		return 1;
	}
	if ((opcode & 0xF000) == 0xA000 && (pc & 0xFFFF0000) == RTAREA_BASE) {
		dprintf("  call");
		// Calltrap.
		m68k_context.pc += 2;
#ifdef USE_AUTOCONFIG
		call_calltrap (opcode & 0xFFF);
#endif
		return 1;
	}
	if ((opcode & 0xF000) == 0xF000) {
		dprintf("  exp8");
		return 0;
	}
	if ((opcode & 0xF000) == 0xA000) {
		dprintf("  expA");
		return 0;
	}
	dprintf("  real illegal");
	write_log ("Illegal instruction: %04x at %08x\n", opcode, pc);
	return 0;
}


/* memory handlers */
static unsigned char read8(unsigned int a)
{
	a &= ~0xff000000;
	uae_u8 *p = baseaddr[a>>16];
	if ((int)p & 1)
	{
		addrbank *ab = (addrbank *) ((unsigned)p & ~1);
		uae_u32 ret = ab->bget(a);
		mdprintf("@ %06x, handler, =%02x", a, ret);
		return ret;
	}
	else
	{
		mdprintf("@ %06x, =%02x", a, p[a^1]);
		return p[a^1];
	}
}

static unsigned short read16(unsigned int a)
{
	a &= ~0xff000000;
	uae_u16 *p = (uae_u16 *) baseaddr[a>>16];
	if ((int)p & 1)
	{
		addrbank *ab = (addrbank *) ((unsigned)p & ~1);
		uae_u32 ret = ab->wget(a);
		mdprintf("@ %06x, handler, =%04x", a, ret);
		return ret;
	}
	else
	{
		mdprintf("@ %06x, =%04x", a, p[a>>1]);
		return p[a>>1];
	}
}

static unsigned int read32(unsigned int a)
{
	a &= ~0xff000000;
	uae_u16 *p = (uae_u16 *) baseaddr[a>>16];
	if ((int)p & 1)
	{
		addrbank *ab = (addrbank *) ((unsigned)p & ~1);
		uae_u32 ret = ab->lget(a);
		mdprintf("@ %06x, handler, =%08x", a, ret);
		return ret;
	}
	else
	{
		mdprintf("@ %06x, =%08x", a, (p[a>>1]<<16)|p[(a>>1)+1]);
		a >>= 1;
		return (p[a]<<16)|p[a+1];
	}
}

static void write8(unsigned int a, unsigned char d)
{
	a &= ~0xff000000;
	uae_u8 *p = baseaddr[a>>16];
	if ((int)p & 1)
	{
		addrbank *ab = (addrbank *) ((unsigned)p & ~1);
		mdprintf("@ %06x, handler, =%02x", a, d);
		ab->bput(a, d&0xff);
	}
	else
	{
		mdprintf("@ %06x, =%02x", a, d);
		p[a^1] = d;
	}
}

static void write16(unsigned int a,unsigned short d)
{
	a &= ~0xff000000;
	uae_u16 *p = (uae_u16 *) baseaddr[a>>16];
	if ((int)p & 1)
	{
		addrbank *ab = (addrbank *) ((unsigned)p & ~1);
		mdprintf("@ %06x, handler, =%04x", a, d);
		ab->wput(a, d&0xffff);
	}
	else
	{
		mdprintf("@ %06x, =%04x", a, d);
		p[a>>1] = d;
	}
}

static void write32(unsigned int a,unsigned int d)
{
	a &= ~0xff000000;
	uae_u16 *p = (uae_u16 *) baseaddr[a>>16];
	if ((int)p & 1)
	{
		addrbank *ab = (addrbank *) ((unsigned)p & ~1);
		mdprintf("@ %06x, handler, =%08x", a, d);
		ab->lput(a, d);
	}
	else
	{
		mdprintf("@ %06x, =%08x", a, d);
		a >>= 1;
		p[a] = d >> 16;
		p[a+1] = d;
	}
}


/* nothing to do? */
void init_memmaps(addrbank* banco)
{
}

void map_zone(unsigned addr, addrbank* banco, unsigned realstart)
{
}

void clear_fame_mem_dummy(void)
{
}

