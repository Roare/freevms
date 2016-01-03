#ifndef __LINUX_SMP_H
#define __LINUX_SMP_H

/*
 *  Generic SMP support
 *      Alan Cox. <alan@redhat.com>
 */

#include <linux/config.h>

#ifdef CONFIG_SMP

#include <linux/kernel.h>
#include <asm/smp.h>

/*
 * main cross-CPU interfaces, handles INIT, TLB flush, STOP, etc.
 * (defined in asm header):
 */

/*
 * stops all CPUs but the current one:
 */
extern void smp_send_stop(void);

/*
 * sends a 'reschedule' event to another CPU:
 */
#if 0
extern void FASTCALL(smp_send_reschedule(int cpu));
#else
extern void smp_send_reschedule(int cpu);
#endif


/*
 * Boot processor call to load the other CPU's
 */
extern void smp_boot_cpus(void);

/*
 * Processor call in. Must hold processors until ..
 */
extern void smp_callin(void);

/*
 * Multiprocessors may now schedule
 */
extern void smp_commence(void);

/*
 * Call a function on all other processors
 */
extern int smp_call_function (void (*func) (void *info), void *info,
                              int retry, int wait);

/*
 * True once the per process idle is forked
 */
extern int smp_threads_ready;

extern int smp_num_cpus;

#else

/*
 *  These macros fold the SMP functionality into a single CPU system
 */

#define smp_num_cpus                1
#define smp_processor_id()          0
#define hard_smp_processor_id()         0
#define smp_threads_ready           1
#define kernel_lock()
#define cpu_logical_map(cpu)            0
#define cpu_number_map(cpu)         0
#define smp_call_function(func,info,retry,wait) ({ 0; })
#define cpu_online_map              1

#endif
#endif
