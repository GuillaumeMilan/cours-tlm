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

#define ORIGIN 0x10000000
#define LIMIT 0x100000FF
using namespace std;
using namespace sc_core;

struct initiator : sc_module {
	ensitlm::initiator_socket<initiator> socket;
	void thread(void) {
		ensitlm::data_t val = 1;
		ensitlm::addr_t addr = ORIGIN - 4;
		while (true) {
			if (addr > LIMIT) 
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

struct memory : sc_module {
	uint32_t *data = NULL;
	ensitlm::target_socket<memory> socket;
	tlm::tlm_response_status write(const ensitlm::addr_t &a,
	                               const ensitlm::data_t &d) {
		cout << "j'ai reçu : " << std::dec << d;
		cout << " à l'adresse : 0x" << std::hex << (int)a << endl;
		cout << " ---- Ecriture ---- " << endl;
		data[(int)a] = (uint32_t) d;
		return tlm::TLM_OK_RESPONSE;
	}

	tlm::tlm_response_status read(const ensitlm::addr_t &a,
	                              /* */ ensitlm::data_t &d) {
	    cout << "Address : " << (int)a << endl;
	    d = data[(int)a];
	    return tlm::TLM_OK_RESPONSE;
	}
	

	memory(sc_core::sc_module_name name,int size) {
	    data = new uint32_t [size/sizeof(uint32_t)];
	}
	~memory() {
	    delete []data;
	}

	SC_CTOR(memory) { /* */	}
    
};

int sc_main(int argc, char **argv) {
	/*
	 +---------+	+-------------+	   +--------+
	 |	  +++  +-+	     +++  +++	    |
	 | Alice  | +--+ |  Router   | +--+ |  Bob  |
	 |	  +++  +-+	     +++  +++	    |
	 +---------+	+-------------+	   +--------+
	 */

	initiator a("Generator1");
	//target b("Bob");
	Bus router("Bus");
	memory mems("Memory",0x100);
	/* Bob is mapped at addresses [0, 100[, i.e. ... */
	/*router.map(b.socket, 0, 100);*/
	router.map(mems.socket, ORIGIN, LIMIT);
	

	/* connect components to the bus */
	a.socket.bind(router.target);
	/*a.socket.bind(router.mems);*/
	router.initiator.bind(mems.socket);
	/*router.initiator.bind(b.socket);*/

	/* and start simulation */
	sc_start();
	return 0;
}
