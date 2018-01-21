/********************************************************************
 * Copyright (C) 2009, 2012 by Verimag                              *
 * Initial author: Matthieu Moy                                     *
 ********************************************************************/

/*!
  \file hal.h
  \brief Harwdare Abstraction Layer : implementation for MicroBlaze
  ISS.


*/
#ifndef HAL_H
#define HAL_H

#include <stdint.h>

extern void _hw_exception_handler(void);
/* Dummy implementation of abort(): invalid instruction */
#define abort() do {				\
	printf("abort() function called\r\n");  \
	_hw_exception_handler();		\
} while (0)

#define hal_read32(a)      *((uint32_t *) (a))
// Mais pourquoi donc faut-il mettre "()" autour de "a" dans la def d'une macro?
// ??????????
// ca nous a couté 6h et mon gouter
// pourriez vous l'expliquer dans un cours ou un commentaire 
#define hal_write32(a, d)  *((uint32_t *) (a)) = d
#define hal_wait_for_irq() abort()
#define hal_cpu_relax()    

void microblaze_enable_interrupts(void) {
	__asm("ori     r3, r0, 2\n"
	      "mts     rmsr, r3");
}

#define printf(a) do { \
        int printf_i = 0;                                                    \
        for(printf_i=0; (a)[printf_i]!='\0'; printf_i ++) {                     \
                *((uint32_t*)(UART_BASEADDR+UART_FIFO_WRITE)) = (a)[printf_i]; \
        }                                                                    \
}while(0)
#endif /* HAL_H */
