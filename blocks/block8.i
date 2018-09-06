//NAME: COMP412
//NETID: comp412
//SIM INPUT:
//OUTPUT: 2 110
//
// COMP 412, Lab 1 - block8.i
//
//  Example usage: ./sim < block8.i
//
	loadI	0    => r0
	loadI	1    => r1 
	loadI	2    => r3 
	loadI	10   => r2 
	loadI	1024 => r10
//
	store	r3    => r10
        add	r0,r0 => r4
	add	r0,r1 => r3
	add     r1,r1 => r0
//
	add	r4,r3 => r4
	add	r3,r1 => r3
	add	r4,r3 => r4
	add	r3,r1 => r3
	add	r4,r3 => r4
	add	r3,r1 => r3
	add	r4,r3 => r4
	add	r3,r1 => r3
	add	r4,r3 => r4
	add	r3,r1 => r3
	add	r4,r3 => r4
	add	r3,r1 => r3
	add	r4,r3 => r4
	add	r3,r1 => r3
	add	r4,r3 => r4
	add	r3,r1 => r3
	add	r4,r3 => r4
	add	r3,r1 => r3
	add	r4,r3 => r4
	load	r10   => r3
	mult	r4,r3 => r4
	add	r10,r2 => r10
	add     r10,r0 => r10
	store	r4    => r10
	output	1024
	output	1036	
//
