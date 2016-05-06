/*
 * Copyright (c) 2008 CO-CONV, Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <ntifs.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/lock.h>
#include <sys/lock_profile.h>
#include <sys/spinlock.h>

/* A wrapper of KSPIN_LOCK */

struct lock_class lock_class_spin = {
	"spin",		/* lc_name */
	0,		/* lc_flags */
	NULL,		/* lc_assert */
	NULL,		/* lc_ddb_show */
	NULL,		/* lc_lock */
	NULL,		/* lc_unlock */
};

void
spinlock_init(struct spinlock *lock, const char *name, const char *type, int flags)
{
	lock_init(&lock->lock_object, &lock_class_spin, name, type, flags);
	lock->locked = 0;
	KeInitializeSpinLock(&lock->spin_lock);
}

void
_spinlock_destroy(struct spinlock *lock, const char *file, int line)
{
	ULONG cpuid = 0;

	DebugPrint(DEBUG_LOCK_VERBOSE, "spinlock_destroy:%p,%d,%d%,%d@%s[%d]\n", lock, lock->locked, cpuid, RaiseDpcLevel[cpuid], file, line);

#ifdef DBG
	if (lock->locked != 0 && lock->locked != 1) {
		DebugPrint(DEBUG_LOCK_ERROR, "spinlock_destroy:%p,%d,%d%,%d@%s[%d]\n", lock, lock->locked, cpuid, RaiseDpcLevel[cpuid], file, line);
		DbgBreakPoint();
	}
#endif
	while (lock->locked > 0)
		spinlock_release(lock);


	lock_destroy(&lock->lock_object);
}

#pragma prefast(suppress:28167, "Can't annotate the logic")
void _spinlock_acquire(struct spinlock *lock, const char *file, int line)
{
	int contested = 0;
	uint64_t waittime = 0;
	ULONG cpuid = 0;
	KIRQL oldIrql;

	oldIrql = KeRaiseIrqlToDpcLevel();
	cpuid = KeGetCurrentProcessorNumber();

	DebugPrint(DEBUG_LOCK_VERBOSE, "spinlock_acquire:%p,%d%,%d@%s[%d]\n", lock, cpuid, RaiseDpcLevel[cpuid], file, line);
	lock_profile_obtain_lock_failed(&lock->lock_object, &contested, &waittime);

	KeAcquireSpinLockAtDpcLevel(&lock->spin_lock);
	if (RaiseDpcLevel[cpuid] == 0) {
		OriginalIrql[cpuid] = oldIrql;
	}
	RaiseDpcLevel[cpuid] += 1;
#ifdef DBG
	if (lock->locked != 0) {
		DebugPrint(DEBUG_LOCK_ERROR, "spinlock_acquire:%p,%d,%d%,%d@%s[%d]\n", lock, lock->locked, cpuid, RaiseDpcLevel[cpuid], file, line);
		DbgBreakPoint();
	}
#endif
	lock->locked++;

	lock_profile_obtain_lock_success(&lock->lock_object, contested, waittime,
	    file, line);
}

#pragma prefast(suppress:28167, "Can't annotate the logic")
void _spinlock_release(struct spinlock *lock, const char *file, int line)
{
	ULONG cpuid = 0;

	cpuid = KeGetCurrentProcessorNumber();

	DebugPrint(DEBUG_LOCK_VERBOSE, "spinlock_release:%p,%d%,%d@%s[%d]\n", lock, cpuid, RaiseDpcLevel[cpuid], file, line);
#ifdef DBG
	if (lock->locked < 1) {
		DebugPrint(DEBUG_LOCK_ERROR, "spinlock_release:%p,%d,%d%,%d@%s[%d]\n", lock, lock->locked, cpuid, RaiseDpcLevel[cpuid], file, line);
		DbgBreakPoint();
	}
#endif
	lock->locked--;
	RaiseDpcLevel[cpuid] -= 1;
	KeReleaseSpinLockFromDpcLevel(&lock->spin_lock);
	if (RaiseDpcLevel[cpuid] == 0) {
		KeLowerIrql(OriginalIrql[cpuid]);
	}

	lock_profile_release_lock(&lock->lock_object);
}
