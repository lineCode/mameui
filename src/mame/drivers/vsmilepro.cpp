// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    Skeleton driver for VTech V.Smile Pro CD System

    30/05/2016

    Some information about the hardware can be found at

    http://www.x86-secret.com/dossier-64-VTech_V_Smile_Pro.html


    In particular
    - It uses a LSI Zevio 1020 CPU + peripherals which comprises ARM926EJ-S CPU,
      ZSP400 DPS, 3D graphics processor & 2D graphics processor
    - The CD controller is a Sony CXD3059AR

****************************************************************************/

#include "emu.h"
#include "cdrom.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "imagedev/chd_cd.h"
#include "softlist.h"


class vsmilpro_state : public driver_device
{
public:
	vsmilpro_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void vsmilpro(machine_config &config);
	void vsmilpro_map(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
};

void vsmilpro_state::vsmilpro_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom();
}


/* Input ports */
static INPUT_PORTS_START( vsmilpro )
INPUT_PORTS_END

MACHINE_CONFIG_START(vsmilpro_state::vsmilpro)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", ARM9, 150000000)
	MCFG_DEVICE_PROGRAM_MAP(vsmilpro_map)

	MCFG_CDROM_ADD("cdrom")
	MCFG_CDROM_INTERFACE("vsmile_vdisk")

	MCFG_SOFTWARE_LIST_ADD("cd_list","vsmile_cd")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( vsmilpro )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "70004.bin", 0x000000, 0x200000, CRC(b9161eac) SHA1(8d75fdeda8c4e228a0b1efd35011f9f667f9fb23) )
ROM_END

/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY  FULLNAME       FLAGS
COMP( 2007, vsmilpro, 0,      0,      vsmilpro, vsmilpro, vsmilpro_state, empty_init, "VTech", "V.Smile Pro", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
