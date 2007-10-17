/*	$OpenBSD: pctrvar.h,v 1.1 2007/10/17 02:30:23 deraadt Exp $	*/

/*
 * Copyright (c) 2007 Mike Belopuhov, Aleksey Lomovtsev
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Pentium performance counter control program for OpenBSD.
 * Copyright 1996 David Mazieres <dm@lcs.mit.edu>.
 *
 * Modification and redistribution in source and binary forms is
 * permitted provided that due credit is given to the author and the
 * OpenBSD project by leaving this copyright notice intact.
 */

#ifndef _PCTRVAR_H_
#define _PCTRVAR_H_

#define ARCH_UNDEF		0
#define ARCH_I386		1
#define ARCH_AMD64		2

#define CPU_UNDEF		0
#define CPU_P5			1
#define CPU_P6			2
#define CPU_CORE		3
#define CPU_AMD			4

#define PCTR_AMD_NUM		4
#define PCTR_INTEL_NUM		2	/* Intel supports only 2 counters */

#define PCTR_MAX_FUNCT		0xff
#define PCTR_MAX_UMASK		0xff

#define CFL_MESI		0x01	/* Unit mask accepts MESI encoding */
#define CFL_SA			0x02	/* Unit mask accepts Self/Any bit */
#define CFL_C0			0x04	/* Counter 0 only */
#define CFL_C1			0x08	/* Counter 1 only */
#define CFL_ED			0x10	/* Edge detect is needed */

/* Pentium defines */
#define PCTR_P5_K		0x040
#define PCTR_P5_U		0x080
#define PCTR_P5_C		0x100

/* AMD & Intel shared defines */
#define PCTR_X86_U		0x010000
#define PCTR_X86_K		0x020000
#define PCTR_X86_E		0x040000
#define PCTR_X86_EN		0x400000
#define PCTR_X86_I		0x800000
#define PCTR_X86_UM_M		0x0800
#define PCTR_X86_UM_E		0x0400
#define PCTR_X86_UM_S		0x0200
#define PCTR_X86_UM_I		0x0100
#define PCTR_X86_UM_MESI	(PCTR_X86_UM_M | PCTR_X86_UM_E | \
				    PCTR_X86_UM_S | PCTR_X86_UM_I)
#define PCTR_X86_UM_A		0x2000

#define PCTR_X86_UM_SHIFT	8
#define PCTR_X86_CM_SHIFT	24


struct ctrfn {
	u_int32_t	 fn;
	int		 flags;
	char		*name;
	char		*desc;
};

struct ctrfn p5fn[] = {
	{ 0x00, 0, "Data read", NULL },
	{ 0x01, 0, "Data write", NULL },
	{ 0x02, 0, "Data TLB miss", NULL },
	{ 0x03, 0, "Data read miss", NULL },
	{ 0x04, 0, "Data write miss", NULL },
	{ 0x05, 0, "Write (hit) to M or E state lines", NULL },
	{ 0x06, 0, "Data cache lines written back", NULL },
	{ 0x07, 0, "Data cache snoops", NULL },
	{ 0x08, 0, "Data cache snoop hits", NULL },
	{ 0x09, 0, "Memory accesses in both pipes", NULL },
	{ 0x0a, 0, "Bank conflicts", NULL },
	{ 0x0b, 0, "Misaligned data memory references", NULL },
	{ 0x0c, 0, "Code read", NULL },
	{ 0x0d, 0, "Code TLB miss", NULL },
	{ 0x0e, 0, "Code cache miss", NULL },
	{ 0x0f, 0, "Any segment register load", NULL },
	{ 0x12, 0, "Branches", NULL },
	{ 0x13, 0, "BTB hits", NULL },
	{ 0x14, 0, "Taken branch or BTB hit", NULL },
	{ 0x15, 0, "Pipeline flushes", NULL },
	{ 0x16, 0, "Instructions executed", NULL },
	{ 0x17, 0, "Instructions executed in the V-pipe", NULL },
	{ 0x18, 0, "Bus utilization (clocks)", NULL },
	{ 0x19, 0, "Pipeline stalled by write backup", NULL },
	{ 0x1a, 0, "Pipeline stalled by data memory read", NULL },
	{ 0x1b, 0, "Pipeline stalled by write to E or M line", NULL },
	{ 0x1c, 0, "Locked bus cycle", NULL },
	{ 0x1d, 0, "I/O read or write cycle", NULL },
	{ 0x1e, 0, "Noncacheable memory references", NULL },
	{ 0x1f, 0, "AGI (Address Generation Interlock)", NULL },
	{ 0x22, 0, "Floating-point operations", NULL },
	{ 0x23, 0, "Breakpoint 0 match", NULL },
	{ 0x24, 0, "Breakpoint 1 match", NULL },
	{ 0x25, 0, "Breakpoint 2 match", NULL },
	{ 0x26, 0, "Breakpoint 3 match", NULL },
	{ 0x27, 0, "Hardware interrupts", NULL },
	{ 0x28, 0, "Data read or data write", NULL },
	{ 0x29, 0, "Data read miss or data write miss", NULL },
	{ 0x0,  0, NULL, NULL },
};

struct ctrfn p6fn[] = {
	{ 0x03, 0, "LD_BLOCKS",
	 "Number of store buffer blocks." },
	{ 0x04, 0, "SB_DRAINS",
	 "Number of store buffer drain cycles." },
	{ 0x05, 0, "MISALIGN_MEM_REF",
	 "Number of misaligned data memory references." },
	{ 0x06, 0, "SEGMENT_REG_LOADS",
	 "Number of segment register loads." },
	{ 0x10, CFL_C0, "FP_COMP_OPS_EXE",
	 "Number of computational floating-point operations executed." },
	{ 0x11, CFL_C1, "FP_ASSIST",
	 "Number of floating-point exception cases handled by microcode." },
	{ 0x12, CFL_C1, "MUL",
	 "Number of multiplies." },
	{ 0x13, CFL_C1, "DIV",
	 "Number of divides." },
	{ 0x14, CFL_C0, "CYCLES_DIV_BUSY",
	 "Number of cycles during which the divider is busy." },
	{ 0x21, 0, "L2_ADS",
	 "Number of L2 address strobes." },
	{ 0x22, 0, "L2_DBUS_BUSY",
	 "Number of cycles durring which the data bus was busy." },
	{ 0x23, 0, "L2_DBUS_BUSY_RD",
	 "Number of cycles during which the data bus was busy transferring "
	 "data from L2 to the processor." },
	{ 0x24, 0, "L2_LINES_IN",
	 "Number of lines allocated in the L2." },
	{ 0x25, 0, "L2_M_LINES_INM",
	 "Number of modified lines allocated in the L2." },
	{ 0x26, 0, "L2_LINES_OUT",
	 "Number of lines removed from the L2 for any reason." },
	{ 0x27, 0, "L2_M_LINES_OUTM",
	 "Number of modified lines removed from the L2 for any reason." },
	{ 0x28, CFL_MESI, "L2_IFETCH",
	 "Number of L2 instruction fetches." },
	{ 0x29, CFL_MESI, "L2_LD",
	 "Number of L2 data loads." },
	{ 0x2a, CFL_MESI, "L2_ST",
	 "Number of L2 data stores." },
	{ 0x2e, CFL_MESI, "L2_RQSTS",
	 "Number of L2 requests." },
	{ 0x43, 0, "DATA_MEM_REF",
	 "Number of all memory references, both cacheable and non-cacheable." },
	{ 0x44, 0, "DATA_MEM_CACHE_REF",
	 "Number of L1 data cacheable read and write operations." },
	{ 0x45, 0, "DCU_LINES_IN",
	 "Total lines allocated in the DCU." },
	{ 0x46, 0, "DCU_M_LINES_IN",
	 "Number of M state lines allocated in the DCU." },
	{ 0x47, 0, "DCU_M_LINES_OUT",
	 "Number of M state lines evicted from the DCU.  "
	 "This includes evictions via snoop HITM, intervention or replacement" },
	{ 0x48, 0, "DCU_MISS_OUTSTANDING",
	 "Weighted number of cycles while a DCU miss is outstanding." },
	{ 0x60, 0, "BUS_REQ_OUTSTANDING",
	 "Number of bus requests outstanding." },
	{ 0x61, 0, "BUS_BNR_DRV",
	 "Number of bus clock cycles during which the processor is "
	 "driving the BNR pin." },
	{ 0x62, CFL_SA, "BUS_DRDY_CLOCKS",
	 "Number of clocks during which DRDY is asserted." },
	{ 0x63, CFL_SA, "BUS_LOCK_CLOCKS",
	 "Number of clocks during which LOCK is asserted." },
	{ 0x64, 0, "BUS_DATA_RCV",
	 "Number of bus clock cycles during which the processor is "
	 "receiving data." },
	{ 0x65, CFL_SA, "BUS_TRAN_BRD",
	 "Number of burst read transactions." },
	{ 0x66, CFL_SA, "BUS_TRAN_RFO",
	 "Number of read for ownership transactions." },
	{ 0x67, CFL_SA, "BUS_TRANS_WB",
	 "Number of write back transactions." },
	{ 0x68, CFL_SA, "BUS_TRAN_IFETCH",
	 "Number of instruction fetch transactions." },
	{ 0x69, CFL_SA, "BUS_TRAN_INVAL",
	 "Number of invalidate transactions." },
	{ 0x6a, CFL_SA, "BUS_TRAN_PWR",
	 "Number of partial write transactions." },
	{ 0x6b, CFL_SA, "BUS_TRANS_P",
	 "Number of partial transactions." },
	{ 0x6c, CFL_SA, "BUS_TRANS_IO",
	 "Number of I/O transactions." },
	{ 0x6d, CFL_SA, "BUS_TRAN_DEF",
	 "Number of deferred transactions." },
	{ 0x6e, CFL_SA, "BUS_TRAN_BURST",
	 "Number of burst transactions." },
	{ 0x6f, CFL_SA, "BUS_TRAN_MEM",
	 "Number of memory transactions." },
	{ 0x70, CFL_SA, "BUS_TRAN_ANY",
	 "Number of all transactions." },
	{ 0x79, 0, "CPU_CLK_UNHALTED",
	 "Number of cycles during which the processor is not halted." },
	{ 0x7a, 0, "BUS_HIT_DRV",
	 "Number of bus clock cycles during which the processor is "
	 "driving the HIT pin." },
	{ 0x7b, 0, "BUS_HITM_DRV",
	 "Number of bus clock cycles during which the processor is "
	 "driving the HITM pin." },
	{ 0x7e, 0, "BUS_SNOOP_STALL",
	 "Number of clock cycles during which the bus is snoop stalled." },
	{ 0x80, 0, "IFU_IFETCH",
	 "Number of instruction fetches, both cacheable and non-cacheable." },
	{ 0x81, 0, "IFU_IFETCH_MISS",
	 "Number of instruction fetch misses." },
	{ 0x85, 0, "ITLB_MISS",
	 "Number of ITLB misses." },
	{ 0x86, 0, "IFU_MEM_STALL",
	 "Number of cycles that the instruction fetch pipe stage is stalled, "
	 "including cache mises, ITLB misses, ITLB faults, "
	 "and victim cache evictions" },
	{ 0x87, 0, "ILD_STALL",
	 "Number of cycles that the instruction length decoder is stalled" },
	{ 0xa2, 0, "RESOURCE_STALLS",
	 "Number of cycles during which there are resource-related stalls." },
	{ 0xc0, 0, "INST_RETIRED",
	 "Number of instructions retired." },
	{ 0xc1, CFL_C0, "FLOPS",
	 "Number of computational floating-point operations retired." },
	{ 0xc2, 0, "UOPS_RETIRED",
	 "Number of UOPs retired." },
	{ 0xc4, 0, "BR_INST_RETIRED",
	 "Number of branch instructions retired." },
	{ 0xc5, 0, "BR_MISS_PRED_RETIRED",
	 "Number of mispredicted branches retired." },
	{ 0xc6, 0, "CYCLES_INT_MASKED",
	 "Number of processor cycles for which interrupts are disabled." },
	{ 0xc7, 0, "CYCLES_INT_PENDING_AND_MASKED",
	 "Number of processor cycles for which interrupts are disabled "
	 "and interrupts are pending." },
	{ 0xc8, 0, "HW_INT_RX",
	 "Number of hardware interrupts received." },
	{ 0xc9, 0, "BR_TAKEN_RETIRED",
	 "Number of taken branches retired." },
	{ 0xca, 0, "BR_MISS_PRED_TAKEN_RET",
	 "Number of taken mispredictioned branches retired." },
	{ 0xd0, 0, "INST_DECODER",
	 "Number of instructions decoded." },
	{ 0xd2, 0, "PARTIAL_RAT_STALLS",
	 "Number of cycles or events for partial stalls." },
	{ 0xe0, 0, "BR_INST_DECODED",
	 "Number of branch instructions decoded." },
	{ 0xe2, 0, "BTB_MISSES",
	 "Number of branches that miss the BTB." },
	{ 0xe4, 0, "BR_BOGUS",
	 "Number of bogus branches." },
	{ 0xe6, 0, "BACLEARS",
	 "Number of times BACLEAR is asserted." },
	{ 0x0, 0, NULL, NULL}
};

struct ctrfn corefn[] = {
	{ 0x03, 0, "LD_BLOCKS",
	 "Number of store buffer blocks." },
	{ 0x04, 0, "SB_DRAINS",
	 "Number of store buffer drain cycles." },
	{ 0x06, 0, "SEGMENT_REG_LOADS",
	 "Number of segment register loads." },
	{ 0x07, 0, "SSE_PRE_EXEC",
	 "Streaming SIMD Extensions Prefetch NTA instructions executed" },
	{ 0x08, 0, "DTLB_MISSES",
	 "Memory accesses missed the DTLB" },
	{ 0x09, 0, "MEMORY_DISAMBIGUATION",
	 "Memory disambiguation reset cycles" },
	{ 0x0c, 0, "PAGE_WALKS",
	 "Number of page-walks executed" },
	{ 0x10, 0, "FP_COMP_OPS_EXE",
	 "Floating point computational micro-ops executed" },
	{ 0x11, 0, "FP_ASSIST",
	 "Number of floating-point exception cases handled by microcode." },
	{ 0x12, 0, "MUL",
	 "Number of multiplies." },
	{ 0x13, 0, "DIV",
	 "Number of divides." },
	{ 0x14, 0, "CYCLES_DIV_BUSY",
	 "Number of cycles during which the divider is busy." },
	{ 0x18, 0, "IDLE_DURING_DIV",
	 "Cycles the divider is busy and all other execution units are idle." },
	{ 0x19, 0, "DELAYED_BYPASS",
	 "Delayed bypass to FP operation." },
	{ 0x21, 0, "L2_ADS",
	 "Number of L2 address strobes." },
	{ 0x23, 0, "L2_DBUS_BUSY_RD",
	 "Number of cycles during which the data bus was busy transferring "
	 "data from L2 to the core."},
	{ 0x24, 0, "L2_LINES_IN",
	 "Number of lines allocated in the L2 (L2 cache misses.)" },
	{ 0x25, 0, "L2_M_LINES_INM",
	 "Number of modified lines allocated in the L2." },
	{ 0x26, 0, "L2_LINES_OUT",
	 "Number of lines removed from the L2 for any reason." },
	{ 0x27, 0, "L2_M_LINES_OUTM",
	 "Number of modified lines removed from the L2 for any reason." },
	{ 0x28, CFL_MESI, "L2_IFETCH",
	 "Number of L2 instruction fetches." },
	{ 0x29, CFL_MESI, "L2_LD",
	 "Number of L2 data loads." },
	{ 0x2a, CFL_MESI, "L2_ST",
	 "Number of L2 data stores." },
	{ 0x2e, CFL_MESI, "L2_RQSTS",
	 "Number of L2 requests." },
	{ 0x30, CFL_MESI, "L2_REJECT_CYCLES",
	 "Number of cycles L2 is busy and rejecting new requests." },
	{ 0x32, CFL_MESI, "L2_NO_REQUEST_CYCLES",
	 "Number of cycles there is no request to access L2." },
	{ 0x3a, 0, "EST_TRANS_ALL",
	 "Number of any Intel Enhanced SpeedStep Technology transitions." },
	{ 0x3b, CFL_ED, "THERMAL_TRIP",
	 "Duration in a thermal trip based on the current core clock." },
	{ 0x3c, 0, "NONHLT_REF_CYCLES",
	 "Number of non-halted bus cycles." },
	{ 0x40, CFL_MESI, "L1D_CACHE_LD",
	 "L1 cacheable data reads." },
	{ 0x41, CFL_MESI, "L1D_CACHE_ST",
	 "L1 cacheable data writes." },
	{ 0x42, CFL_MESI, "L1D_CACHE_LOCK",
	 "L1 data cacheable locked reads." },
	{ 0x43, 0, "L1D_ALL_REF",
	 "All memory references to the L1 DCACHE."},
	{ 0x45, 0, "L1D_REPL",
	 "Total lines allocated in the L1 DCACHE." },
	{ 0x46, 0, "L1D_M_REPL",
	 "Number of M state lines allocated in the L1 DCACHE." },
	{ 0x47, 0, "L1D_M_EVICT",
	 "Number of M state lines evicted from the L1 DCACHE.  "
	 "This includes evictions via snoop HITM, intervention or "
	 "replacement." },
	{ 0x48, 0, "L1D_PEND_MISS",
	 "Total number of outstanding L1 data cache misses at any cycle." },
	{ 0x49, 0, "DTLB_MISS",
	 "Number of data references that missed TLB." },
	{ 0x4b, 0, "SSE_PRE_MISS",
	 "Number of cache misses by the SSE Prefetch NTA instructions." },
	{ 0x4c, 0, "LOAD_HIT_PRE",
	 "Load operations conflicting with a software prefetch." },
	{ 0x4e, 0, "L1D_PREFETCH",
	 "L1 DCACHE prefetch requests" },
	{ 0x4f, 0, "L1_PREF_REQ",
	 "Number of L1 prefetch requests due to DCU cache misse.s" },
	{ 0x60, CFL_SA, "BUS_REQ_OUTSTANDING",
	 "Number of bus requests outstanding." },
	{ 0x61, CFL_SA, "BUS_BNR_DRV",
	 "Number of bus clock cycles during which the processor is "
	 "driving the BNR pin." },
	{ 0x62, CFL_SA, "BUS_DRDY_CLOCKS",
	 "Number of clocks during which DRDY is asserted." },
	{ 0x63, CFL_SA, "BUS_LOCK_CLOCKS",
	 "Number of clocks during which LOCK is asserted." },
	{ 0x64, 0, "BUS_DATA_RCV",
	 "Number of bus clock cycles during which the processor is "
	 "receiving data." },
	{ 0x65, CFL_SA, "BUS_TRAN_BRD",
	 "Number of burst read transactions." },
	{ 0x66, CFL_SA, "BUS_TRAN_RFO",
	 "Number of read for ownership transactions." },
	{ 0x67, CFL_SA, "BUS_TRANS_WB",
	 "Number of write back transactions." },
	{ 0x68, CFL_SA, "BUS_TRAN_IFETCH",
	 "Number of instruction fetch transactions." },
	{ 0x69, CFL_SA, "BUS_TRAN_INVAL",
	 "Number of invalidate transactions." },
	{ 0x6a, CFL_SA, "BUS_TRAN_PWR",
	 "Number of partial write transactions." },
	{ 0x6b, CFL_SA, "BUS_TRANS_P",
	 "Number of partial transactions." },
	{ 0x6c, CFL_SA, "BUS_TRANS_IO",
	 "Number of I/O transactions." },
	{ 0x6d, CFL_SA, "BUS_TRAN_DEF",
	 "Number of deferred transactions." },
	{ 0x6e, CFL_SA, "BUS_TRAN_BURST",
	 "Number of burst transactions." },
	{ 0x6f, CFL_SA, "BUS_TRAN_MEM",
	 "Number of memory transactions." },
	{ 0x70, CFL_SA, "BUS_TRAN_ANY",
	 "Number of all transactions." },
	{ 0x77, CFL_MESI, "BUS_SNOOPS",
	 "Number of external bus cycles while bus lock signal asserted." },
	{ 0x78, 0, "CMP_SNOOP",
	 "Number of L1 DCACHE snoops by other core." },
	{ 0x7a, CFL_SA, "BUS_HIT_DRV",
	 "Number of bus clock cycles during which the processor is "
	 "driving the HIT pin." },
	{ 0x7b, CFL_SA, "BUS_HITM_DRV",
	 "Number of bus clock cycles during which the processor is "
	 "driving the HITM pin." },
	{ 0x7d, CFL_SA, "BUS_NOT_IN_USE",
	 "Number of cycles there is no transaction from the core." },
	{ 0x7e, CFL_SA, "SNOOP_STALL_DRV",
	 "Number of clock cycles during which the bus is snoop stalled." },
	{ 0x7f, 0, "BUS_IO_WAIT",
	 "Number of cycles during which IO requests wait int the bus queue." },
	{ 0x80, 0, "L1I_READS",
	 "Number of instruction fetches, both cacheable and non-cacheable." },
	{ 0x81, 0, "L1I_MISSES",
	 "Number of instruction fetch misses." },
	{ 0x82, 0, "ITLB_MISS",
	 "Number of ITLB misses." },
	{ 0x83, 0, "INSQ_QUEUE",
	 "Cycles during which the instruction queue is full." },
	{ 0x85, 0, "ITLB_MISSES",
	 "Number of ITLB misses." },
	{ 0x86, 0, "CYCLES_L1I_MEM_STALLED",
	 "Number of cycles that the instruction fetches stalled, "
	 "including cache mises, ITLB misses, ITLB faults, "
	 "and victim cache evictions" },
	{ 0x87, 0, "ILD_STALL",
	 "Number of cycles that the instruction length decoder is stalled." },
	{ 0x88, 0, "BR_INST_EXEC",
	 "Number of branch instructions executed." },
	{ 0x89, 0, "BR_MISSP_EXEC",
	 "Number of mispredicted branch instructions that were executed." },
	{ 0x8a, 0, "BR_BAC_MISSP_EXEC",
	 "Number of branch instructions that were mispredicted at decoding." },
	{ 0x8b, 0, "BR_CND_EXEC",
	 "Number of conditional branch instructions executed, but not "
	 "necessarily retired." },
	{ 0x8c, 0, "BR_CND_MISSP_EXEC",
	 "Number of mispredicted conditional branch instructions that "
	 "were executed." },
	{ 0x8d, 0, "BR_IND_EXEC",
	 "Number of indirect branch instructions that were executed." },
	{ 0x8e, 0, "BR_IND_MISSP_EXEC",
	 "number of mispredicted indirect branch instructions that were "
	 "executed." },
	{ 0x8f, 0, "BR_RET_EXEC",
	 "Number of RET instructions that were executed." },
	{ 0x90, 0, "BR_RET_MISSP_EXEC",
	 "Number of mispredicted RET instructions that were executed." },
	{ 0x91, 0, "BR_RET_BAC_MISSP_EXEC",
	 "Number of RET instructions that were executed and were mispredicted "
	 "at decoding." },
	{ 0x92, 0, "BR_CALL_EXEC",
	 "Number of CALL instructions executed." },
	{ 0x93, 0, "BR_CALL_MISSP_EXEC",
	 "Number of mispredicted CALL instructions that were executed." },
	{ 0x94, 0, "BR_IND_CALL_EXEC",
	 "Number of indirect CALL instructions that were executed." },
	{ 0x97, 0, "BR_TKN_BUBBLE_1",
	 "Number of times a taken branch predicted taken with bubble 1." },
	{ 0x98, 0, "BR_TKN_BUBBLE_2",
	 "Number of times a taken branch predicted taken with bubble 2." },
	{ 0xa0, 0, "RS_UOPS_DISPATCHED",
	 "Number of microops dispatched for execution." },
	{ 0xa2, 0, "RESOURCE_STALL",
	 "Number of cycles while there us a resource related stall." },
	{ 0xaa, 0, "MACRO_INSTS",
	 "Number of instructions decoded (but not necessarily executed "
	 "or retired)." },
	{ 0xab, 0, "ESP",
	 "ESP register operations." },
	{ 0xb0, 0, "SIMD_UOPS_EXEC",
	 "Number of SIMD micro-ops executed (excluding stores)." },
	{ 0xb1, 0, "SIMD_SAT_UOP_EXEC",
	 "Number of SIMD saturated arithmetic micro-ops executed." },
	{ 0xb3, 0, "SIMD_UOP_TYPE_EXEC",
	 "Number of SIMD packed multiply micro-ops executed." },
	{ 0xc0, 0, "INST_RETIRED",
	 "Number of instructions retired." },
	{ 0xc1, 0, "X87_OPS_RETIRED",
	 "Number of computational floating-point operations retired." },
	{ 0xc2, 0, "UOPS_RETIRED",
	 "Number of UOPs retired." },
	{ 0xc3, 0, "MACHINE_NUKES",
	 "Number of times the pipeline is restarted due to either "
	 "multithreaded memory ordering conflicts or memory disambiguation "
	 "misprediction." },
	{ 0xc4, 0, "BR_INST_RETIRED",
	 "Number of branch instructions retired." },
	{ 0xc5, 0, "BR_MISS_PRED_RETIRED",
	 "Number of mispredicted branches retired." },
	{ 0xc6, 0, "CYCLES_INT_MASKED",
	 "Number of processor cycles for which interrupts are disabled." },
	{ 0xc7, 0, "SIMD_INST_RETIRED",
	 "Number of SSE instructions retired." },
	{ 0xc8, 0, "HW_INT_RCV",
	 "Number of hardware interrupts received." },
	{ 0xc9, 0, "ITLB_MISS_RETIRED",
	 "Number of retired instructions that missed the ITLB when they "
	 "were fetched."},
	{ 0xca, 0, "SIMD_COMP_INST_RETIRED",
	 "Number of computational SSE instructions retired." },
	{ 0xcb, 0, "MEM_LOAD_RETIRED",
	 "Number of retired load operations that missed the L1 DCACHE." },
	{ 0xcc, 0, "FP_MMX_TRANS_TO_MMX",
	 "Number of the first MMX instructions following a floating-point "
	 "instruction." },
	{ 0xcd, 0, "SIMD_ASSIST",
	 "Number of SIMD assists invoked." },
	{ 0xce, 0, "SIMD_INSTR_RETIRED",
	 "Number of SIMD instructions that retired." },
	{ 0xcf, 0, "SIMD_SAT_INSTR_RETIRED",
	 "Number of saturated arithmetic SIMD instructions that retired." },
	{ 0xd0, 0, "INSTR_DECODED",
	 "Number of instructions decoded." },
	{ 0xd2, 0, "RAT_STALLS",
	 "Number of cycles or events for partial stalls." },
	{ 0xd4, 0, "SEG_RENAME_STALLS",
	 "Number of stalls due to the lack of renaming resources." },
	{ 0xd5, 0, "SEG_REG_RENAMES",
	 "Number of times the segment register is renamed." },
	{ 0xd7, 0, "ESP_UOPS",
	 "Number of ESP folding instruction decoded." },
	{ 0xd8, 0, "SIMD_FD_RET",
	 "Number of SSE/SSE2 instructions retired." },
	{ 0xd9, 0, "SIMD_FP_COM_RET",
	 "Number of SSE/SSE2 compute instructions retired." },
	{ 0xda, 0, "FUSED_UOPS_RET",
	 "Number of all fused uops retired." },
	{ 0xdb, 0, "UNFUSION",
	 "Number of all unfusion events in the ROB." },
	{ 0xdc, 0, "RESOURCE_STALLS",
	 "Number of cycles when the number of instructions in the pipeline "
	 "waiting for retirement reaches the limit the processor can handle." },
	{ 0xe0, 0, "BR_INST_DECODED",
	 "Number of branch instructions decoded." },
	{ 0xe2, 0, "BTB_MISSES",
	 "Number of branches the BTB did not produce a prediction." },
	{ 0xe4, 0, "BOGUS_BR",
	 "Number of byte sequences that were mistakenly detected as taken "
	 "branch instructions." },
	{ 0xe6, 0, "BACLEARS",
	 "Number of times BACLEAR is asserted." },
	{ 0xf0, 0, "PREF_RQSTS_UP",
	 "Number of upward prefetches issued from the Data Prefetch Logic "
	 "(DPL) to the L2 cache." },
	{ 0xf8, 0, "PREF_RQSTS_DN",
	 "Number of downward prefetches issued from the Data Prefetch Logic "
	 "(DPL) to the L2 cache." },
	{ 0x0, 0, NULL, NULL }
};

struct ctrfn amdfn[] = {
	{ 0x00, 0, "Dispatched FPU operations", NULL },
	{ 0x01, 0, "Cycles with no FPU ops retired", NULL },
	{ 0x02, 0, "Dispatched fast flag FPU operations", NULL },
	{ 0x20, 0, "Segment register loads", NULL },
	{ 0x21, 0, "Pipeline restart due to self-modifying code", NULL },
	{ 0x22, 0, "Pipeline restart due to probe hit", NULL },
	{ 0x23, 0, "LS2 buffer is full", NULL },
	{ 0x24, 0, "Locked operations", NULL },
	{ 0x26, 0, "Retired CFLUSH instructions", NULL },
	{ 0x27, 0, "Retired CPUID instructions", NULL },
	{ 0x40, 0, "Data cache accesses", NULL },
	{ 0x41, 0, "Data cache misses", NULL },
	{ 0x42, 0, "Data cache refills from L2 or system", NULL },
	{ 0x43, 0, "Data cache refills from system", NULL },
	{ 0x44, 0, "Data cache lines evicted", NULL },
	{ 0x45, 0, "L1 DTLB miss and L2 DTLB hit", NULL },
	{ 0x46, 0, "L1 DTLB miss and L2 DTLB miss", NULL },
	{ 0x47, 0, "Misaligned access", NULL },
	{ 0x48, 0, "Microarchitectural late cancel of an access", NULL },
	{ 0x49, 0, "Microarchitectural early cancel of an access", NULL },
	{ 0x4a, 0, "Single bit ECC errors recorded by scrubber", NULL },
	{ 0x4b, 0, "Prefetch instructions dispatched", NULL },
	{ 0x4c, 0, "DCACHE misses by locked instructions", NULL },
	{ 0x65, 0, "Memory requests by type", NULL },
	{ 0x67, 0, "Data prefetcher", NULL },
	{ 0x6c, 0, "System read responses by coherency state", NULL },
	{ 0x6d, 0, "Quadwords written to system", NULL },
	{ 0x76, 0, "CPU clocks not halted", NULL },
	{ 0x7d, 0, "Requests to L2 cache", NULL },
	{ 0x7e, 0, "L2 cache misses", NULL },
	{ 0x7f, 0, "L2 cache fill/writeback", NULL },
	{ 0x80, 0, "ICACHE fetches", NULL },
	{ 0x81, 0, "ICACHE misses", NULL },
	{ 0x82, 0, "ICACHE refills from L2", NULL },
	{ 0x83, 0, "ICACHE refills from system", NULL },
	{ 0x84, 0, "L1 ITLB miss and L2 ITLB hit", NULL },
	{ 0x85, 0, "L1 ITLB miss and L2 ITLB miss", NULL },
	{ 0x86, 0, "Pipeline restart due to instruction stream probe", NULL },
	{ 0x87, 0, "Instruction fetch stall", NULL },
	{ 0x88, 0, "Return stack hits", NULL },
	{ 0x89, 0, "Return stack overflows", NULL },
	{ 0xc0, 0, "Retired instructions", NULL },
	{ 0xc1, 0, "Retired microops", NULL },
	{ 0xc2, 0, "Retired branch instructions", NULL },
	{ 0xc3, 0, "Retired mispredicted branch instructions", NULL },
	{ 0xc4, 0, "Retired taken branch instructions", NULL },
	{ 0xc5, 0, "Retired mispredicted taken branch instructions", NULL },
	{ 0xc6, 0, "Retired far control transfers", NULL },
	{ 0xc7, 0, "Retired branch resyncs", NULL },
	{ 0xc8, 0, "Retired near returns", NULL },
	{ 0xc9, 0, "Retired mispredicted near returns", NULL },
	{ 0xca, 0, "Retired mispredicted indirect brnaches", NULL },
	{ 0xcb, 0, "Retired MMX/FP instructions", NULL },
	{ 0xcc, 0, "Retired fastpath double op instructions", NULL },
	{ 0xcd, 0, "Interrupts-masked cycles", NULL },
	{ 0xce, 0, "Interrupts-masked cycles with interrupts pending", NULL },
	{ 0xcf, 0, "Interrupts taken", NULL },
	{ 0xd0, 0, "Decoder empty", NULL },
	{ 0xd1, 0, "Dispatch stalls", NULL },
	{ 0xd2, 0, "Dispatch stalls for branch abort retire", NULL },
	{ 0xd3, 0, "Dispatch stalls for serialisation", NULL },
	{ 0xd4, 0, "Dispatch stalls for segment load", NULL },
	{ 0xd5, 0, "Dispatch stalls for reoder buffer full", NULL },
	{ 0xd6, 0, "Dispatch stalls for reservation station full", NULL },
	{ 0xd7, 0, "Dispatch stalls for FPU full", NULL },
	{ 0xd8, 0, "Dispatch stalls for LS full", NULL },
	{ 0xd9, 0, "Dispatch stalls waiting for all quite", NULL },
	{ 0xda, 0, "Dispatch stalls for far transfer or resync to retire", NULL },
	{ 0xdb, 0, "FPU exceptions", NULL },
	{ 0xdc, 0, "DR0 breakpoint matches", NULL },
	{ 0xdd, 0, "DR1 breakpoint matches", NULL },
	{ 0xde, 0, "DR2 breakpoint matches", NULL },
	{ 0xdf, 0, "DR3 breakpoint matches", NULL },
	{ 0xe0, 0, "DRAM accesses", NULL },
	{ 0xe1, 0, "Memory controller page tables overflow", NULL },
	{ 0xe3, 0, "Memory controller turnarounds", NULL },
	{ 0xe4, 0, "Memory controller bypass counter saturation", NULL },
	{ 0xe5, 0, "Sized blocks", NULL },
	{ 0xe8, 0, "ECC errors", NULL },
	{ 0xe9, 0, "CPU/IO requests to memory/IO", NULL },
	{ 0xea, 0, "Cache blocks commands", NULL },
	{ 0xeb, 0, "Sized commands", NULL },
	{ 0xec, 0, "Probe responses and upstream requests", NULL },
	{ 0xee, 0, "GART events", NULL },
	{ 0xf6, 0, "HT link 0 transmit bandwidth", NULL },
	{ 0xf7, 0, "HT link 1 transmit bandwidth", NULL },
	{ 0xf8, 0, "HT link 2 transmit bandwidth", NULL },
	{ 0x0,  0, NULL, NULL }
};

#endif	/* _PCTRVAR_H_ */
