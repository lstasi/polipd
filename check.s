message:
 .ascii "Check Point\0"
 .align 4
.globl check
check:
 pushl %ebp            
 movl %esp,%ebp
 #call ___check          
 pushl $message       
 call puts
 addl $4,%esp
 xorl %eax,%eax
 movl %ebp,%esp     
 popl %ebp
 ret

 message1:
 .ascii "Check Point 1\0"
 .align 4
.globl check1
check1:
 pushl %ebp            
 movl %esp,%ebp
 #call ___check1          
 pushl $message1      
 call puts
 addl $4,%esp
 xorl %eax,%eax
 movl %ebp,%esp     
 popl %ebp
 ret

message2:
 .ascii "Check Point 2\0"
 .align 4
.globl check2
check2:
 pushl %ebp            
 movl %esp,%ebp
 #call ___check2          
 pushl $message2      
 call puts
 addl $4,%esp
 xorl %eax,%eax
 movl %ebp,%esp     
 popl %ebp
 ret

message3:
 .ascii "Check Point 3\0"
 .align 4
.globl check3
check3:
 pushl %ebp            
 movl %esp,%ebp
 #call ___check3          
 pushl $message3      
 call puts
 addl $4,%esp
 xorl %eax,%eax
 movl %ebp,%esp     
 popl %ebp
 ret
 
 message4:
 .ascii "Check Point 4\0"
 .align 4
.globl check4
check4:
 pushl %ebp            
 movl %esp,%ebp
 #call ___check4          
 pushl $message4      
 call puts
 addl $4,%esp
 xorl %eax,%eax
 movl %ebp,%esp     
 popl %ebp
 ret
