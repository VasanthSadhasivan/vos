global HandleGeneric
global interrupts_enabled

extern current_page_table_base
extern kernel_page_table_base
extern isr

%macro pushaq 0
	push rax
	push rcx
	push rdx
	push rbx
	push rbp
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	mov r15, ds
	push r15
	mov r15, es
	push r15
	mov r15, fs
	push r15
	mov r15, gs
	push r15
%endmacro

%macro popaq 0
	pop r15
	mov gs, r15
	pop r15
	mov fs, r15
	pop r15
	mov es, r15
	pop r15
	mov ds, r15
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rbp
	pop rbx
	pop rdx
	pop rcx
	pop rax
%endmacro

%macro HandleWithErrorCode 1
global HandleWithErrorCode%1
HandleWithErrorCode%1:
;	add rsp, 0x8
;	pushaq
;	mov rdi, %1
;	call isr
;	popaq
;	mov [originalrdi], rdi
;	mov rdi, [current_page_table_base]
;	cmp rdi, 0
;	je .iret%1
;	mov cr3, rdi
;.iret%1:
;	mov rdi, [originalrdi]
;	iretq
;check if current_page_table_base == 0, if it is, jmp
	add rsp, 0x8
	mov [originalr14], r14
	mov r14, [current_page_table_base]
	cmp r14, 0x00
	je .no_user%1
	push r14 ;top of stack is original page table base
	mov r14, [kernel_page_table_base]
	mov [current_page_table_base], r14
	mov cr3, r14
	mov r14, [originalr14]
	pushaq
	mov rdi, %1
	call isr
	popaq
	mov [originalr14], r14
	pop r14 ;grab original page table base
	mov [current_page_table_base], r14
	mov cr3, r14
	mov r14, [originalr14]
	iretq
.no_user%1:
	mov r14, [originalr14]
	pushaq
	mov rdi, %1
	call isr
	popaq
	iretq
%endmacro

%macro Handle 1
global Handle%1
Handle%1:
	pushaq
	mov rdi, %1
	call isr
	popaq
	iretq
%endmacro

%macro HandleHardware 1
global HandleHardware%1
HandleHardware%1:
	mov [originalr14], r14
	mov r14, [current_page_table_base]
	cmp r14, 0x00
	je .no_user%1
	push r14 ;top of stack is original page table base
	mov r14, [kernel_page_table_base]
	mov [current_page_table_base], r14
	mov cr3, r14
	mov r14, [originalr14]
	pushaq
	mov rdi, %1
	call isr
	popaq
	mov [originalr14], r14
	pop r14 ;grab original page table base
	mov [current_page_table_base], r14
	mov cr3, r14
	mov r14, [originalr14]
	iretq
.no_user%1:
	mov r14, [originalr14]
	pushaq
	mov rdi, %1
	call isr
	popaq
	iretq
%endmacro

section .text
bits 64
interrupts_enabled:
	push rcx
	pushfq
	pop rcx
	and rcx, 0b1000000000
	jz .disabled
.enabled:
	mov rax, 0xFF
	pop rcx
	ret
.disabled:
	mov rax, 0x00
	pop rcx
	ret

HandleGeneric:
	pushaq
	mov rdi, 256
	call isr
	popaq
	iretq

HandleWithErrorCode 8
HandleWithErrorCode 10
HandleWithErrorCode 11
HandleWithErrorCode 12
HandleWithErrorCode 13

global HandleWithErrorCode14
HandleWithErrorCode14:
	add rsp, 0x8
	pushaq
	mov rdi, 14
	call isr
	popaq
	mov [originalrdi], rdi
	mov rdi, [current_page_table_base]
	cmp rdi, 0
	je .iret14
	mov cr3, rdi
.iret14:
	mov rdi, [originalrdi]
	iretq

HandleWithErrorCode 17
HandleWithErrorCode 21
HandleWithErrorCode 29
HandleWithErrorCode 30

HandleHardware 0
HandleHardware 1
HandleHardware 2
HandleHardware 3
HandleHardware 4
HandleHardware 5
HandleHardware 6
HandleHardware 7
HandleHardware 9
HandleHardware 15
HandleHardware 16
HandleHardware 18
HandleHardware 19
HandleHardware 20
HandleHardware 22
HandleHardware 23
HandleHardware 24
HandleHardware 25
HandleHardware 26
HandleHardware 27
HandleHardware 28
HandleHardware 31
HandleHardware 32
HandleHardware 33
HandleHardware 34
HandleHardware 35
HandleHardware 36
HandleHardware 37
HandleHardware 38
HandleHardware 39
HandleHardware 40
HandleHardware 41
HandleHardware 42
HandleHardware 43
HandleHardware 44
HandleHardware 45
HandleHardware 46
HandleHardware 47
Handle 48
Handle 49
Handle 50
Handle 51
Handle 52
Handle 53
Handle 54
Handle 55
Handle 56
Handle 57
Handle 58
Handle 59
Handle 60
Handle 61
Handle 62
Handle 63
Handle 64
Handle 65
Handle 66
Handle 67
Handle 68
Handle 69
Handle 70
Handle 71
Handle 72
Handle 73
Handle 74
Handle 75
Handle 76
Handle 77
Handle 78
Handle 79
Handle 80
Handle 81
Handle 82
Handle 83
Handle 84
Handle 85
Handle 86
Handle 87
Handle 88
Handle 89
Handle 90
Handle 91
Handle 92
Handle 93
Handle 94
Handle 95
Handle 96
Handle 97
Handle 98
Handle 99
Handle 100
Handle 101
Handle 102
Handle 103
Handle 104
Handle 105
Handle 106
Handle 107
Handle 108
Handle 109
Handle 110
Handle 111
Handle 112
Handle 113
Handle 114
Handle 115
Handle 116
Handle 117
Handle 118
Handle 119
Handle 120
Handle 121
Handle 122
Handle 123
Handle 124
Handle 125
Handle 126
Handle 127
global Handle128
Handle128:
	mov [originalr14], r14
	mov r14, [current_page_table_base]
	mov [original_page_table_base], r14
	push r14
	mov r14, [kernel_page_table_base]
	mov [current_page_table_base], r14
	mov cr3, r14
	mov r14, [originalr14]
	mov [originalr15], r15
	mov r15, [original_page_table_base]
	pushaq
	mov rdi, 128
	call isr
	popaq
	mov r15, [originalr15]
	mov [originalr14], r14
	mov r14, [current_page_table_base]
	cmp r14, [kernel_page_table_base]
	je still_kernel
	add rsp, 0x08
	push r14
	;mov [original_page_table_base], r14
still_kernel:
	pop r14	
	;mov r14, [original_page_table_base]
	mov cr3, r14
	mov [current_page_table_base], r14
	mov r14, [originalr14]
	iretq
Handle 129
Handle 130
Handle 131
Handle 132
Handle 133
Handle 134
Handle 135
Handle 136
Handle 137
Handle 138
Handle 139
Handle 140
Handle 141
Handle 142
Handle 143
Handle 144
Handle 145
Handle 146
Handle 147
Handle 148
Handle 149
Handle 150
Handle 151
Handle 152
Handle 153
Handle 154
Handle 155
Handle 156
Handle 157
Handle 158
Handle 159
Handle 160
Handle 161
Handle 162
Handle 163
Handle 164
Handle 165
Handle 166
Handle 167
Handle 168
Handle 169
Handle 170
Handle 171
Handle 172
Handle 173
Handle 174
Handle 175
Handle 176
Handle 177
Handle 178
Handle 179
Handle 180
Handle 181
Handle 182
Handle 183
Handle 184
Handle 185
Handle 186
Handle 187
Handle 188
Handle 189
Handle 190
Handle 191
Handle 192
Handle 193
Handle 194
Handle 195
Handle 196
Handle 197
Handle 198
Handle 199
Handle 200
Handle 201
Handle 202
Handle 203
Handle 204
Handle 205
Handle 206
Handle 207
Handle 208
Handle 209
Handle 210
Handle 211
Handle 212
Handle 213
Handle 214
Handle 215
Handle 216
Handle 217
Handle 218
Handle 219
Handle 220
Handle 221
Handle 222
Handle 223
Handle 224
Handle 225
Handle 226
Handle 227
Handle 228
Handle 229
Handle 230
Handle 231
Handle 232
Handle 233
Handle 234
Handle 235
Handle 236
Handle 237
Handle 238
Handle 239
Handle 240
Handle 241
Handle 242
Handle 243
Handle 244
Handle 245
Handle 246
Handle 247
Handle 248
Handle 249
Handle 250
Handle 251
Handle 252
global Handle253
Handle253:
	pushaq
	mov rdi, 253
	call isr
	popaq
	mov [originalr14], r14
	mov r14, [current_page_table_base]
	mov cr3, r14
	mov r14, [originalr14]
	iretq
global Handle254
Handle254:
	pushaq
	mov rdi, 254
	call isr
	popaq
	mov [originalr14], r14
	mov r14, [current_page_table_base]
	mov cr3, r14
	mov r14, [originalr14]
	iretq
Handle 255

section .data
originalr14:
        dq 0 ; zero entry
original_page_table_base:
	dq 0
originalrdi:
	dq 0
originalr15:
	dq 0
