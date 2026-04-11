.globl uswtch
uswtch:
    movl 4(%esp), %eax    # &old_esp
    movl 8(%esp), %edx    # new_esp


    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi

   
    movl %esp, (%eax)

    
    movl %edx, %esp

    
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp

    ret