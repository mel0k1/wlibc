.text
.globl __restore_rt
.type __restore_rt, @function
# точка входа после обработчика сигнала: rt_sigreturn восстанавливает контекст
__restore_rt:
        mov $15, %eax
        syscall
.size __restore_rt, . - __restore_rt

.section .note.GNU-stack, "", @progbits
