#include "ensitlm.h"
#include "native_wrapper.h"

/*
 * The C compiler does a bit of magic on the main() function. Trick it
 * by changing the name to something else.
 */
#define main __start

/* extern "C" is needed since the software is compiled in C and
 * is linked against native_wrapper.cpp, which is compiled in C++.
 */

using namespace sc_core;
extern "C" int main();
extern "C" void interrupt_handler();

extern "C" void hal_write32(uint32_t addr, uint32_t data) {
	NativeWrapper::get_instance()->hal_write32(addr, data);
}

extern "C" unsigned int hal_read32(uint32_t addr) {
	return NativeWrapper::get_instance()->hal_read32(addr);
}

extern "C" void hal_cpu_relax() {
	NativeWrapper::get_instance()->hal_cpu_relax();
}

extern "C" void hal_wait_for_irq() {
	NativeWrapper::get_instance()->hal_wait_for_irq();
}

/* To keep it simple, the soft wrapper is a singleton, we can
 * call its methods in a simple manner, using
 * NativeWrapper::get_instance()->method_name()
 */
NativeWrapper * NativeWrapper::get_instance() {
	static NativeWrapper * instance = NULL;
	if (!instance)
		instance = new NativeWrapper("native_wrapper");
	return instance;
}

NativeWrapper::NativeWrapper(sc_core::sc_module_name name) : sc_module(name),
							     irq("irq")
{
	SC_THREAD(compute);
	SC_METHOD(hal_wait_for_irq);
	sensitive << irq;
}

void NativeWrapper::hal_write32(unsigned int addr, unsigned int data)
{
	socket.write(addr, data);
}

unsigned int NativeWrapper::hal_read32(unsigned int addr)
{
        ensitlm::data_t data;
        tlm::tlm_response_status status;
        status = socket.read(addr, data);
        if (status != tlm::TLM_OK_RESPONSE) {
            return status;
        }
        return data;
}

void NativeWrapper::hal_cpu_relax()
{
	wait(sc_time(1.0 / 100, SC_SEC));
}

void NativeWrapper::hal_wait_for_irq()
{
	if( irq == 1 ) 
                this->interrupt_handler_internal();
}

void NativeWrapper::compute()
{
        main();
}

void NativeWrapper::interrupt_handler_internal()
{
	interrupt_handler();
}
