/********************************************************************
 * Copyright (C) 2009 by Ensimag and Verimag                        *
 * Initial author: Matthieu Moy                                     *
 ********************************************************************/

/* Plateforme SystemC utilisant le protocole TLM Ensitlm.

   Cette version est en un seul fichier pour faciliter la
   compréhension. Une version multi-fichiers (donc plus propre) est
   également disponible.
*/

#include "ensitlm.h"
#include "bus.h"
#include "memory.h" 
#include "LCDC.h"
#include "LCDC_registermap.h"
#include "ROM.h"

#define ORIGIN 0x00000000
#define SIZE 0x15400
#define VIDEO_MEM_ORIGIN 0x2800
#define SCREEN_ORIGIN 0x10000000
#define SCREEN_SIZE 0xC
#define ROM_ADDR 0x00015400
using namespace std;
using namespace sc_core;

LCDC screen("Screen",sc_time(1.0 / 25, SC_SEC));

struct initiator : sc_module {
	ensitlm::initiator_socket<initiator> socket;
	sc_signal < bool , SC_MANY_WRITERS > irq_signal;
	void thread(void) {
		ensitlm::data_t val = 0xFFFFFFFF;
		ensitlm::data_t val_hi = 0xFFFFFFFF;
		ensitlm::data_t val_lo = 0xFFFFFFFF;
		ensitlm::data_t val_mask_hi = 0xFFFF0000;
		ensitlm::data_t val_mask_lo = 0x0000FFFF;
		ensitlm::addr_t addr = VIDEO_MEM_ORIGIN;
		for(ensitlm::addr_t i=0x00000000;i<ROM_SIZE; i+=4) {
		    socket.read(i+ROM_ADDR,val);
		    val_hi = val&val_mask_hi;
		    val_hi = val_hi | (val_hi >> 16);
		    val_lo = (val&val_mask_lo)<< 16;
		    val_lo = val_lo | (val_lo >> 16);
		    socket.write((i*2)+VIDEO_MEM_ORIGIN,val_hi);
		    socket.write((i*2+4)+VIDEO_MEM_ORIGIN,val_lo);
		}
		for(ensitlm::addr_t i=0x00000000;i<SIZE; i+=4) {
		    socket.read(i+ORIGIN,val);
		    cout << "0x" << std::hex << i+ORIGIN << " | ";
		    cout << "0x" << std::hex << val << endl;
		}

		cout << "Fin écriture mémoire" << endl;
		socket.write(SCREEN_ORIGIN+LCDC_ADDR_REG,VIDEO_MEM_ORIGIN);
		socket.write(SCREEN_ORIGIN+LCDC_START_REG, 0x1);
		socket.write(SCREEN_ORIGIN+LCDC_INT_REG, 0x0);
		socket.read(SCREEN_ORIGIN+LCDC_ADDR_REG,addr);
		cout << "Fin Init écran" << endl;
		wait(sc_time(1.0 / 25, SC_SEC));
		cout << std::hex << addr << endl;
		while (true) {
		    wait(sc_time(1.0 / 25, SC_SEC));
		    /** Loop permettant d'écrire à la main dans l'écran
		     * cette boucle limite le temps de rafraichissement de 
		     * l'écran et doit etre enlever pour un taux de 25 img/s
		     **/
		    if (addr > ORIGIN+SIZE) 
			addr = ORIGIN - 4;
		    addr = addr + 4;
		    cout << "Entrer un nombre" << endl;
		    cin >> val;
		    cout << name() <<": je vais envoyer : " << std::dec << val;
		    cout << " à l'adresse : 0x" << std::hex << addr << endl;
		    socket.write(addr, val);
		    socket.read(addr,val);
		    cout << "Nous retrouvons à cette adresse " << std::dec << val <<  endl;

		}
	}
	SC_CTOR(initiator) {
		SC_THREAD(thread);
	}
};

int sc_main(int argc, char **argv) {
	/*
	 +---------+	+-------------+	   +--------+
	 |	  +++  +-+	     +++  +++	    |
	 | Alice  | +--+ |  Router   | +--+ |  Bob  |
	 |	  +++  +-+	     +++  +++	    |
	 +---------+	+-------------+	   +--------+
	 */

	initiator bench("Generator1");
	Bus router("Bus");
	Memory mems("Memory",SIZE);
	ROM ro_mem("ROM");
	/* Bob is mapped at addresses [0, 100[, i.e. ... */
	router.map(ro_mem.socket, ROM_ADDR, ROM_SIZE);
	router.map(screen.target_socket, SCREEN_ORIGIN, SCREEN_SIZE);
	router.map(mems.target, ORIGIN, SIZE);

	/* connect components to the bus */
	bench.socket.bind(router.target);
	screen.initiator_socket.bind(router.target);
	

	
	router.initiator.bind(mems.target);
	router.initiator.bind(screen.target_socket);
	router.initiator.bind(ro_mem.socket);

	screen.display_int.bind(bench.irq_signal);

	/* and start simulation */
	sc_start();
	return 0;
}
