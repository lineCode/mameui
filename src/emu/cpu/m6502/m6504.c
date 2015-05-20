// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6504.c

    Mostek 6502, NMOS variant with reduced address bus

***************************************************************************/

#include "emu.h"
#include "m6504.h"

const device_type M6504 = &device_creator<m6504_device>;

m6504_device::m6504_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	m6502_device(mconfig, M6504, "M6504", tag, owner, clock, "m6504", __FILE__)
{
	program_config.m_addrbus_width = 13;
}

void m6504_device::device_start()
{
	if(direct_disabled)
		mintf = new mi_6504_nd;
	else
		mintf = new mi_6504_normal;

	init();
}

UINT8 m6504_device::mi_6504_normal::read(UINT16 adr)
{
	return program->read_byte(adr & 0x1fff);
}

UINT8 m6504_device::mi_6504_normal::read_direct(UINT16 adr)
{
	return direct->read_raw_byte(adr & 0x1fff);
}

UINT8 m6504_device::mi_6504_normal::read_decrypted(UINT16 adr)
{
	return direct->read_decrypted_byte(adr & 0x1fff);
}

void m6504_device::mi_6504_normal::write(UINT16 adr, UINT8 val)
{
	program->write_byte(adr & 0x1fff, val);
}

UINT8 m6504_device::mi_6504_nd::read_direct(UINT16 adr)
{
	return read(adr);
}

UINT8 m6504_device::mi_6504_nd::read_decrypted(UINT16 adr)
{
	return read(adr);
}
