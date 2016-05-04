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

#ifndef _SYS_SPINLOCK_H_
#define _SYS_SPINLOCK_H_

/* A wrapper of KSPIN_LOCK */
struct spinlock {
	struct lock_object	lock_object;
	KSPIN_LOCK		spin_lock;
	int			locked;
};

extern KSPIN_LOCK Giant;

#ifdef _KERNEL

#define SPINLOCK_NOPROFILE	0x00000001

void spinlock_init(struct spinlock *, const char *, const char *, int);
void _spinlock_destroy(struct spinlock *, const char *, int);

#pragma prefast(suppress:28167, "Can't annotate the logic")
void _spinlock_acquire(struct spinlock *, const char *, int);

#pragma prefast(suppress:28167, "Can't annotate the logic")
void _spinlock_release(struct spinlock *, const char *, int);

#define spinlock_destroy(lock) \
    _spinlock_destroy((lock), __FILE__, __LINE__)

#define spinlock_acquire(lock) \
    _spinlock_acquire((lock), __FILE__, __LINE__)

#define spinlock_release(lock) \
    _spinlock_release((lock), __FILE__, __LINE__)
#endif
#endif
