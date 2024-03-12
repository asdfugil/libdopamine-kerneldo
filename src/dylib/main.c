#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <spawn.h>
#include <dlfcn.h>
#include <util.h>
#include <substrate.h>

#define INSERT_DYLIB_PATH "/var/jb/usr/lib/libdopaminekerneldo.dylib"

static uint64_t gLabelBackup;
static uint64_t gUcredBackup;
static bool has_label = false;
static bool has_ucred = false;
void restore_ucred(void);
void steal_ucred(void);

int jbclient_root_steal_ucred(uint64_t ucredToSteal, uint64_t *orgUcred);
int jbclient_root_set_mac_label(uint64_t slot, uint64_t label, uint64_t *orgLabel);

int (*posix_spawn_orig)(pid_t * __restrict pid, const char * __restrict path,
    const posix_spawn_file_actions_t * actions,
    const posix_spawnattr_t * __restrict attr,
    char *const __argv[__restrict],
    char *const __envp[__restrict]);

int (*fork_orig)(void);
int (*vfork_orig)(void);
int (*forkpty_orig)(void);
int (*daemon_orig)(int nochdir, int noclose);

int fork_hook_hook(void) {
    restore_ucred();
    int retval = fork_orig();
    steal_ucred();
    return retval;
}

int vfork_hook_hook(void) {
    restore_ucred();
    int retval = vfork_orig();
    steal_ucred();
    return retval;
}

int forkpty_hook_hook(void) {
    restore_ucred();
    int retval = forkpty_orig();
    steal_ucred();
    return retval;
}

int daemon_hook_hook(int nochdir, int noclose) {
    restore_ucred();
    int retval = daemon_orig(nochdir, noclose);
    steal_ucred();
    return retval;
}

int (*spawn_hook_common_orig)(pid_t *restrict pid, const char *restrict path,
                                           const posix_spawn_file_actions_t *restrict file_actions,
                                           const posix_spawnattr_t *restrict attrp,
                                           char *const argv[restrict],
                                           char *const envp[restrict],
                                           void *orig,
                                           int (*trust_binary)(const char *),
                                           int (*set_process_debugged)(uint64_t pid, bool fullyDebugged));

int spawn_hook_common_hook(pid_t *restrict pid, const char *restrict path,
                                           const posix_spawn_file_actions_t *restrict file_actions,
                                           const posix_spawnattr_t *restrict attrp,
                                           char *const argv[restrict],
                                           char *const envp[restrict],
                                           void *orig,
                                           int (*trust_binary)(const char *),
                                           int (*set_process_debugged)(uint64_t pid, bool fullyDebugged)) {
    int retval = 0;
    fprintf(stderr, "meow!\n");
    if (!attrp) goto finish;
    short pflags = 0;
    posix_spawnattr_getflags(attrp, &pflags);
    if ((pflags & POSIX_SPAWN_SETEXEC) == 0) goto finish;
    fprintf(stderr, "Restoring ucred!\n");
    sleep(1);
    restore_ucred();
finish:
    retval = spawn_hook_common_orig(pid, path, file_actions, attrp, argv, envp, orig, trust_binary, set_process_debugged);
    steal_ucred();
    return retval;
}


void restore_ucred(void) {
    if (has_label && jbclient_root_set_mac_label(1, gLabelBackup, NULL) == 0) {
        has_label = false;
    }
    if (has_ucred && jbclient_root_steal_ucred(gUcredBackup, NULL) == 0) {
        has_ucred = false;
    }
}

void steal_ucred(void) {
    if ((!has_label) && jbclient_root_set_mac_label(1, -1, &gLabelBackup) == 0) has_label = true;
    if ((!has_ucred) && jbclient_root_steal_ucred(0, &gUcredBackup) == 0) has_ucred = true;
}


void cleanup_signal(int __unused signal) {
    restore_ucred();

    exit(1);
}

__attribute__((constructor))void ctor(void) {
    if (geteuid() != 0) return;
    if (atexit(restore_ucred)) return;

    for (int i = 1; i < 17; i++) {
        signal(i, cleanup_signal);
    }

    for (int i = 24; i < 28; i++) {
        signal(i, cleanup_signal);
    }

    void* systemhook = dlopen("@rpath/systemhook.dylib", RTLD_LAZY);
    if (!systemhook) return;
    void* spawn_hook_common = dlsym(systemhook, "spawn_hook_common");
    if (!spawn_hook_common) return; 

    MSImageRef libsystem_c = MSGetImageByName("/usr/lib/system/libsystem_c.dylib");
    if (!libsystem_c) return;
    void *fork_hook, *forkpty_hook, *vfork_hook, *daemon_hook;

    fork_hook = MSFindSymbol(libsystem_c, "_fork");
    forkpty_hook = MSFindSymbol(libsystem_c, "_forkpty");
    vfork_hook = MSFindSymbol(libsystem_c, "_vfork");
    daemon_hook = MSFindSymbol(libsystem_c, "_daemon");

    if (!fork_hook || !forkpty_hook || !vfork_hook || !daemon_hook) return;

    MSHookFunction(spawn_hook_common, (void*)spawn_hook_common_hook, (void**)&spawn_hook_common_orig);

    MSHookFunction(fork_hook, (void*)fork_hook_hook, (void**)&fork_orig);
    MSHookFunction(forkpty_hook, (void*)forkpty_hook_hook, (void**)&forkpty_orig);
    MSHookFunction(vfork_hook, (void*)vfork_hook_hook, (void**)&vfork_orig);
    MSHookFunction(daemon_hook, (void*)daemon_hook_hook, (void**)&daemon_orig);

    steal_ucred();
}
