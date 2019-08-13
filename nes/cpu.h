#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <memory.h>

typedef struct cpu_registers_s
{
	/* Accumulator 1 byte wide */
	uint8_t A;

	/* X register 1 byte wide */
	uint8_t X;

	/* Y register 1 byte wide */
	uint8_t Y;

	/* Program Counter 2 byte wide */
	uint16_t PC;

	/* Stack Pointer 1 byte wide */
	uint8_t S;

	/* Status Register 1 byte wide */
	uint8_t P;


} cpu_registers_t;

typedef struct nes_cpu_s 
{

	uint64_t num_cycles;
	uint16_t op_address;
	uint8_t  op_value;
	uint8_t  additional_cycles;

	/* CPU Registers */
	cpu_registers_t regs;

	/* Mem interface */
	nes_memmap_t *memmap;

} nes_cpu_t;

void nes_cpu_init(nes_cpu_t *nes_cpu, nes_memmap_t *nes_memmap);
uint32_t nes_cpu_run(nes_cpu_t *nes_cpu);
void nes_cpu_interrupt(nes_cpu_t *nes_cpu);
void nes_cpu_print_state(nes_cpu_t *nes_cpu, uint8_t opcode);

#endif