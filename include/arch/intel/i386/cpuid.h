/*
  Copyright (c) 2018-2019 Tom Hancocks
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
 */

#if !defined(CPUID_H) && __i386__
#define CPUID_H

static inline void cpuid(uint32_t s, uint32_t *d)
{
	__asm__ __volatile__(
		"cpuid" 
		: "=a"(d[0]), "=b"(d[1]), "=c"(d[2]), "=d"(d[3])
		: "a"(s), "b"(0), "c"(0), "d"(0)
	);
}

enum i386_feature
{
	/* Low Feature Bits */
	i386_fpu = 1 << 0,		/* Floating point on chip */
	i386_vme = 1 << 1,		/* Virtual mode extension */
	i386_de = 1 << 2,		/* Debugging extension */
	i386_pse = 1 << 3,		/* Page size extension */
	i386_tsc = 1 << 4,		/* Time stamp counter */
	i386_msr = 1 << 5,		/* Model specific registers */
	i386_pae = 1 << 6,		/* Physical address extension */
	i386_mce = 1 << 7,		/* Machine check exception */
	i386_cx8 = 1 << 8,		/* ? */
	i386_apic = 1 << 9,		/* On-chip APIC */
	i386_sep = 1 << 11,		/* Fast system call */
	i386_mtrr = 1 << 12,	/* Memory type range register */
	i386_pge = 1 << 13,		/* Page global enable */
	i386_mca = 1 << 14,		/* Machine check architecture */
	i386_cmov = 1 << 15,	/* Conditional move instruction */
	i386_pat = 1 << 16,		/* Page attribute table */
	i386_pse36 = 1 << 17,	/* 36-bit page size extension */
	i386_psn = 1 << 18,		/* Processor serial number */
	i386_clfsh = 1 << 19,	/* CLFLUSH instruction supported */
	i386_ds = 1 << 21,		/* Debug store */
	i386_acpi = 1 << 22,	/* Thermal monitor and clock control */
	i386_mmx = 1 << 23,		/* MMX supported */
	i386_fxsr = 1 << 24,	/* Fast floating point save/restore */
	i386_sse = 1 << 25,		/* Streaming SIMD extensions */
	i386_sse2 = 1 << 26,	/* Streaming SIMD extensions 2 */
	i386_ss = 1 << 27,		/* Self-snoop */
	i386_htt = 1 << 28,		/* Hyper-threading technology */
	i386_tm = 1 << 29,		/* Thermal monitor */
	i386_pbe = 1 << 31,		/* Pend break enable */

	/* High Feature Bits */
	i386_sse3 = 1 << 0,		/* Streaming SIMD extensions 3 */
	i386_pclmulqdq = 1 << 1,/* PCLMULQDQ instruction */
	i386_dtes64 = 1 << 2,	/* 64-bit DS layout */
	i386_monitor = 1 << 3,	/* Monitor/mwait */
	i386_dscpl = 1 << 4,	/* Debug store CPL */
	i386_vmx = 1 << 5,		/* VMX */
	i386_smx = 1 << 6,		/* SMX */
	i386_est = 1 << 7,		/* Enhanced speedstep (GV3) */
	i386_tm2 = 1 << 8,		/* Thermal monitor 2 */
	i386_ssse3 = 1 << 9,	/* Supplemental SSE3 instructions */
	i386_cid = 1 << 10,		/* L1 context id */
	i386_seglim64 = 1 << 11,/* 64-bit segment limit checking */
	i386_fma = 1 << 12,		/* Fused multiply add support */
	i386_cx16 = 1 << 13,	/* CmpXchg16b instruction */
	i386_xtpr = 1 << 14,	/* Send task priority msgs */
	i386_pdcm = 1 << 15,	/* Performace/Debug capability MSR */
	i386_pcid = 1 << 17,	/* ASID-PCID support */
	i386_dca = 1 << 18,		/* Direct cache access */
	i386_sse4_1 = 1 << 19,	/* Streaming SIMD extensions 4.1 */
	i386_sse4_2 = 1 << 20,	/* Streaming SIMD extensions 4.2 */
	i386_x2apic = 1 << 21,	/* Extended APIC mode */
	i386_movbe = 1 << 22,	/* MOVBE instruction */
	i386_popcnt = 1 << 23,	/* POPCNT instruction */
	i386_tsctmr = 1 << 24,	/* TSC deadline timer */
	i386_aes = 1 << 25,		/* AES instructions */
	i386_xsave = 1 << 26,	/* XSAVE instructions */
	i386_osxsave = 1 << 27,	/* XGETBV/XSETBV instructions */
	i386_avx1_0 = 1 << 28,	/* AVX 1.0 instructions */
	i386_f16c = 1 << 29,	/* Float16 convert instructions */
	i386_rdrand = 1 << 30,	/* RDRAND instruction */
	i386_vmm = 1 << 31,		/* VMM Hypervisor present */
};

#endif