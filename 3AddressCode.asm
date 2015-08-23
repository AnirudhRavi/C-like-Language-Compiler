	 	 	 3 ADDRESS CODE 
	 	 	 ______________ 
 
a = 0;
ab = 0;
b = 0;
c = 0;


t0 = 1
a = t0

t1 = 2
b = t1

t2 = a
c = t2

t3 = 0
c = t3
for_loop_start1:
t4 = c
t5 = 5
CMP t4, t5
JG for_loop_end1
t6 = a
t7 = 1
t8 = t6 + t7
a = t8
t9 = c
t10 = 1
t11 = t9 + t10
c = t11
JMP for_loop_start1
for_loop_end1:

t12 = c
d = t12
