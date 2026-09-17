;
; gptr_shim.S - SDCC 4.5.0 quirk workaround
;
; Objects in libsdcc.lib (gptrget1..4.o, gptrload.o, gptrput1..4.o)
; reference the *historical* eeprom names ___eeprom_gptr{get,load,put}N
; which are no longer provided (they were split into ___eeprom8_gptr* /
; ___eeprom16_gptr*).  All use the same register convention, so the old
; names can simply forward to the ___eeprom8_gptr* implementations.
;
	list	p=18f25k80, r=hex, c=132
	radix	hex

#include <p18f25k80.inc>

	global	___eeprom_gptrget1
	global	___eeprom_gptrget2
	global	___eeprom_gptrget3
	global	___eeprom_gptrget4
	global	___eeprom_gptrload
	global	___eeprom_gptrput1
	global	___eeprom_gptrput2
	global	___eeprom_gptrput3
	global	___eeprom_gptrput4

	extern	___eeprom8_gptrget1
	extern	___eeprom8_gptrget2
	extern	___eeprom8_gptrget3
	extern	___eeprom8_gptrget4
	extern	___eeprom8_gptrload
	extern	___eeprom8_gptrput1
	extern	___eeprom8_gptrput2
	extern	___eeprom8_gptrput3
	extern	___eeprom8_gptrput4

shim_a	code

___eeprom_gptrget1:
	goto	___eeprom8_gptrget1
___eeprom_gptrget2:
	goto	___eeprom8_gptrget2
___eeprom_gptrget3:
	goto	___eeprom8_gptrget3
___eeprom_gptrget4:
	goto	___eeprom8_gptrget4
___eeprom_gptrload:
	goto	___eeprom8_gptrload
___eeprom_gptrput1:
	goto	___eeprom8_gptrput1
___eeprom_gptrput2:
	goto	___eeprom8_gptrput2
___eeprom_gptrput3:
	goto	___eeprom8_gptrput3
___eeprom_gptrput4:
	goto	___eeprom8_gptrput4

	end