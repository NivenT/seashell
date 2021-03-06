#ifndef SIGNALS_H_INCLUDED
#define SIGNALS_H_INCLUDED

#include <signal.h>

#include "defs.h"

extern bool install_signal_handlers();

extern sigset_t get_sig_singleton(int sig);
extern sigset_t get_sig_pair(int sig1, int sig2);
extern sigset_t get_sig_full();
extern sigset_t block_sig(int sig);
extern sigset_t block_sig2(int sig1, int sig2);
// Doesn't unblock sig if it was blocked before companion call to block_sig
extern void unblock_sig(int sig, sigset_t prevmask);
extern void unblock_sig2(int sig1, int sig2, sigset_t prevmask);

#endif // SIGNALS_H_INCLUDED
