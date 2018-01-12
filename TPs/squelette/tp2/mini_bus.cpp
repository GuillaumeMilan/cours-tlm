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
//#define DEBUG
using namespace std;
using namespace sc_core;

LCDC screen("Screen",sc_time(1.0 / 25, SC_SEC));

struct initiator : sc_module {
    ensitlm::initiator_socket<initiator> socket;
    sc_signal < bool , SC_MANY_WRITERS > irq_signal;
    sc_core::sc_event trigger_event;

    void write_def(int px_size) {
	ensitlm::data_t val = 0xFFFFFFFF;
	ensitlm::data_t img_val = 0x00000000;
	ensitlm::data_t val_mask = 0xF0000000;
	ensitlm::data_t saved_color = 0xF0000000;
	int current_px = px_size;

	for(ensitlm::addr_t i=0x00000000;i<ROM_SIZE; i+=4) {
	    val_mask = 0xF0000000;
	    img_val = 0;
	    socket.read(i+ROM_ADDR,val);

	    for (int j = 0; j < 4; j++) {
		if (current_px == 0) {
		    saved_color = val_mask&val;
		    for (int h = j; h > -8+j; h--) {
			if (h>0) 
			    saved_color = saved_color | (saved_color<<(4*h));
			else 
			    saved_color = saved_color | (saved_color>>(-4*h));
		    }
#ifdef DEBUG
		    cout << "COLOR";
		    cout << saved_color << endl;
		    cout << j << endl;
#endif
		    current_px = px_size;
		}
		img_val = (img_val|(val_mask&saved_color)>>(4*j));
		val_mask = val_mask >> 4;
		current_px--;
	    }
	    socket.write((i*2)
		    +VIDEO_MEM_ORIGIN,img_val);
	    img_val = 0;
	    for (int j = 0; j < 4; j++) {
		if (current_px == 0) {
		    saved_color = val_mask&val;
		    for (int h = 4+j; h > -4+j; h--) {
			if (h>0)
			    saved_color = saved_color | (saved_color<<(4*h));
			else 
			    saved_color = saved_color | (saved_color>>(-4*h));

		    }
#ifdef DEBUG
		    cout << "COLOR";
		    cout << saved_color << endl;
#endif
		    current_px = px_size;
		}
		img_val = (img_val|(val_mask&saved_color)<<(16-4*j));
		val_mask = val_mask >> 4;
		current_px--;
	    }
#ifdef DEBUG
	    cout << "----------" << endl;
	    cout << "0x" << std::hex << 2*i+4+VIDEO_MEM_ORIGIN << " | ";
	    cout << "0x" << std::hex << img_val << endl;
#endif
	    socket.write(i*2+4
		    +VIDEO_MEM_ORIGIN,img_val);
	}
    }


    void write_to(ensitlm::addr_t offset_addr) {
	ensitlm::data_t val = 0xFFFFFFFF;
	ensitlm::data_t img_val = 0x00000000;
	ensitlm::data_t val_mask = 0xF0000000;

	for(ensitlm::addr_t i=0x00000000;i<ROM_SIZE; i+=4) {
	    val_mask = 0xF0000000;
	    img_val = 0;
	    socket.read(i+ROM_ADDR,val);

	    for (int j = 0; j < 4; j++) {
		img_val = (img_val|(val_mask&val)>>(4*j));
		val_mask = val_mask >> 4;
	    }
	    socket.write(((i*2)+offset_addr)%(SIZE-VIDEO_MEM_ORIGIN)
		    +VIDEO_MEM_ORIGIN,img_val);
	    img_val = 0;
	    for (int j = 0; j < 4; j++) {
		img_val = (img_val|(val_mask&val)<<(16-4*j));
		val_mask = val_mask >> 4;
	    }
#ifdef DEBUG
	    cout << "----------" << endl;
	    cout << "0x" << std::hex << 2*i+4+VIDEO_MEM_ORIGIN << " | ";
	    cout << "0x" << std::hex << img_val << endl;
#endif
	    socket.write((i*2+4+offset_addr)%(SIZE-VIDEO_MEM_ORIGIN)
		    +VIDEO_MEM_ORIGIN,img_val);
	}
    }

    void thread(void) {
	ensitlm::data_t val =      0xFFFFFFFF;
	ensitlm::data_t img_val =  0x00000000;
	ensitlm::data_t val_mask = 0xF0000000;
	/* If wanna do a scrolling image
	 * ensitlm::addr_t addr =     0x00000000;
	 */
	int px_size = 320;
	int inc = 0;
	for(ensitlm::addr_t i=0x00000000;i<ROM_SIZE; i+=4) {
	    val_mask = 0xF0000000;
	    img_val = 0;
	    socket.read(i+ROM_ADDR,val);

	    for (int j = 0; j < 4; j++) {
		img_val = (img_val|(val_mask&val)>>(4*j));
		val_mask = val_mask >> 4;
	    }
	    socket.write((i*2)+VIDEO_MEM_ORIGIN,img_val);
	    img_val = 0;
	    for (int j = 0; j < 4; j++) {
		img_val = (img_val|(val_mask&val)<<(16-4*j));
		val_mask = val_mask >> 4;
	    }
#ifdef DEBUG
	    cout << "----------" << endl;
	    cout << "0x" << std::hex << 2*i+4+VIDEO_MEM_ORIGIN << " | ";
	    cout << "0x" << std::hex << img_val << endl;
#endif
	    socket.write((i*2+4)+VIDEO_MEM_ORIGIN,img_val);
	}
#ifdef DEBUG
	for(ensitlm::addr_t i=0x00000000;i<SIZE; i+=4) {
	    socket.read(i+ORIGIN,val);
	    cout << "0x" << std::hex << i+ORIGIN << " | ";
	    cout << "0x" << std::hex << val << endl;
	}
	cout << "Fin écriture mémoire" << endl;
#endif 
	socket.write(SCREEN_ORIGIN+LCDC_ADDR_REG,VIDEO_MEM_ORIGIN);
	socket.write(SCREEN_ORIGIN+LCDC_START_REG, 0x1);
	socket.write(SCREEN_ORIGIN+LCDC_INT_REG, 0x0);
#ifdef DEBUG
	socket.read(SCREEN_ORIGIN+LCDC_ADDR_REG,addr);
	cout << "Fin Init écran" << endl;
#endif
	wait(sc_time(1.0 / 25, SC_SEC));
#ifdef DEBUG
	cout << std::hex << addr << endl;
#endif
	while (true) {
#ifndef DEBUG
	    wait(trigger_event);
	    inc = (inc + 1)%50;
	    if (inc==0) {
		if (px_size !=1) {
		    px_size=px_size/2;
		    if (px_size==0)
			px_size=1;
		}
	    }
	    /* => défilement 
	     * addr = addr-1*4;
	     * write_to(addr);
	     */
	    write_def(px_size);
	    wait(sc_time(1.0 / 2, SC_SEC));
	    socket.write(SCREEN_ORIGIN+LCDC_INT_REG, 0x0);
	    /* Do stuff to annimate the screen */
#endif
#ifdef DEBUG
	    wait(sc_time(1.0 / 25, SC_SEC));
	    /** Loop permettant d'écrire à la main dans l'écran
	     * cette boucle limite le temps de rafraichissement de 
	     * l'écran et doit etre enlever pour un taux de 25 img/s
	     **/
	    if (addr > ORIGIN+SIZE) 
		addr = VIDEO_MEM_ORIGIN - 4;
	    addr = addr + 4;
	    cout << "Entrer un nombre" << endl;
	    cin >> val;
	    cout << name() <<": je vais envoyer : " << std::dec << val;
	    cout << " à l'adresse : 0x" << std::hex << addr << endl;
	    socket.write(addr, val);
	    socket.read(addr,val);
	    cout << "Nous retrouvons à cette adresse " << std::dec << val <<  endl;
#endif
	}
    }
    /* Nous créons une SC_METHOD qui a pour sensibilité le signal d'interuption
     * Cela nous permet d'écrire au bon moment dans la mémoire
     */
    void handler() {
	/* Put code to modifiy the screen content here */
	if (irq_signal==1) {
#ifdef DEBUG
	    cout << "J'ai recus le signal: ";
	    cout << irq_signal << endl;
#endif 
	    trigger_event.notify();
	}
    }

    SC_CTOR(initiator) {
	SC_THREAD(thread);
	SC_METHOD(handler);
	sensitive << irq_signal;
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
