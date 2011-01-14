#include "stdafx.h"

#define CALC_C
#include "gui.h"
#include "core.h"
#include "81hw.h"
#include "83hw.h"
#include "83phw.h"
#include "83psehw.h"
#include "86hw.h"
#include "device.h"
#include "var.h"
#include "disassemble.h"

#ifdef _WINDOWS
#include "CCalcAddress.h"
#include "CPage.h"
#include "gifhandle.h"
#endif

/*
 * Determine the slot for a new calculator.  Return a pointer to the calc
 */
calc_t *calc_slot_new(void) {
#ifdef WITH_BACKUPS
	current_backup_index = 10;
#endif
	int i;
	for (i = 0; i < MAX_CALCS; i++) {
		if (calcs[i].active == FALSE) {
			memset(&calcs[i], 0, sizeof(calc_t));
			calcs[i].active = TRUE;
			calcs[i].gif_disp_state = GDS_IDLE;
			calcs[i].speed = 100;
			calcs[i].slot = i;
			return &calcs[i];
		}
	}
	return NULL;
}

u_int calc_count(void) {
	u_int count = 0;

	int i;
	for (i = 0; i < MAX_CALCS; i++) {
		if (calcs[i].active == TRUE)
			count++;
	}
	return count;
}

/* 81 */
int calc_init_81(LPCALC lpCalc) {
	/* INTIALIZE 81 */
	memory_init_81(&lpCalc->mem_c);
	tc_init(&lpCalc->timer_c, MHZ_2);
	CPU_init(&lpCalc->cpu, &lpCalc->mem_c, &lpCalc->timer_c);
	ClearDevices(&lpCalc->cpu);
	device_init_81(&lpCalc->cpu);
	/* END INTIALIZE 81 */

	return 0;
}

/*  82 83 */
static BOOL calc_init_83(LPCALC lpCalc, char *os) {
	/* INTIALIZE 83 */
	memory_init_83(&lpCalc->mem_c);
	tc_init(&lpCalc->timer_c, MHZ_6);
	CPU_init(&lpCalc->cpu, &lpCalc->mem_c, &lpCalc->timer_c);
	ClearDevices(&lpCalc->cpu);
	if (lpCalc->model == TI_82) {
		if (memcmp(os, "19.006", 6)==0) {
			device_init_83(&lpCalc->cpu, 0);
		} else {
			device_init_83(&lpCalc->cpu, 1);
		}
	} else {
		device_init_83(&lpCalc->cpu, 0);
	}
	/* END INTIALIZE 83 */

#ifdef WINVER // FIXME: dirty cheater!
	lpCalc->audio			= &lpCalc->cpu.pio.link->audio;
	lpCalc->audio->enabled	= FALSE;
	lpCalc->audio->init		= FALSE;
	lpCalc->audio->timer_c	= &lpCalc->timer_c;
#endif
	return TRUE;
}

/* 85 86 */
static int calc_init_86(LPCALC lpCalc) {

	/* INTIALIZE 86 */
	printf("initializing 86!\n");
	memory_init_86(&lpCalc->mem_c);
	tc_init(&lpCalc->timer_c, MHZ_4_8);
	CPU_init(&lpCalc->cpu, &lpCalc->mem_c, &lpCalc->timer_c);
	ClearDevices(&lpCalc->cpu);
	device_init_86(&lpCalc->cpu);
	/* END INTIALIZE 86 */

#ifdef WINVER // FIXME: dirty cheater!
	lpCalc->audio			= &lpCalc->cpu.pio.link->audio;
	lpCalc->audio->enabled	= FALSE;
	lpCalc->audio->init		= FALSE;
	lpCalc->audio->timer_c	= &lpCalc->timer_c;
#endif
	return 0;
}

/* 73 83+ */
int calc_init_83p(LPCALC lpCalc) {
	/* INTIALIZE 83+ */
	memory_init_83p(&lpCalc->mem_c);
	tc_init(&lpCalc->timer_c, MHZ_6);
	CPU_init(&lpCalc->cpu, &lpCalc->mem_c, &lpCalc->timer_c);
	ClearDevices(&lpCalc->cpu);
	device_init_83p(&lpCalc->cpu);
	/* END INTIALIZE 83+ */

#ifdef WINVER // FIXME: dirty cheater!
	lpCalc->audio			= &lpCalc->cpu.pio.link->audio;
	lpCalc->audio->enabled	= FALSE;
	lpCalc->audio->init		= FALSE;
	lpCalc->audio->timer_c	= &lpCalc->timer_c;
#endif
	return 0;
}

/* 83+se */
int calc_init_83pse(LPCALC lpCalc) {
	/* INTIALIZE 83+se */
	memory_init_83pse(&lpCalc->mem_c);
	tc_init(&lpCalc->timer_c, MHZ_6);
	CPU_init(&lpCalc->cpu, &lpCalc->mem_c, &lpCalc->timer_c);
	ClearDevices(&lpCalc->cpu);
	device_init_83pse(&lpCalc->cpu);
	/* END INTIALIZE 83+se */
#ifdef WINVER // FIXME: dirty cheater!
	lpCalc->audio			= &lpCalc->cpu.pio.link->audio;
	lpCalc->audio->enabled	= FALSE;
	lpCalc->audio->init		= FALSE;
	lpCalc->audio->timer_c	= &lpCalc->timer_c;
#endif
	return 0;
}

/* 84+ */
int calc_init_84p(LPCALC lpCalc) {
	/* INTIALIZE 84+ */
	memory_init_84p(&lpCalc->mem_c);
	tc_init(&lpCalc->timer_c, MHZ_6);
	CPU_init(&lpCalc->cpu, &lpCalc->mem_c, &lpCalc->timer_c);
	ClearDevices(&lpCalc->cpu);
	device_init_83pse(&lpCalc->cpu);
	void port21_84p(CPU_t *cpu, device_t *dev);
	lpCalc->cpu.pio.devices[0x21].code = (devp) &port21_84p;
#ifdef WITH_BACKUPS
	init_backups();
#endif
	/* END INTIALIZE 84+ */

#ifdef WINVER // FIXME: dirty cheater!
	lpCalc->audio			= &lpCalc->cpu.pio.link->audio;
	lpCalc->audio->enabled	= FALSE;
	lpCalc->audio->init		= FALSE;
	lpCalc->audio->timer_c	= &lpCalc->timer_c;
#endif
	return 0;
}


void calc_erase_certificate(unsigned char *mem, int size) {
	if (mem == NULL || size < 32768) return;

	memset(mem + size - 32768, 0xFF, 16384);

	mem[(size-32768)]			= 0x00;
	mem[(size-32768)+0x1FE0]	= 0x00;
	mem[(size-32768)+0x1FE1]	= 0x00;
	return;
}

BOOL rom_load(LPCALC lpCalc, LPCTSTR FileName) {
	if (lpCalc == NULL)
		return FALSE;
	TIFILE_t* tifile = newimportvar(FileName); //importvar(FileName, (int) NULL, (int) NULL);
	if (tifile == NULL)
		return FALSE;

	lpCalc->speed = 100;
	if (lpCalc->active)
		calc_slot_free(lpCalc);
	lpCalc->model = tifile->model;

	if (tifile->type == SAV_TYPE) {
		lpCalc->active 	= TRUE;
		switch (tifile->model) {
			case TI_82:
			case TI_83:
			{
				int size;
				char *rom = GetRomOnly(tifile->save,&size);
				char VerString[64];
				FindRomVersion(	tifile->model,
								VerString,
								(unsigned char *) rom,
								size);

				calc_init_83(lpCalc, VerString);
				break;
			}
			case TI_73:
			case TI_83P:
				calc_init_83p(lpCalc);
				break;
			case TI_84PSE:
			case TI_83PSE:
				calc_init_83pse(lpCalc);
				break;
			case TI_84P:
				calc_init_84p(lpCalc);
				break;
			case TI_85:
			case TI_86:
				calc_init_86(lpCalc);
				break;
			default:
				puts("Unknown model");
				FreeTiFile(tifile);
				return FALSE;
		}

		LoadSlot(tifile->save, lpCalc);
#ifdef WINVER
		StringCbCopy(lpCalc->rom_path, sizeof(lpCalc->rom_path), FileName);
#else
		strcpy(lpCalc->rom_path, FileName);
#endif
		FindRomVersion(	tifile->model,
						lpCalc->rom_version,
						lpCalc->mem_c.flash,
						lpCalc->mem_c.flash_size);
	} else if (tifile->type == ROM_TYPE) {

		switch (tifile->model) {
			case TI_81:
				calc_init_81(lpCalc);
				memcpy(	lpCalc->cpu.mem_c->flash,
						tifile->rom->data,
						(lpCalc->cpu.mem_c->flash_size<=tifile->rom->size)?lpCalc->cpu.mem_c->flash_size:tifile->rom->size);
				break;
			case TI_82:
			case TI_83:
				calc_init_83(lpCalc, tifile->rom->version);
				memcpy(	lpCalc->cpu.mem_c->flash,
						tifile->rom->data,
						(lpCalc->cpu.mem_c->flash_size<=tifile->rom->size)?lpCalc->cpu.mem_c->flash_size:tifile->rom->size);
				break;
			case TI_85:
			case TI_86:
				calc_init_86(lpCalc);
				memcpy(	lpCalc->cpu.mem_c->flash,
						tifile->rom->data,
						(lpCalc->cpu.mem_c->flash_size<=tifile->rom->size)?lpCalc->cpu.mem_c->flash_size:tifile->rom->size);
				break;
			case TI_73:
			case TI_83P:
				calc_init_83p(lpCalc);
				memcpy(	lpCalc->cpu.mem_c->flash,
						tifile->rom->data,
						(lpCalc->cpu.mem_c->flash_size<=tifile->rom->size)?lpCalc->cpu.mem_c->flash_size:tifile->rom->size);
				calc_erase_certificate(lpCalc->cpu.mem_c->flash,lpCalc->cpu.mem_c->flash_size);
				break;
			case TI_84P:
				calc_init_84p(lpCalc);
				memcpy(	lpCalc->cpu.mem_c->flash,
						tifile->rom->data,
						(lpCalc->cpu.mem_c->flash_size<=tifile->rom->size)?lpCalc->cpu.mem_c->flash_size:tifile->rom->size);
				calc_erase_certificate(lpCalc->cpu.mem_c->flash,lpCalc->cpu.mem_c->flash_size);
				break;
			case TI_84PSE:
			case TI_83PSE:
				calc_init_83pse(lpCalc);
				memcpy(	lpCalc->cpu.mem_c->flash,
						tifile->rom->data,
						(lpCalc->cpu.mem_c->flash_size<=tifile->rom->size)?lpCalc->cpu.mem_c->flash_size:tifile->rom->size);
				calc_erase_certificate(lpCalc->cpu.mem_c->flash,lpCalc->cpu.mem_c->flash_size);
				break;
			default:
				FreeTiFile(tifile);
				return FALSE;
		}

		lpCalc->active = TRUE;
		memcpy(lpCalc->rom_version, tifile->rom->version, sizeof(lpCalc->rom_version));
#ifdef WINVER
		StringCbCopy(lpCalc->rom_path, sizeof(lpCalc->rom_path), FileName);
#else
		strcpy(lpCalc->rom_path, FileName);
#endif
		

	} else {
		lpCalc = NULL;
	}
	if (lpCalc != NULL) {
		lpCalc->cpu.pio.model = lpCalc->model;
		if (tifile->save == NULL)
			calc_reset(lpCalc);
	}

	FreeTiFile(tifile);
	return TRUE;
}

void calc_slot_free(LPCALC lpCalc) {
	if (lpCalc == NULL)
		return;

	if (lpCalc->active) {
		lpCalc->active = FALSE;

#ifdef WINVER
		KillSound(lpCalc->audio);
		lpCalc->audio = NULL;
#endif

		free(lpCalc->mem_c.flash);
		lpCalc->mem_c.flash = NULL;
		free(lpCalc->mem_c.ram);
		lpCalc->mem_c.ram = NULL;
		free(lpCalc->mem_c.flash_break);
		lpCalc->mem_c.flash_break = NULL;
		free(lpCalc->mem_c.ram_break);
		lpCalc->mem_c.ram_break = NULL;

		free(lpCalc->cpu.pio.link);
		lpCalc->cpu.pio.link = NULL;

		free(lpCalc->cpu.pio.keypad);
		lpCalc->cpu.pio.keypad = NULL;

		free(lpCalc->cpu.pio.stdint);
		lpCalc->cpu.pio.stdint = NULL;

		free(lpCalc->cpu.pio.se_aux);
		lpCalc->cpu.pio.se_aux = NULL;

		free(lpCalc->cpu.pio.lcd);
		lpCalc->cpu.pio.lcd = NULL;

#ifdef WITH_BACKUPS
		if (do_backups)
			free_backups(lpCalc);
#endif

	}

}

void calc_turn_on(LPCALC lpCalc) {
	lpCalc->running = TRUE;
	calc_run_timed(lpCalc, 200);
	lpCalc->cpu.pio.keypad->on_pressed |= KEY_FALSEPRESS;
	calc_run_timed(lpCalc, 300);
	lpCalc->cpu.pio.keypad->on_pressed &= ~KEY_FALSEPRESS;
}

int calc_reset(LPCALC lpCalc) {
	//lpCalc->running = FALSE;
	return CPU_reset(&lpCalc->cpu);
}

/* Clear RAM and start calculator at $0000 
 * 10/20/10 Calc84 says that starting at $0000 is wrong we need to start on the boot page,
 */
int CPU_reset(CPU_t *lpCPU) {
	lpCPU->sp			= 0;
	lpCPU->interrupt	= FALSE;
	lpCPU->imode		= 1;
	lpCPU->ei_block		= FALSE;
	lpCPU->iff1			= FALSE;
	lpCPU->iff2			= FALSE;
	lpCPU->halt			= FALSE;
	lpCPU->read			= FALSE;
	lpCPU->write		= FALSE;
	lpCPU->output		= FALSE;
	lpCPU->input		= FALSE;
	lpCPU->prefix		= 0;
	lpCPU->mem_c->port27_remap_count = 0;
	lpCPU->mem_c->port28_remap_count = 0;
	if (lpCPU->pio.model >= TI_83P) {
		lpCPU->pc		= 0x8000;
		switch (lpCPU->pio.model) {
			case TI_83P: {
				/*	Address										page	write?	ram?	no exec?	*/
				bank_state_t banks[5] = {
					{lpCPU->mem_c->flash, 						0, 		FALSE,	FALSE, 	FALSE},
					{lpCPU->mem_c->flash+0x1f*PAGE_SIZE,		0x1f, 	FALSE, 	FALSE, 	FALSE},
					{lpCPU->mem_c->flash+0x1f*PAGE_SIZE,		0x1f, 	FALSE, 	FALSE, 	FALSE},
					{lpCPU->mem_c->ram,							0,		FALSE,	TRUE,	FALSE},
					{NULL,										0,		FALSE,	FALSE,	FALSE}
				};
				memcpy(lpCPU->mem_c->banks, banks, sizeof(banks));
				break;
			}
			case TI_83PSE:
			case TI_84PSE: {
				/*	Address										page	write?	ram?	no exec?	*/
				bank_state_t banks[5] = {
					{lpCPU->mem_c->flash, 						0, 		FALSE,	FALSE, 	FALSE},
					{lpCPU->mem_c->flash+0x7f*PAGE_SIZE,		0x7f, 	FALSE, 	FALSE, 	FALSE},
					{lpCPU->mem_c->flash+0x7f*PAGE_SIZE,		0x7f, 	FALSE, 	FALSE, 	FALSE},
					{lpCPU->mem_c->ram,							0,		FALSE,	TRUE,	TRUE},
					{NULL,										0,		FALSE,	FALSE,	FALSE}
				};

				memcpy(lpCPU->mem_c->banks, banks, sizeof(banks));
				break;
			}
			case TI_84P: {
				/*	Address									page	write?	ram?	no exec?	*/
				bank_state_t banks[5] = {
					{lpCPU->mem_c->flash, 					0, 		FALSE,	FALSE, 	FALSE},
					{lpCPU->mem_c->flash+0x3f*PAGE_SIZE,	0x3f, 	FALSE, 	FALSE, 	FALSE},
					{lpCPU->mem_c->flash+0x3f*PAGE_SIZE,	0x3f, 	FALSE, 	FALSE, 	FALSE},
					{lpCPU->mem_c->ram,						0,		FALSE,	TRUE,	TRUE},
					{NULL,									0,		FALSE,	FALSE,	FALSE}
				};
				memcpy(lpCPU->mem_c->banks, banks, sizeof(banks));
				break;
			}
		}
	} else {
		//memset(lpCalc->mem_c.ram, 0, lpCalc->mem_c.ram_size);
		lpCPU->pc			= 0;
	}
	return 0;
}

int calc_run_frame(LPCALC lpCalc) {
	double cpu_sync = tc_elapsed((&lpCalc->timer_c));

	while(lpCalc->running) {
		CPU_step(&lpCalc->cpu);

		/* sync CPU */
		if (tc_elapsed((&lpCalc->timer_c)) - cpu_sync > (1.0f / FPS)) {
			if (lpCalc->speed == MAX_SPEED) return 0;
			if (tc_elapsed((&lpCalc->timer_c)) - cpu_sync > (lpCalc->speed / FPS)) return 0;
		}
	}

	return 0;
}

int calc_run_tstates(LPCALC lpCalc, time_t tstates) {
	unsigned long long time_end = tc_tstates((&lpCalc->timer_c)) + tstates - lpCalc->time_error;

	while (lpCalc->running) {
		if (check_break(&lpCalc->mem_c, lpCalc->cpu.pc) & 1) {
#ifdef WINVER
			lpCalc->running = FALSE;
			bank_t *bank = &lpCalc->mem_c.banks[mc_bank(lpCalc->cpu.pc)];

			Z80_info_t z[2];
			disassemble(&lpCalc->mem_c, lpCalc->cpu.pc, 1, z);

			if (lpCalc->pCalcNotify != NULL) {
				
				bank_t *bank = &lpCalc->mem_c.banks[mc_bank(lpCalc->cpu.pc)];
				CCalcAddress *pCalcAddress = new CComObject<CCalcAddress>();
				pCalcAddress->Initialize(lpCalc->pWabbitemu, bank->ram, bank->page, lpCalc->cpu.pc);
				pCalcAddress->AddRef();
				lpCalc->pCalcNotify->Breakpoint(pCalcAddress);
			} else {
#endif
				gui_debug(lpCalc);
#ifdef WINVER
			}
#endif
			return 0;
		}

		unsigned long long oldTStates;
		if(lpCalc->profiler.running)
			oldTStates = tc_tstates((&lpCalc->timer_c));
		CPU_step(&lpCalc->cpu);
		if (lpCalc->profiler.running) {
			unsigned long long time = tc_tstates((&lpCalc->timer_c)) - oldTStates;
			lpCalc->profiler.totalTime += time;
			if(lpCalc->cpu.pc <= lpCalc->profiler.highAddress && lpCalc->cpu.pc >= lpCalc->profiler.lowAddress )
				lpCalc->profiler.data[lpCalc->cpu.pc / lpCalc->profiler.blockSize] += (long) time;
		}
		if (tc_tstates((&lpCalc->timer_c)) >= time_end) {
			lpCalc->time_error = tc_tstates((&lpCalc->timer_c)) - time_end;
			break;
		}
	}

	return 0;
}


#define FRAME_SUBDIVISIONS	1024
int calc_run_all(void) {
	int i, j, hub_val, active_calc = -1;

	for (i = 0; i < FRAME_SUBDIVISIONS; i++) {
		for (j = 0, hub_val = 0x0; j < MAX_CALCS; j++) {
			if (link_hub[j] != NULL)
				hub_val |= link_hub[j]->host;
		}
		link_hub[MAX_CALCS]->host = hub_val;
		for (j = 0; j < MAX_CALCS; j++) {
			if (calcs[j].active == TRUE) {
				active_calc = j;
				int time = ((long long) calcs[j].speed * calcs[j].timer_c.freq / FPS / 100) / FRAME_SUBDIVISIONS / 2;
				calc_run_tstates(&calcs[j], time);
				frame_counter += time;
#ifdef WITH_BACKUPS
				if (frame_counter >= calcs[j].timer_c.freq / 2) {
					frame_counter = 0;
					if (do_backups && calcs[j].speed <= 100)
						do_backup(&calcs[j]);
				}
#endif
				calc_run_tstates(&calcs[j], time);
				frame_counter += time;
#ifdef WITH_BACKUPS
				if (frame_counter >= calcs[j].timer_c.freq / 2) {
					frame_counter = 0;
					if (do_backups && calcs[j].speed <= 100)
						do_backup(&calcs[j]);
				}
#endif
			}
		}

		//this code handles screenshotting if were actually taking screenshots right now
		if (active_calc >= 0 && ((tc_elapsed(calcs[active_calc].cpu.timer_c) - calcs[active_calc].cpu.pio.lcd->lastgifframe) >= 0.01)) {
			handle_screenshot();
			calcs[active_calc].cpu.pio.lcd->lastgifframe += 0.01;
		}
	}

	return 0;
}

void port_debug_callback(void *arg1, void *arg2) {
	CPU_t *cpu = (CPU_t *) arg1;
	device_t *dev = (device_t *) arg2;
	LPCALC lpCalc = calc_from_cpu(cpu);
	gui_debug(lpCalc);
}

void mem_debug_callback(void *arg1) {
	CPU_t *cpu = (CPU_t *) arg1;
	LPCALC lpCalc = calc_from_cpu(cpu);
	gui_debug(lpCalc);
}

#ifdef WITH_BACKUPS
void do_backup(LPCALC lpCalc) {
	if (!lpCalc->running)
		return;
	int slot = lpCalc->slot;
	if (number_backup > MAX_BACKUPS) {
		debugger_backup* oldestBackup = backups[slot];
		while(oldestBackup->prev != NULL)
			oldestBackup = oldestBackup->prev;
		oldestBackup->next->prev = NULL;
		free_backup(oldestBackup);
	}
	debugger_backup *backup = (debugger_backup *) malloc(sizeof(debugger_backup));
	backup->save = SaveSlot(lpCalc);
	backup->next = NULL;
	backup->prev = backups[slot];
	if (backups[slot] != NULL)
		backups[slot]->next = backup;
	backups[slot] = backup;
	number_backup++;
}

void restore_backup(int index, LPCALC lpCalc) {
	int slot = lpCalc->slot;
	debugger_backup* backup = backups[slot];
	while (index > 0) {
		if (backup->prev == NULL)
			break;
		backup = backup->prev;
		free_backup(backup->next);
		index--;
	}
	//shouldnt happen
	if (backup != NULL)
		LoadSlot(backup->save, lpCalc);
	backups[slot] = backup;
}

void init_backups() {
	int i;
	number_backup = 0;
	for(i = 0; i < MAX_CALCS; i++)
		backups[i] = NULL;
}

void free_backup(debugger_backup* backup) {
	if (backup == NULL)
		return;
	FreeSave(backup->save);
	free(backup);
	number_backup--;
}

/*
 * Frees all backups from memory
 */
void free_backups(LPCALC lpCalc) {
	int slot = lpCalc->slot;
	debugger_backup *backup_prev, *backup = backups[slot];
	if (backup == NULL)
		return;
	do {
		backup_prev = backup->prev;
		free_backup(backup);
		backup = backup_prev;
	} while(backup != NULL);
	backups[slot] = NULL;
	number_backup = 0;
}

#endif

int calc_run_seconds(LPCALC lpCalc, double seconds) {
#ifdef MACVER
	time_t time = (seconds * CLOCKS_PER_SEC) / 1000;
#else
	time_t time = (time_t ) (seconds * CLOCKS_PER_SEC);
#endif
	return calc_run_timed(lpCalc, time);
}


// ticks
int calc_run_timed(LPCALC lpCalc, time_t time) {
	int frames = (int) time / TPF;

	int speed_backup = lpCalc->speed;

	lpCalc->speed = MAX_SPEED;
	while (frames--) 
		calc_run_frame(lpCalc);
	lpCalc->speed = speed_backup;
	return 0;
}

LPCALC calc_from_cpu(CPU_t *cpu) {
	int i;
	for (i = 0; i < MAX_CALCS; i++) {
		if (&calcs[i].cpu == cpu)
			return &calcs[i];
	}
	return NULL;
}

#ifdef WINVER
LPCALC calc_from_hwnd(HWND hwnd) {
	if (hwnd == NULL)
		return NULL;

	int slot;
	for (slot = 0; slot < MAX_CALCS; slot++) {
		if (calcs[slot].active) {
			if (hwnd == calcs[slot].hwndFrame ||
				hwnd == calcs[slot].hwndLCD ||
				hwnd == calcs[slot].hwndStatusBar ||
				hwnd == calcs[slot].hwndSmallClose ||
				hwnd == calcs[slot].hwndSmallMinimize) {
				return &calcs[slot];
			}
		}
	}
	return NULL;
}
#endif