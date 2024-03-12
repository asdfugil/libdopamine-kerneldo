#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define INSERT_DYLIB_PATH "/var/jb/usr/lib/libdopaminekerneldo.dylib"

static uint64_t gLabelBackup;
static uint64_t gUcredBackup;
static bool has_label = false;
static bool has_ucred = false;

int jbclient_root_steal_ucred(uint64_t ucredToSteal, uint64_t *orgUcred);
int jbclient_root_set_mac_label(uint64_t slot, uint64_t label, uint64_t *orgLabel);

void cleanup(void) {
    if (has_label) jbclient_root_set_mac_label(1, gLabelBackup, NULL);
    if (has_ucred) jbclient_root_steal_ucred(gUcredBackup, NULL);
}


void cleanup_signal(int __unused signal) {
    cleanup();

    exit(1);
}

__attribute__((constructor))void ctor(void) {
    if (geteuid() != 0) return;
    if (atexit(cleanup)) return;

    for (int i = 1; i < 17; i++) {
        signal(i, cleanup_signal);
    }

    for (int i = 24; i < 28; i++) {
        signal(i, cleanup_signal);
    }

    if (jbclient_root_set_mac_label(1, -1, &gLabelBackup) == 0) has_label = true;
    if (jbclient_root_steal_ucred(0, &gUcredBackup) == 0) has_ucred = true;
}
