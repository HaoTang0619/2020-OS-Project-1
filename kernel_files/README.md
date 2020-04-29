### Reference
https://fenghe.us/compile-5-4-2-linux-kernel-with-a-new-syscall-for-ubuntu-18-04-in-vmware/

### system call function:
At the end of `sys.c` (line 2653 ~ 2666)

```powershell
#include <linux/timekeeping32.h>

SYSCALL_DEFINE0(os_pj1_time){
	static const long BASE = 1000000000;
	struct timespec t;
    	getnstimeofday(&t);
    	return t.tv_sec * BASE + t.tv_nsec;
}

SYSCALL_DEFINE3(os_pj1_dmesg, int, pid, long, start_time, long, end_time){
	static const long BASE = 1000000000;
	printk(KERN_INFO "[Project1] %d %ld.%09ld %ld.%09ld", pid, start_time / BASE, start_time % BASE, end_time / BASE, end_time % BASE);
	return 0;
}
```
