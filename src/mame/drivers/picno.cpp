// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************************************************************

Konami Picno and Picno2

Skeleton driver started on 2017-11-30, can be claimed by anyone interested.

Information provided by Team Europe.

Chips: HD6435328F10 (H8/532 CPU with inbuilt ROM), HN62334BP (27c040 ROM), Konami custom chip 054715 (rectangular 100 pins),
       HM538121JP-10, M514256B-70J, OKI M6585.
Crystals: D200L2 (Y1) and D214A3 (Y2), frequencies unknown.

The size of the address space and other things is controlled by the 3 mode pins. It's assumed we are in Mode 4.

Can't do anything until the internal ROM is dumped.

******************************************************************************************************************************/

#include "emu.h"
#include "cpu/h8/h83002.h"
//#include "sound/multipcm.h"
//#include "screen.h"
//#include "speaker.h"

class picno_state : public driver_device
{
public:
	picno_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

private:
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START( mem_map, AS_PROGRAM, 16, picno_state )
	AM_RANGE(0x00000, 0x07fff) AM_ROM AM_REGION("roms", 0) // 32kb internal rom
	AM_RANGE(0x0fb80, 0x0ff7f) AM_RAM // internal ram
	AM_RANGE(0x0ff80, 0x0ffff) // internal controls
	AM_RANGE(0x10000, 0x8ffff) AM_ROM AM_REGION("roms", 0x8000) // guess
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, picno_state )
//  ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static INPUT_PORTS_START( picno )
INPUT_PORTS_END

static MACHINE_CONFIG_START( picno )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",  H83002, XTAL_20MHz) /* TODO: correct CPU type (H8/532), crystal is a guess, divided by 2 in the cpu */
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)

	//MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker") // no speaker in the unit, but there's a couple of sockets on the back
	//MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	//MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

ROM_START( picno )
	ROM_REGION(0x88000, "roms", 0)
	ROM_LOAD( "hd6435328f10.u5", 0x00000, 0x08000, NO_DUMP ) // internal rom
	ROM_LOAD( "hn62334bp.u2",    0x08000, 0x80000, CRC(ae89a9a5) SHA1(51ed458ffd151e19019beb23517263efce4be272) )
ROM_END

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT  COMPANY   FULLNAME       FLAGS
CONS( 1993, picno,    0,      0,      picno,    picno,    picno_state,    0,    "Konami", "Picno",      MACHINE_IS_SKELETON )
