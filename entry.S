# entry.S
#

.equ U_EDI, 4
.equ U_ESI, 8
.equ U_EBP, 12
.equ U_ESP, 16
.equ U_EBX, 20
.equ U_EDX, 24
.equ U_ECX, 28
.equ U_EAX, 32
.equ U_EFLAGS, 36
.equ U_EIP, 40

.equ K_EDI, 44
.equ K_ESI, 48
.equ K_EBP, 52
.equ K_ESP, 56
.equ K_EBX, 60
.equ K_EDX, 64
.equ K_ECX, 68
.equ K_EAX, 72
.equ K_EFLAGS, 76
.equ K_EIP, 80

	.data
	.text
	.code32

	.globl	kernel_entry
	.globl  restore_user_stack
kernel_entry:
		after_call:
# push esp
		push %eax
		movl %esp,%eax
		movl current_running,%esp
		addl $U_EIP,%esp
# push user stack registers onto stack, order is EAX,EBX,ECX,EDX,ESP,EBP,ESI,EDI
		pushf
		pushal
# restore value of esp
		movl %eax,%esp
		movl current_running,%eax
		popl %ebx
		movl %ebx,U_EAX(%eax)
# Save return state to eip of PCB
		popl U_EIP(%eax)
# Esp back to where it was at start, Get system call number argument from stack
		popl %ebx
		pushl %ebx
		movl %esp,U_ESP(%eax)
		d:
# Restore kernel stack

		movl K_ESP(%eax),%esp # esp
		movl K_EBP(%eax),%ebp # ebp

		pushl 	%ebx
    call    kernel_entry_helper
    popl    %eax

restore_user_stack:
		movl current_running,%eax
		movl U_ECX(%eax),%ecx # ecx
		movl U_EDX(%eax),%edx # edx
		movl U_ESI(%eax),%esi # esi
		movl U_EDI(%eax),%edi # edi

		movl U_ESP(%eax),%esp # esp
		movl U_EBP(%eax),%ebp # ebp

		movl U_EFLAGS(%eax),%ebx # eflags
		push %ebx
		popf
# eip -  restore program counter
		movl U_EIP(%eax),%ebx
		push %ebx
		movl U_EBX(%eax),%ebx # ebx
		movl U_EAX(%eax),%eax # eax
		before_call:
    ret

	.globl	scheduler_entry
scheduler_entry:


# push esp
		push %eax
		movl %esp,%eax
		movl current_running,%esp
		addl $K_EIP,%esp
# push user stack registers onto stack, order is EAX,EBX,ECX,EDX,ESP,EBP,ESI,EDI
		pushf
		pushal
# restore value of esp
		movl %eax,%esp
		movl current_running,%eax
		popl %ebx
		movl %ebx,K_EAX(%eax)
# Save return state to eip of PCB
		popl K_EIP(%eax)
# Esp back to where it was at start, Get system call number argument from stack
		popl %ebx
		pushl %ebx
		movl %esp,K_ESP(%eax)

# scheduler has no arguments
		call scheduler


# After scheduler is called restore kernel stack of new process
		movl current_running,%eax
		movl K_ECX(%eax),%ecx # ecx
		movl K_EDX(%eax),%edx # edx
		movl K_ESI(%eax),%esi # esi
		movl K_EDI(%eax),%edi # edi
		movl K_ESP(%eax),%esp # esp
		movl K_EBP(%eax),%ebp # ebp
# flags must be restored with popf
		movl K_EFLAGS(%eax),%ebx
		push %ebx
		popf
# program counter (eip) must be restoded as a return statement
		movl K_EIP(%eax),%ebx
		push %ebx
# restore ebx and eax
		movl K_EBX(%eax),%ebx # ebx
		movl K_EAX(%eax),%eax # eax

	ret
