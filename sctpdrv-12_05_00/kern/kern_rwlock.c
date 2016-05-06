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
#include <ndis.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/lock.h>
#include <sys/lock_profile.h>
#include <sys/rwlock.h>

/* A wrapper of NDIS_RW_LOCK */

struct lock_class lock_class_rwlock = {
	"rwlock",	/* lc_name */
	0,		/* lc_flags */
	NULL,		/* lc_assert */
	NULL,		/* lc_ddb_show */
	NULL,		/* lc_lock */
	NULL,		/* lc_unlock */
};

void
rwlock_init(struct rwlock *lock, const char *name, const char *type, int flags)
{
	lock_init(&lock->lock_object, &lock_class_rwlock, name, type, flags);
	lock->locked = 0;
	NdisInitializeReadWriteLock(&lock->rw_lock);
	RtlZeroMemory(&lock->rw_lock_state, sizeof(lock->rw_lock_state));
}

void
_rwlock_destroy(struct rwlock *lock, const char *file, int line)
{
	ULONG cpuid = 0;


	DebugPrint(DEBUG_LOCK_VERBOSE, "rwlock_destroy:%p,%d,%d%,%d@%s[%d]\n", lock, lock->locked, cpuid, RaiseDpcLevel[cpuid], file, line);
#ifdef DBG
	if (lock->locked < 0) {
		DebugPrint(DEBUG_LOCK_ERROR, "rwlock_destroy:%p,%d,%d%,%d@%s[%d]\n", lock, lock->locked, cpuid, RaiseDpcLevel[cpuid], file, line);
		DbgBreakPoint();
	}
#endif

	while (lock->locked > 0)
		rwlock_release(lock);


	lock_destroy(&lock->lock_object);
}

#pragma prefast(suppress:28167, "Can't annotate the logic")
void _rwlock_acquire(struct rwlock *lock, int how, const char *file, int line)
{
	int contested = 0;
	uint64_t waittime = 0;
	ULONG cpuid = 0;
	KIRQL oldIrql;

	oldIrql = KeRaiseIrqlToDpcLevel();
	cpuid = KeGetCurrentProcessorNumber();

	DebugPrint(DEBUG_LOCK_VERBOSE, "rwlock_acquire:%p,%d%,%d@%s[%d]\n", lock, cpuid, RaiseDpcLevel[cpuid], file, line);
	lock_profile_obtain_lock_failed(&lock->lock_object, &contested, &waittime);

	NdisAcquireReadWriteLock(&lock->rw_lock, how, &lock->rw_lock_state[cpuid]);

	if (RaiseDpcLevel[cpuid] == 0) {
		OriginalIrql[cpuid] = oldIrql;
	}
	RaiseDpcLevel[cpuid] += 1;
#ifdef DBG
	if (lock->locked < 0) {
		DebugPrint(DEBUG_LOCK_ERROR, "rwlock_acquire:%p,%d,%d%,%d@%s[%d]\n", lock, lock->locked, cpuid, RaiseDpcLevel[cpuid], file, line);
		DbgBreakPoint();
	}
#endif
	atomic_add_int(&lock->locked, 1);

	lock_profile_obtain_lock_success(&lock->lock_object, contested, waittime,
	    file, line);
}

#pragma prefast(suppress:28167, "Can't annotate the logic")
void _rwlock_release(struct rwlock *lock, const char *file, int line)
{
	ULONG cpuid = 0;

	cpuid = KeGetCurrentProcessorNumber();

	DebugPrint(DEBUG_LOCK_VERBOSE, "rwlock_release:%p,%d%,%d@%s[%d]\n", lock, cpuid, RaiseDpcLevel[cpuid], file, line);
#ifdef DBG
	if (lock->locked < 1) {
		DebugPrint(DEBUG_LOCK_ERROR, "rwlock_release:%p,%d,%d%,%d@%s[%d]\n", lock, lock->locked, cpuid, RaiseDpcLevel[cpuid], file, line);
		DbgBreakPoint();
	}
#endif
	atomic_add_int(&lock->locked, -1);
	RaiseDpcLevel[cpuid] -= 1;
	
	NdisReleaseReadWriteLock(&lock->rw_lock, &lock->rw_lock_state[cpuid]);
	
	if (RaiseDpcLevel[cpuid] == 0) {
		KeLowerIrql(OriginalIrql[cpuid]);
	}

	lock_profile_release_lock(&lock->lock_object);
}
