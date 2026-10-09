.text
.globl __unmapself
.type __unmapself, @function
# munmap(base, size) и выход текущего потока без возврата
__unmapself:
        mov $11, %eax
        syscall
        mov $60, %eax
        xor %edi, %edi
        syscall
        hlt
.size __unmapself, . - __unmapself

.section .note.GNU-stack, "", @progbits
