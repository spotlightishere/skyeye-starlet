/*
        ppc_e500_exc.c - implementation of e500 exception 
        Copyright (C) 2003-2007 Skyeye Develop Group
        for help please send mail to <skyeye-developer@lists.sf.linuxforum.net>

        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
        (at your option) any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/*
 * 07/04/2007   Michael.Kang  <blackfin.kang@gmail.com>
 */

#include "ppc_cpu.h"
#include "ppc_e500_exc.h"
#include "ppc_mmu.h"
#include "tracers.h"

bool FASTCALL ppc_exception(uint32 type, uint32 flags, uint32 a)
{
	switch(type){
		case CRI_INPUT:
		case MACH_CHECK:
		case DATA_ST:
		case INSN_ST:
			fprintf(stderr,"Unimplement exception type %d, pc=0x%x.\n", type, gCPU.pc);
                        skyeye_exit(-1);

		case EXT_INT:
			gCPU.srr[0] = gCPU.npc;
                        gCPU.srr[1] = gCPU.msr;
			/* CE,ME,DE is unchanged, other bits should be clear */
			gCPU.msr &= 0x21200;
			break;
		case ALIGN:
		case PROG:
		case FP_UN:
			fprintf(stderr,"Unimplement exception type %d, pc=0x%x.\n", type, gCPU.pc);
                        skyeye_exit(-1);
                        break;
		case SYSCALL:
			gCPU.srr[0] = gCPU.npc;
                        gCPU.srr[1] = gCPU.msr;

			/* WE,EE,PR,IS,DS,FP,FE0,FE1 in msr should be cleared */
			gCPU.msr &= ~(0x2e930);
			break;
		case AP_UN:
                        fprintf(stderr,"Unimplement exception type %d, pc=0x%x.\n", type, gCPU.pc);
                        skyeye_exit(-1);

		case DEC:
			gCPU.srr[0] = gCPU.npc;
                        gCPU.srr[1] = gCPU.msr;

			/* CE,ME and DE bit unchanged, other bit should be clear*/
			gCPU.msr &= 0x21200; 

			/* DIS bit is set */
			gCPU.tsr |= 0x8000000;

			break;
		case FIT:
		case WD:
			fprintf(stderr,"Unimplement exception type %d, pc=0x%x.\n", type, gCPU.pc);
                        skyeye_exit(-1);
			break;
		case DATA_TLB:
			//printf(" In %s, DATA_TLB exp happened, pc=0x%x,addr=0x%x\n", __FUNCTION__, gCPU.pc, a);
			gCPU.srr[0] = gCPU.pc;
        	        gCPU.srr[1] = gCPU.msr;
	                //gCPU.esr |= ST;
	                gCPU.dear = a; /* save the data address accessed by exception instruction */

        	        gCPU.msr &= 0x21200;
			/* Update TLB */
		/**
		 * if TLBSELD = 00, MAS0[ESEL] is updated with the next victim information for TLB0.Finially, 		      * the MAS[0] field is updated with the incremented value of TLB0[NV].Thus, ESEL points to 
		 * the current victim
		 * (the entry to be replaced), while MAS0[NV] points to the next victim to be used if a TLB0 		      * entry is replaced
		 */

		/**
		 * update TLBSEL with TLBSELD
		 */
			e500_mmu.mas[0] = (e500_mmu.mas[4] & 0x10000000) | (e500_mmu.mas[0] & (~0x10000000)); 
			/* if TLBSELD == 0, update ESEL and NV bit in MAS Register*/
			if(!TLBSELD(e500_mmu.mas[4])){
				/* if TLBSELD == 0, ESEL = TLB[0].NV */
				/* update ESEL of MAS0 */
				e500_mmu.mas[0] = (e500_mmu.tlb0_nv << 16) | (e500_mmu.mas[0] & 0xFFF0FFFF) ;
				/* update NV of MAS0 , NV = ~TLB[0].NV */
				e500_mmu.mas[0] = (~e500_mmu.tlb0_nv & 0x1) | (e500_mmu.mas[0] & 0xFFFFFFFE);
			}
		/**
		 * Set EPN to EPN of access
		 */
			e500_mmu.mas[2] = (a & 0xFFFFF000) | (e500_mmu.mas[3] &0xFFF);
		/**
		 * Set TSIZE[0 - 3] to TSIZED
		 */ 
			e500_mmu.mas[1] = (e500_mmu.mas[4] & 0xF00)|(e500_mmu.mas[1] & 0xFFFFF0FF);
			break;
		case INSN_TLB:
		case DEBUG:
		default:
			fprintf(stderr,"Unknown exception type %d.pc=0x%x\n", type, gCPU.pc);
			skyeye_exit(-1);
	}
	gCPU.npc = (gCPU.ivpr & 0xFFFF0000) | (gCPU.ivor[type] & 0xFFF0);
	return true;
}
