#ifndef BFI2_H
# define BFI2_H

# ifndef MIN
#  define MIN(a,b)	((a)<(b)?(a):(b))
# endif

# ifndef MAX
#  define MAX(a,b)	((a)>(b)?(a):(b))
# endif

enum opcode {
	OPCODE_TRAP = 0x00,
	OPCODE_JUMP,
	OPCODE_JUMPZERO,
	OPCODE_MOVERIGHT,
	OPCODE_MOVELEFT,
	OPCODE_INCREMENT,
	OPCODE_DECREMENT,
	OPCODE_INPUT,
	OPCODE_OUTPUT,
};

# define MEMORY_SIZE	0x10000		// 64K
# define CODE_SIZE	0x10000

struct instruction {
	enum opcode opcode;
	union {
		unsigned int addr;
		unsigned int repeat;
	} param;
};

struct machine {
	char memory[MEMORY_SIZE];	// machine memory
	struct instruction ops[CODE_SIZE];	// instructions

	unsigned int ip;	// instruction pointer.
	unsigned int memp;	// memory pointer.
	unsigned int codesize;	// code size.

	int inputfd;		// input fd
	int outputfd;		// output fd
};

# include <termios.h>
# include <sys/ioctl.h>

struct interface {
	struct winsize ws;

	struct {
		char buf[8192];
		int length;
	} output;

	struct {
		char buf[CODE_SIZE];
		int length;
	} code;

	int inputfd;
	int outputfd;
	unsigned int startaddr;
};

# include <stdbool.h>

int brainfuck_compile(struct machine *mp, const char *code);
void brainfuck_eval_chr(struct machine *mp, int ch, bool execute);

struct machine *machine_create(struct machine **machinep);
void machine_destroy(struct machine **machinep);
void machine_run(struct machine *mp);

void raw(bool enable, bool noecho);
int cli(struct machine *machinep);
void cliread(struct interface *ifacep, struct machine *machinep);

void clr(void);
void moveto(int y, int x);
void termsize(struct winsize *wsp);

struct interface *interface_create(struct interface **interfacep);
void interface_destroy(struct interface **interfacep);
void interface_appendoutput(struct interface *ifacep, const char *fmt, ...);
void interface_clroutput(struct interface *ifacep);

void showhelp(void);

void drawheader(struct interface *ifacep, struct machine *machinep);
void drawseparator(struct interface *ifacep);
void drawfooter(struct interface *ifacep, struct machine *machinep);

void updatestartaddr(struct interface *ifacep, struct machine *machinep);
void updatecursor(struct interface *ifacep, struct machine *machinep);

void showmemory(struct interface *ifacep, struct machine *machinep);
void showoutput(struct interface *ifacep, struct machine *machinep);
void showcode(struct interface *ifacep, struct machine *machinep);

int userinput(struct interface *ifacep, struct machine *machinep, int ch);

// cols is 9 + 3 * 16 + 3 + 16 + 1
# define FIXED_LINE_WIDTH	77
# define FIXED_MEM_WIDTH	16
# define FIXED_CODE_LENGTH	60

#endif	/* !BFI2_H */
