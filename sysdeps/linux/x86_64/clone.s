.text
.globl __clone
.type __clone, @function
# (flags, sp, ptid, ctid, tls): syscall clone(flags, sp, ptid, ctid=r10, tls)
__clone:
        mov %rcx, %r10
        mov $56, %eax
        syscall
        test %eax, %eax
        jnz 1f
        # потомок: на стеке лежат [__thread_start][td]
        xor %ebp, %ebp
        pop %rax
        pop %rdi
        call *%rax
        hlt
1:      ret
.size __clone, . - __clone

.section .note.GNU-stack, "", @progbits
