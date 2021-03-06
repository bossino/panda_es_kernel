/*
 *  linux/arch/arm/kernel/arch_timer.c
 *
 *  Copyright (C) 2011 ARM Ltd.
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/smp.h>
#include <linux/cpu.h>
#include <linux/jiffies.h>
#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/io.h>

#include <asm/cputype.h>
#include <asm/sched_clock.h>
#include <asm/system_info.h>

static unsigned long arch_timer_rate;
static int arch_timer_ppi;
static int arch_timer_ppi2 = -1;

static struct clock_event_device __percpu **arch_timer_evt;

extern void smp_timer_broadcast(const struct cpumask *mask);
extern struct clock_event_device percpu_clockevent;


/*
 * Architected system timer support.
 */

#define ARCH_TIMER_CTRL_ENABLE		(1 << 0)
#define ARCH_TIMER_CTRL_IT_MASK		(1 << 1)

#define ARCH_TIMER_REG_CTRL		0
#define ARCH_TIMER_REG_FREQ		1
#define ARCH_TIMER_REG_TVAL		2

static void arch_timer_reg_write(int reg, u32 val)
{
	switch (reg) {
	case ARCH_TIMER_REG_CTRL:
		asm volatile("mcr p15, 0, %0, c14, c2, 1" : : "r" (val));
		break;
	case ARCH_TIMER_REG_TVAL:
		asm volatile("mcr p15, 0, %0, c14, c2, 0" : : "r" (val));
		break;
	}

	isb();
}

static u32 arch_timer_reg_read(int reg)
{
	u32 val;

	switch (reg) {
	case ARCH_TIMER_REG_CTRL:
		asm volatile("mrc p15, 0, %0, c14, c2, 1" : "=r" (val));
		break;
	case ARCH_TIMER_REG_FREQ:
		asm volatile("mrc p15, 0, %0, c14, c0, 0" : "=r" (val));
		break;
	case ARCH_TIMER_REG_TVAL:
		asm volatile("mrc p15, 0, %0, c14, c2, 0" : "=r" (val));
		break;
	default:
		BUG();
	}

	return val;
}

static irqreturn_t arch_timer_handler(int irq, void *dev_id)
{
	struct clock_event_device *evt = *(struct clock_event_device **)dev_id;
	unsigned long ctrl;

	ctrl = arch_timer_reg_read(ARCH_TIMER_REG_CTRL);
	if (ctrl & 0x4) {
		ctrl |= ARCH_TIMER_CTRL_IT_MASK;
		arch_timer_reg_write(ARCH_TIMER_REG_CTRL, ctrl);
		evt->event_handler(evt);
		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

static void arch_timer_stop(void)
{
	unsigned long ctrl;

	ctrl = arch_timer_reg_read(ARCH_TIMER_REG_CTRL);
	ctrl &= ~ARCH_TIMER_CTRL_ENABLE;
	arch_timer_reg_write(ARCH_TIMER_REG_CTRL, ctrl);
}

static void arch_timer_set_mode(enum clock_event_mode mode,
				struct clock_event_device *clk)
{
	switch (mode) {
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
		arch_timer_stop();
		break;
	default:
		break;
	}
}

static int arch_timer_set_next_event(unsigned long evt,
				     struct clock_event_device *unused)
{
	unsigned long ctrl;

	ctrl = arch_timer_reg_read(ARCH_TIMER_REG_CTRL);
	ctrl |= ARCH_TIMER_CTRL_ENABLE;
	ctrl &= ~ARCH_TIMER_CTRL_IT_MASK;

	arch_timer_reg_write(ARCH_TIMER_REG_TVAL, evt);
	arch_timer_reg_write(ARCH_TIMER_REG_CTRL, ctrl);

	return 0;
}

static void __cpuinit arch_timer_setup(void *data)
{
	struct clock_event_device *clk = data;
	struct clock_event_device **this_cpu_clk;

	/* Be safe... */
	arch_timer_stop();
	clk->features = CLOCK_EVT_FEAT_ONESHOT;
	clk->name = "arch_sys_timer";
	clk->rating = 450;
	clk->set_mode = arch_timer_set_mode;
	clk->set_next_event = arch_timer_set_next_event;
	clk->irq = arch_timer_ppi;
	clk->cpumask = cpumask_of(smp_processor_id());

	clockevents_config_and_register(clk, arch_timer_rate,
					0xf, 0x7fffffff);
	this_cpu_clk = __this_cpu_ptr(arch_timer_evt);
	*this_cpu_clk = clk;
	enable_percpu_irq(clk->irq, 0);
}

/* Is the optional system timer available? */
static int local_timer_is_architected(void)
{
	return (cpu_architecture() >= CPU_ARCH_ARMv7) &&
	       ((read_cpuid_ext(CPUID_EXT_PFR1) >> 16) & 0xf) == 1;
}

static int arch_timer_available(void)
{
	unsigned long freq;

	if (!local_timer_is_architected())
		return -ENXIO;

	if (arch_timer_rate == 0) {
		arch_timer_reg_write(ARCH_TIMER_REG_CTRL, 0);
		freq = arch_timer_reg_read(ARCH_TIMER_REG_FREQ);

		/* Check the timer frequency. */
		if (freq == 0) {
			pr_warn("Architected timer frequency not available\n");
			return -EINVAL;
		}

		arch_timer_rate = freq;
		pr_info("Architected local timer running at %lu.%02luMHz.\n",
			arch_timer_rate / 1000000, (arch_timer_rate % 100000) / 100);
	}

	return 0;
}

static inline cycle_t arch_counter_get_cntpct(void)
{
	u32 cvall1, cvalh1, cvall2, cvalh2;

	asm volatile("mrrc p15, 0, %0, %1, c14" : "=r" (cvall1), "=r" (cvalh1));

	/* Workaround for TM bug 4718 */
	if (read_cpuid_id() & 0x410FC0F0) {
		asm volatile("mrrc p15, 0, %0, %1, c14" : "=r" (cvall2), "=r" (cvalh2));
		if (cvalh1 != cvalh2)
			asm volatile("mrrc p15, 0, %0, %1, c14" : "=r" (cvall2), "=r" (cvalh2));

		return ((u64) cvalh2 << 32) | cvall2;
	}

	return ((u64) cvalh1 << 32) | cvall1;
}

static inline cycle_t arch_counter_get_cntvct(void)
{
	u32 cvall1, cvalh1, cvall2, cvalh2;

	asm volatile("mrrc p15, 1, %0, %1, c14" : "=r" (cvall1), "=r" (cvalh1));

	/* Workaround for TM bug 4718. Valid only for A15 R0P0 revision */
	if (read_cpuid_id() & 0x410FC0F0) {
		asm volatile("mrrc p15, 1, %0, %1, c14" : "=r" (cvall2), "=r" (cvalh2));
		if (cvalh1 != cvalh2)
			asm volatile("mrrc p15, 0, %0, %1, c14" : "=r" (cvall2), "=r" (cvalh2));

		return ((u64) cvalh2 << 32) | cvall2;
	}

	return ((u64) cvalh1 << 32) | cvall1;
}

static u32 notrace arch_counter_get_cntvct32(void)
{
	cycle_t cntvct = arch_counter_get_cntvct();

	/*
	 * The sched_clock infrastructure only knows about counters
	 * with at most 32bits. Forget about the upper 24 bits for the
	 * time being...
	 */
	return (u32)(cntvct & (u32)~0);
}

static cycle_t arch_counter_read(struct clocksource *cs)
{
	return arch_counter_get_cntpct();
}

static struct clocksource clocksource_counter = {
	.name	= "arch_sys_counter",
	.rating	= 400,
	.read	= arch_counter_read,
	.mask	= CLOCKSOURCE_MASK(56),
	.flags	= CLOCK_SOURCE_IS_CONTINUOUS,
};

static void __cpuinit arch_timer_teardown(void *data)
{
	struct clock_event_device *clk = data;
	pr_debug("arch_timer_teardown disable IRQ%d cpu #%d\n",
		 clk->irq, smp_processor_id());
	disable_percpu_irq(clk->irq);
	if (arch_timer_ppi2 >= 0)
		disable_percpu_irq(arch_timer_ppi2);
	arch_timer_set_mode(CLOCK_EVT_MODE_UNUSED, clk);
}

static int __cpuinit arch_timer_cpu_notify(struct notifier_block *self,
					   unsigned long action, void *data)
{
	int cpu = (int)data;
	struct clock_event_device *clk = &__get_cpu_var(percpu_clockevent);

	switch (action) {
	case CPU_STARTING:
	case CPU_STARTING_FROZEN:
		smp_call_function_single(cpu, arch_timer_setup, clk, 1);
		break;

	case CPU_DOWN_PREPARE:
	case CPU_DOWN_PREPARE_FROZEN:
		smp_call_function_single(cpu, arch_timer_teardown, clk, 1);
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block __cpuinitdata arch_timer_cpu_nb = {
	.notifier_call = arch_timer_cpu_notify,
};

int __init arch_timer_register(struct resource *res, int res_nr)
{
	int err;

	if (!res_nr || res[0].start < 0 || !(res[0].flags & IORESOURCE_IRQ))
		return -EINVAL;

	if ((res_nr > 1) && (res[1].flags & IORESOURCE_MEM))
		arch_timer_rate = res[1].start;

	err = arch_timer_available();
	if (err)
		return err;

	arch_timer_evt = alloc_percpu(struct clock_event_device *);
	if (!arch_timer_evt)
		return -ENOMEM;

	arch_timer_ppi = res[0].start;
	if (res_nr > 1 && (res[1].flags & IORESOURCE_IRQ))
		arch_timer_ppi2 = res[1].start;

	clocksource_register_hz(&clocksource_counter, arch_timer_rate);

	err = request_percpu_irq(arch_timer_ppi, arch_timer_handler,
				 "arch_timer", arch_timer_evt);
	if (err) {
		pr_err("arch_timer: can't register interrupt %d (%d)\n",
		       arch_timer_ppi, err);
		goto out_free;
	}

	if (arch_timer_ppi2 >= 0) {
		err = request_percpu_irq(arch_timer_ppi2, arch_timer_handler,
					 "arch_timer", arch_timer_evt);
		if (err) {
			pr_err("arch_timer: can't register interrupt %d (%d)\n",
			       arch_timer_ppi2, err);
			goto out_free_irq;
		}
	}

	/* Immediately configure the timer on the boot CPU */
	arch_timer_setup(&__get_cpu_var(percpu_clockevent));

	register_cpu_notifier(&arch_timer_cpu_nb);

	return 0;

out_free_irq:
	free_percpu_irq(arch_timer_ppi, arch_timer_evt);

out_free:
	free_percpu(arch_timer_evt);

	return err;
}

int arch_timer_sched_clock_init(void)
{
	int err;

	err = arch_timer_available();
	if (err)
		return err;

	setup_sched_clock(arch_counter_get_cntvct32, 32, arch_timer_rate);
	return 0;
}
