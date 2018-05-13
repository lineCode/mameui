// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Candy Crane by A.G.E.  (Advanced Game Engineering)

(company no longer in business)


Device is a 27c256   location U3


*/

#include "emu.h"
#include "speaker.h"


class age_candy_state : public driver_device
{
public:
	age_candy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	//  ,m_maincpu(*this, "maincpu")
	{ }

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void age_candy(machine_config &config);
	void age_candy_map(address_map &map);
//  required_device<mcs51_cpu_device> m_maincpu;
};

static INPUT_PORTS_START( age_candy )
INPUT_PORTS_END



void age_candy_state::machine_start()
{
}

void age_candy_state::machine_reset()
{
}


#ifdef UNUSED_DEFINITION
void age_candy_state::age_candy_map(address_map &map)
{
	map(0xc000, 0xffff).rom().region("maincpu", 0x4000);
}
#endif

MACHINE_CONFIG_START(age_candy_state::age_candy)

	/* basic machine hardware */
//  MCFG_DEVICE_ADD("maincpu", HPC46104, 8000000) // unknown clock; HPC emulation needed
//  MCFG_DEVICE_PROGRAM_MAP(age_candy_map)
//  MCFG_DEVICE_IO_MAP(age_candy_io)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
MACHINE_CONFIG_END



ROM_START( age_cand )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "agecandy.u3", 0x0000, 0x8000, CRC(c8cfc666) SHA1(a1c475ae105746e984741af0723a712f09d7b847) )
ROM_END

GAME( 19??, age_cand, 0, age_candy, age_candy, age_candy_state, empty_init, ROT0, "Advanced Game Engineering", "Candy Crane (AGE)", MACHINE_IS_SKELETON_MECHANICAL )
