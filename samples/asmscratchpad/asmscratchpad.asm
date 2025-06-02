.longbranch on
.longlea on
.longldr on
.longstr on

.text
b .L1
.zero 262144
.zero 262144
.zero 262144
.zero 262144
.L1:

hlt

.rodata
; string to print

.zero 32000
_hello: .string "Hello World!"

