
.text
.global spin_lock
.global spin_unlock

spin_lock:
	xorl %ecx, %ecx
	incl %ecx
	spin_lock_retry:
		rep; nop
		xorl %eax, %eax
		lock cmpxchgl %ecx, (%rdi)
		jnz spin_lock_retry
	ret

spin_unlock:
	movl $0, (%rdi)
	ret	
