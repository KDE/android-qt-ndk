/*
 * Copyright 2012, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _FCNTL_PORTABLE_H_
#define _FCNTL_PORTABLE_H_

/* Derived from development/ndk/platforms/android-3/arch-arm/include/asm/fcntl.h */
/* NB x86 does not have these and only uses the generic definitions */
#define O_DIRECTORY_PORTABLE    040000
#define O_NOFOLLOW_PORTABLE     0100000
#define O_DIRECT_PORTABLE       0200000
#define O_LARGEFILE_PORTABLE    0400000

/* Derived from development/ndk/platforms/android-3/include/asm-generic/fcntl.h */
#define O_ACCMODE_PORTABLE  00000003
#define O_RDONLY_PORTABLE   00000000
#define O_WRONLY_PORTABLE   00000001
#define O_RDWR_PORTABLE     00000002
#ifndef O_CREAT_PORTABLE
#define O_CREAT_PORTABLE    00000100
#endif
#ifndef O_EXCL_PORTABLE
#define O_EXCL_PORTABLE     00000200
#endif
#ifndef O_NOCTTY_PORTABLE
#define O_NOCTTY_PORTABLE   00000400
#endif
#ifndef O_TRUNC_PORTABLE
#define O_TRUNC_PORTABLE    00001000
#endif
#ifndef O_APPEND_PORTABLE
#define O_APPEND_PORTABLE   00002000
#endif
#ifndef O_NONBLOCK_PORTABLE
#define O_NONBLOCK_PORTABLE 00004000
#endif
#ifndef O_SYNC_PORTABLE
#define O_SYNC_PORTABLE     00010000
#endif
#ifndef FASYNC_PORTABLE
#define FASYNC_PORTABLE     00020000
#endif
#ifndef O_DIRECT_PORTABLE
#define O_DIRECT_PORTABLE   00040000
#endif
#ifndef O_LARGEFILE_PORTABLE
#define O_LARGEFILE_PORTABLE    00100000
#endif
#ifndef O_DIRECTORY_PORTABLE
#define O_DIRECTORY_PORTABLE    00200000
#endif
#ifndef O_NOFOLLOW_PORTABLE
#define O_NOFOLLOW_PORTABLE 00400000
#endif
#ifndef O_NOATIME_PORTABLE
#define O_NOATIME_PORTABLE  01000000
#endif
#ifndef O_NDELAY_PORTABLE
#define O_NDELAY_PORTABLE   O_NONBLOCK_PORTABLE
#endif

struct flock64_portable {
   short l_type;
   short l_whence;
   unsigned char __padding[4];
   loff_t l_start;
   loff_t l_len;
   pid_t l_pid;
   __ARCH_FLOCK64_PAD
};

/*
The X86 Version is

struct flock64 {
   short l_type;
   short l_whence;
   loff_t l_start;
   loff_t l_len;
   pid_t l_pid;
   __ARCH_FLOCK64_PAD
};
*/

#endif /* _FCNTL_PORTABLE_H */
