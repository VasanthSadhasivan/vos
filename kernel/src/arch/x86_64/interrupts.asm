global HandleGeneric
global interrupts_enabled


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
	add rsp, 0x8
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
HandleWithErrorCode 14
HandleWithErrorCode 17
HandleWithErrorCode 21
HandleWithErrorCode 29
HandleWithErrorCode 30

Handle 0
Handle 1
Handle 2
Handle 3
Handle 4
Handle 5
Handle 6
Handle 7
Handle 9
Handle 15
Handle 16
Handle 18
Handle 19
Handle 20
Handle 22
Handle 23
Handle 24
Handle 25
Handle 26
Handle 27
Handle 28
Handle 31
Handle 32
Handle 33
Handle 34
Handle 35
Handle 36
Handle 37
Handle 38
Handle 39
Handle 40
Handle 41
Handle 42
Handle 43
Handle 44
Handle 45
Handle 46
Handle 47
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
Handle 128
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
Handle 253
Handle 254
Handle 255
