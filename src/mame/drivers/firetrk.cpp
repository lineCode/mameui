// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

Atari Fire Truck + Super Bug + Monte Carlo driver

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "includes/firetrk.h"
#include "sound/discrete.h"

#include "superbug.lh"

#define MASTER_CLOCK (XTAL_12_096MHz)



void firetrk_state::set_service_mode(int enable)
{
	m_in_service_mode = enable;

	/* watchdog is disabled during service mode */
	machine().watchdog_enable(!enable);

	/* change CPU clock speed according to service switch change */
	m_maincpu->set_unscaled_clock(enable ? (MASTER_CLOCK/16) : (MASTER_CLOCK/12));
}


INPUT_CHANGED_MEMBER(firetrk_state::service_mode_switch_changed)
{
	set_service_mode(newval);
}


INPUT_CHANGED_MEMBER(firetrk_state::firetrk_horn_changed)
{
	m_discrete->write(generic_space(), FIRETRUCK_HORN_EN, newval);
}


INPUT_CHANGED_MEMBER(firetrk_state::gear_changed)
{
	if (newval)
	{
		m_gear = (FPTR)param;
		output().set_value("P1gear", m_gear+1);
	}
}


TIMER_DEVICE_CALLBACK_MEMBER(firetrk_state::firetrk_scanline)
{
	int scanline = param;

	// periodic IRQs are generated by inverse 16V signal
	if ((scanline & 0x1f) == 0)
		generic_pulse_irq_line(m_maincpu, 0, 1);

	// vblank interrupt
	// NMIs are disabled during service mode
	if (!m_in_service_mode && scanline == 240)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


WRITE8_MEMBER(firetrk_state::firetrk_output_w)
{
	/* BIT0 => START1 LAMP */
	output().set_led_value(0, !(data & 0x01));

	/* BIT1 => START2 LAMP */
	output().set_led_value(1, !(data & 0x02));

	/* BIT2 => FLASH       */
	m_flash = data & 0x04;

	/* BIT3 => TRACK LAMP  */
	output().set_led_value(3, !(data & 0x08));

	/* BIT4 => ATTRACT     */
	m_discrete->write(space, FIRETRUCK_ATTRACT_EN, data & 0x10);
	machine().bookkeeping().coin_lockout_w(0, !(data & 0x10));
	machine().bookkeeping().coin_lockout_w(1, !(data & 0x10));

	/* BIT5 => START3 LAMP */
	output().set_led_value(2, !(data & 0x20));

	/* BIT6 => UNUSED      */

	/* BIT7 => BELL OUT    */
	m_discrete->write(space, FIRETRUCK_BELL_EN, data & 0x80);
}


WRITE8_MEMBER(firetrk_state::superbug_output_w)
{
	/* BIT0 => START LAMP */
	output().set_led_value(0, offset & 0x01);

	/* BIT1 => ATTRACT    */
	m_discrete->write(space, SUPERBUG_ATTRACT_EN, offset & 0x02);
	machine().bookkeeping().coin_lockout_w(0, !(offset & 0x02));
	machine().bookkeeping().coin_lockout_w(1, !(offset & 0x02));

	/* BIT2 => FLASH      */
	m_flash = offset & 0x04;

	/* BIT3 => TRACK LAMP */
	output().set_led_value(1, offset & 0x08);
}


WRITE8_MEMBER(firetrk_state::montecar_output_1_w)
{
	/* BIT0 => START LAMP    */
	output().set_led_value(0, !(data & 0x01));

	/* BIT1 => TRACK LAMP    */
	output().set_led_value(1, !(data & 0x02));

	/* BIT2 => ATTRACT       */
	m_discrete->write(space, MONTECAR_ATTRACT_INV, data & 0x04);

	/* BIT3 => UNUSED        */
	/* BIT4 => UNUSED        */

	/* BIT5 => COIN3 COUNTER */
	machine().bookkeeping().coin_counter_w(0, data & 0x80);

	/* BIT6 => COIN2 COUNTER */
	machine().bookkeeping().coin_counter_w(1, data & 0x40);

	/* BIT7 => COIN1 COUNTER */
	machine().bookkeeping().coin_counter_w(2, data & 0x20);
}


WRITE8_MEMBER(firetrk_state::montecar_output_2_w)
{
	m_flash = data & 0x80;

	m_discrete->write(space, MONTECAR_BEEPER_EN, data & 0x10);
	m_discrete->write(space, MONTECAR_DRONE_LOUD_DATA, data & 0x0f);
}


void firetrk_state::machine_reset()
{
	set_service_mode(0);
}


READ8_MEMBER(firetrk_state::firetrk_dip_r)
{
	UINT8 val0 = ioport("DIP_0")->read();
	UINT8 val1 = ioport("DIP_1")->read();

	if (val1 & (1 << (2 * offset + 0))) val0 |= 1;
	if (val1 & (1 << (2 * offset + 1))) val0 |= 2;

	return val0;
}


READ8_MEMBER(firetrk_state::montecar_dip_r)
{
	UINT8 val0 = ioport("DIP_0")->read();
	UINT8 val1 = ioport("DIP_1")->read();

	if (val1 & (1 << (3 - offset))) val0 |= 1;
	if (val1 & (1 << (7 - offset))) val0 |= 2;

	return val0;
}


CUSTOM_INPUT_MEMBER(firetrk_state::steer_dir_r)
{
	return m_steer_dir[(FPTR)param];
}


CUSTOM_INPUT_MEMBER(firetrk_state::steer_flag_r)
{
	return m_steer_flag[(FPTR)param];
}


CUSTOM_INPUT_MEMBER(firetrk_state::skid_r)
{
	UINT32 ret;
	int which = (FPTR)param;

	if (which != 2)
		ret = m_skid[which];
	else
		ret = m_skid[0] | m_skid[1];

	return ret;
}


CUSTOM_INPUT_MEMBER(firetrk_state::crash_r)
{
	UINT32 ret;
	int which = (FPTR)param;

	if (which != 2)
		ret = m_crash[which];
	else
		ret = m_crash[0] | m_crash[1];

	return ret;
}


CUSTOM_INPUT_MEMBER(firetrk_state::gear_r)
{
	return (m_gear == (FPTR)param) ? 1 : 0;
}


READ8_MEMBER(firetrk_state::firetrk_input_r)
{
	int i;

	/* update steering wheels */
	for (i = 0; i < 2; i++)
	{
		UINT32 const new_dial = read_safe(ioport(i ? "STEER_2" : "STEER_1"), 0);
		INT32 const delta = new_dial - m_dial[i];

		if (delta != 0)
		{
			m_steer_flag[i] = 0;
			m_steer_dir[i] = (delta < 0) ? 1 : 0;

			m_dial[i] = m_dial[i] + delta;
		}
	}

	return ((read_safe(ioport("BIT_0"), 0) & (1 << offset)) ? 0x01 : 0) |
			((read_safe(ioport("BIT_6"), 0) & (1 << offset)) ? 0x40 : 0) |
			((read_safe(ioport("BIT_7"), 0) & (1 << offset)) ? 0x80 : 0);
}


READ8_MEMBER(firetrk_state::montecar_input_r)
{
	UINT8 ret = firetrk_input_r(space, offset);

	if (m_crash[0])
		ret |= 0x02;

	/* can this be right, bit 0 again ???? */
	if (m_crash[1])
		ret |= 0x01;

	return ret;
}


WRITE8_MEMBER(firetrk_state::blink_on_w)
{
	*m_blink = TRUE;
}


WRITE8_MEMBER(firetrk_state::montecar_car_reset_w)
{
	m_crash[0] = 0;
	m_skid[0] = 0;
}


WRITE8_MEMBER(firetrk_state::montecar_drone_reset_w)
{
	m_crash[1] = 0;
	m_skid[1] = 0;
}


WRITE8_MEMBER(firetrk_state::steer_reset_w)
{
	m_steer_flag[0] = 1;
	m_steer_flag[1] = 1;
}


WRITE8_MEMBER(firetrk_state::crash_reset_w)
{
	m_crash[0] = 0;
	m_crash[1] = 0;
}


static ADDRESS_MAP_START( firetrk_map, AS_PROGRAM, 8, firetrk_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x0700) AM_RAM AM_SHARE("alpha_num_ram")
	AM_RANGE(0x0800, 0x08ff) AM_MIRROR(0x0700) AM_RAM AM_SHARE("playfield_ram")
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x001f) AM_WRITEONLY AM_SHARE("scroll_y")
	AM_RANGE(0x1020, 0x1020) AM_MIRROR(0x001f) AM_WRITEONLY AM_SHARE("scroll_x")
	AM_RANGE(0x1040, 0x1040) AM_MIRROR(0x001f) AM_WRITE(crash_reset_w)
	AM_RANGE(0x1060, 0x1060) AM_MIRROR(0x001f) AM_WRITE(firetrk_skid_reset_w)
	AM_RANGE(0x1080, 0x1080) AM_MIRROR(0x001f) AM_WRITEONLY AM_SHARE("car_rot")
	AM_RANGE(0x10a0, 0x10a0) AM_MIRROR(0x001f) AM_WRITE(steer_reset_w)
	AM_RANGE(0x10c0, 0x10c0) AM_MIRROR(0x001f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x10e0, 0x10e0) AM_MIRROR(0x001f) AM_WRITE(blink_on_w) AM_SHARE("blink")
	AM_RANGE(0x1400, 0x1400) AM_MIRROR(0x001f) AM_WRITE(firetrk_motor_snd_w)
	AM_RANGE(0x1420, 0x1420) AM_MIRROR(0x001f) AM_WRITE(firetrk_crash_snd_w)
	AM_RANGE(0x1440, 0x1440) AM_MIRROR(0x001f) AM_WRITE(firetrk_skid_snd_w)
	AM_RANGE(0x1460, 0x1460) AM_MIRROR(0x001f) AM_WRITEONLY AM_SHARE("drone_x")
	AM_RANGE(0x1480, 0x1480) AM_MIRROR(0x001f) AM_WRITEONLY AM_SHARE("drone_y")
	AM_RANGE(0x14a0, 0x14a0) AM_MIRROR(0x001f) AM_WRITEONLY AM_SHARE("drone_rot")
	AM_RANGE(0x14c0, 0x14c0) AM_MIRROR(0x001f) AM_WRITE(firetrk_output_w)
	AM_RANGE(0x14e0, 0x14e0) AM_MIRROR(0x001f) AM_WRITE(firetrk_xtndply_w)
	AM_RANGE(0x1800, 0x1807) AM_MIRROR(0x03f8) AM_READ(firetrk_input_r) AM_WRITENOP
	AM_RANGE(0x1c00, 0x1c03) AM_MIRROR(0x03fc) AM_READ(firetrk_dip_r)
	AM_RANGE(0x2000, 0x3fff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( superbug_map, AS_PROGRAM, 8, firetrk_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x0000, 0x00ff) AM_RAM
	AM_RANGE(0x0100, 0x0100) AM_MIRROR(0x001f) AM_WRITEONLY AM_SHARE("scroll_y")
	AM_RANGE(0x0120, 0x0120) AM_MIRROR(0x001f) AM_WRITEONLY AM_SHARE("scroll_x")
	AM_RANGE(0x0140, 0x0140) AM_MIRROR(0x001f) AM_WRITE(crash_reset_w)
	AM_RANGE(0x0160, 0x0160) AM_MIRROR(0x001f) AM_WRITE(firetrk_skid_reset_w)
	AM_RANGE(0x0180, 0x0180) AM_MIRROR(0x001f) AM_WRITEONLY AM_SHARE("car_rot")
	AM_RANGE(0x01a0, 0x01a0) AM_MIRROR(0x001f) AM_WRITE(steer_reset_w)
	AM_RANGE(0x01c0, 0x01c0) AM_MIRROR(0x001f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x01e0, 0x01e0) AM_MIRROR(0x001f) AM_WRITE(blink_on_w) AM_SHARE("blink")
	AM_RANGE(0x0200, 0x0207) AM_MIRROR(0x0018) AM_READ(firetrk_input_r)
	AM_RANGE(0x0220, 0x0220) AM_MIRROR(0x001f) AM_WRITE(firetrk_xtndply_w)
	AM_RANGE(0x0240, 0x0243) AM_MIRROR(0x001c) AM_READ(firetrk_dip_r)
	AM_RANGE(0x0260, 0x026f) AM_MIRROR(0x0010) AM_WRITE(superbug_output_w)
	AM_RANGE(0x0280, 0x0280) AM_MIRROR(0x001f) AM_WRITE(superbug_motor_snd_w)
	AM_RANGE(0x02a0, 0x02a0) AM_MIRROR(0x001f) AM_WRITE(firetrk_crash_snd_w)
	AM_RANGE(0x02c0, 0x02c0) AM_MIRROR(0x001f) AM_WRITE(firetrk_skid_snd_w)
	AM_RANGE(0x0400, 0x041f) AM_RAM AM_SHARE("alpha_num_ram")
	AM_RANGE(0x0500, 0x05ff) AM_RAM AM_SHARE("playfield_ram")
	AM_RANGE(0x0800, 0x1fff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( montecar_map, AS_PROGRAM, 8, firetrk_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x0700) AM_RAM AM_SHARE("alpha_num_ram")
	AM_RANGE(0x0800, 0x08ff) AM_MIRROR(0x0700) AM_RAM AM_SHARE("playfield_ram")
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x001f) AM_WRITEONLY AM_SHARE("scroll_y")
	AM_RANGE(0x1020, 0x1020) AM_MIRROR(0x001f) AM_WRITEONLY AM_SHARE("scroll_x")
	AM_RANGE(0x1040, 0x1040) AM_MIRROR(0x001f) AM_WRITE(montecar_drone_reset_w)
	AM_RANGE(0x1060, 0x1060) AM_MIRROR(0x001f) AM_WRITE(montecar_car_reset_w)
	AM_RANGE(0x1080, 0x1080) AM_MIRROR(0x001f) AM_WRITEONLY AM_SHARE("car_rot")
	AM_RANGE(0x10a0, 0x10a0) AM_MIRROR(0x001f) AM_WRITE(steer_reset_w)
	AM_RANGE(0x10c0, 0x10c0) AM_MIRROR(0x001f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x10e0, 0x10e0) AM_MIRROR(0x001f) AM_WRITE(montecar_skid_reset_w)
	AM_RANGE(0x1400, 0x1400) AM_MIRROR(0x001f) AM_WRITE(firetrk_motor_snd_w)
	AM_RANGE(0x1420, 0x1420) AM_MIRROR(0x001f) AM_WRITE(firetrk_crash_snd_w)
	AM_RANGE(0x1440, 0x1440) AM_MIRROR(0x001f) AM_WRITE(firetrk_skid_snd_w)
	AM_RANGE(0x1460, 0x1460) AM_MIRROR(0x001f) AM_WRITEONLY AM_SHARE("drone_x")
	AM_RANGE(0x1480, 0x1480) AM_MIRROR(0x001f) AM_WRITEONLY AM_SHARE("drone_y")
	AM_RANGE(0x14a0, 0x14a0) AM_MIRROR(0x001f) AM_WRITEONLY AM_SHARE("drone_rot")
	AM_RANGE(0x14c0, 0x14c0) AM_MIRROR(0x001f) AM_WRITE(montecar_output_1_w)
	AM_RANGE(0x14e0, 0x14e0) AM_MIRROR(0x001f) AM_WRITE(montecar_output_2_w)
	AM_RANGE(0x1800, 0x1807) AM_MIRROR(0x03f8) AM_READ(montecar_input_r) AM_WRITENOP
	AM_RANGE(0x1c00, 0x1c03) AM_MIRROR(0x03fc) AM_READ(montecar_dip_r)
	AM_RANGE(0x2000, 0x3fff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( firetrk )
	PORT_START("STEER_1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("STEER_2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("DIP_0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) /* other DIPs connect here */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) /* other DIPs connect here */
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	PORT_START("DIP_1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( French ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Spanish ) )
	PORT_DIPSETTING(    0x03, DEF_STR( German ) )
	PORT_DIPNAME( 0x0c, 0x04, "Play Time" )
	PORT_DIPSETTING(    0x00, "60 Seconds" )
	PORT_DIPSETTING(    0x04, "90 Seconds" )
	PORT_DIPSETTING(    0x08, "120 Seconds" )
	PORT_DIPSETTING(    0x0c, "150 Seconds" )
	PORT_DIPNAME( 0x30, 0x20, "Extended Play" )
	PORT_DIPSETTING(    0x10, "Liberal" )
	PORT_DIPSETTING(    0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x30, "Conservative" )
	PORT_DIPSETTING(    0x00, "Never" )

	PORT_START("BIT_0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Gas") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,steer_dir_r, (void *)nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,steer_dir_r, (void *)1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Bell") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,skid_r, (void *)2)
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Service_Mode ) ) PORT_CODE(KEYCODE_F2) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state,service_mode_switch_changed, 0)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("BIT_6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Front Player Start")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Back Player Start")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START3 ) PORT_NAME("Both Players Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Track Select") PORT_CODE(KEYCODE_SPACE) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ))
	PORT_DIPSETTING(    0x00, "Smokey Joe (1 Player)" )
	PORT_DIPSETTING(    0x40, "Fire Truck (2 Players)" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Diag Hold") PORT_CODE(KEYCODE_F6)

	PORT_START("BIT_7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,steer_flag_r, (void *)nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,steer_flag_r, (void *)1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,crash_r, (void *)2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Diag Step") PORT_CODE(KEYCODE_F1)

	PORT_START("HORN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Horn") PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state,firetrk_horn_changed, 0)

	PORT_START("R27")
	PORT_ADJUSTER( 20, "R27 - Motor Frequency" )
INPUT_PORTS_END


static INPUT_PORTS_START( superbug )
	PORT_START("STEER_1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("DIP_0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) /* other DIPs connect here */
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) /* other DIPs connect here */

	PORT_START("DIP_1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x0c, 0x04, "Play Time" )
	PORT_DIPSETTING(    0x00, "60 seconds" )
	PORT_DIPSETTING(    0x04, "90 seconds" )
	PORT_DIPSETTING(    0x08, "120 seconds" )
	PORT_DIPSETTING(    0x0c, "150 seconds" )
	PORT_DIPNAME( 0x30, 0x20, "Extended Play" )
	PORT_DIPSETTING(    0x10, "Liberal" )
	PORT_DIPSETTING(    0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x30, "Conservative" )
	PORT_DIPSETTING(    0x00, "Never" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( French ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Spanish ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( German ) )

	PORT_START("BIT_0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,gear_r, (void *)1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Gas")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,steer_dir_r, (void *)nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Hiscore Reset") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_SERVICE( 0x20, IP_ACTIVE_HIGH )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,skid_r, (void *)nullptr)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT )

	PORT_START("BIT_7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,gear_r, (void *)2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,gear_r, (void *)nullptr)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,steer_flag_r, (void *)nullptr)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,crash_r, (void *)nullptr)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Track Select") PORT_CODE(KEYCODE_SPACE)

	PORT_START("GEAR")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Gear 1") PORT_CODE(KEYCODE_Z) PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state,gear_changed, (void *)nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Gear 2") PORT_CODE(KEYCODE_X) PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state,gear_changed, (void *)1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Gear 3") PORT_CODE(KEYCODE_C) PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state,gear_changed, (void *)2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Gear 4") PORT_CODE(KEYCODE_V) PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state,gear_changed, (void *)3)

	PORT_START("R62")
	PORT_ADJUSTER( 20, "R62 - Motor Frequency" )
INPUT_PORTS_END


static INPUT_PORTS_START( montecar )
	PORT_START("STEER_1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("DIP_0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) /* other DIPs connect here */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) /* other DIPs connect here */
	PORT_DIPNAME( 0x0c, 0x0c, "Coin 3 Multiplier" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x10, 0x10, "Coin 2 Multiplier" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	PORT_START("DIP_1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x0c, 0x08, "Extended Play" )
	PORT_DIPSETTING(    0x04, "Liberal" )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x00, "Conservative" )
	PORT_DIPSETTING(    0x0c, "Never" )
	PORT_DIPNAME( 0x30, 0x20, "Play Time" )
	PORT_DIPSETTING(    0x30, "60 Seconds" )
	PORT_DIPSETTING(    0x10, "90 Seconds" )
	PORT_DIPSETTING(    0x20, "120 Seconds" )
	PORT_DIPSETTING(    0x00, "150 Seconds" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Language ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( English ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Spanish ) )
	PORT_DIPSETTING(    0x40, DEF_STR( French ) )
	PORT_DIPSETTING(    0x00, DEF_STR( German ) )

	PORT_START("BIT_6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,gear_r, (void *)nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,gear_r, (void *)1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,gear_r, (void *)2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Track Select") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Gas")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,steer_dir_r, (void *)nullptr)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,skid_r, (void *)1)

	PORT_START("BIT_7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_SERVICE( 0x04, IP_ACTIVE_HIGH ) PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state,service_mode_switch_changed, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,steer_flag_r, (void *)nullptr)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, firetrk_state,skid_r, (void *)nullptr)

	PORT_START("GEAR")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Gear 1") PORT_CODE(KEYCODE_Z) PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state,gear_changed, (void *)nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Gear 2") PORT_CODE(KEYCODE_X) PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state,gear_changed, (void *)1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Gear 3") PORT_CODE(KEYCODE_C) PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state,gear_changed, (void *)2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Gear 4") PORT_CODE(KEYCODE_V) PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state,gear_changed, (void *)3)

	PORT_START("R89")
	PORT_ADJUSTER( 20, "R89 - Motor Frequency" )

	PORT_START("R88")
	PORT_ADJUSTER( 25, "R88 - Drone Motor Frequency" )
INPUT_PORTS_END


static const gfx_layout firetrk_text_layout =
{
	16, 16, /* width, height */
	32,     /* total         */
	1,      /* planes        */
	{ 0 },  /* plane offsets */
	{
		0x1c, 0x1d, 0x1e, 0x1f, 0x04, 0x05, 0x06, 0x07,
		0x0c, 0x0d, 0x0e, 0x0f, 0x14, 0x15, 0x16, 0x17
	},
	{
		0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
		0x100, 0x120, 0x140, 0x160, 0x180, 0x1a0, 0x1c0, 0x1e0
	},
	0x200
};


static const gfx_layout superbug_text_layout =
{
	16, 16, /* width, height */
	32,     /* total         */
	1,      /* planes        */
	{ 0 },  /* plane offsets */
	{
		0x0c, 0x0d, 0x0e, 0x0f, 0x14, 0x15, 0x16, 0x17,
		0x1c, 0x1d, 0x1e, 0x1f, 0x04, 0x05, 0x06, 0x07
	},
	{
		0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
		0x100, 0x120, 0x140, 0x160, 0x180, 0x1a0, 0x1c0, 0x1e0
	},
	0x200
};


static const gfx_layout montecar_text_layout =
{
	8, 8,   /* width, height */
	64,     /* total         */
	1,      /* planes        */
	{ 0 },  /* plane offsets */
	{
		0xc, 0xd, 0xe, 0xf, 0x4, 0x5, 0x6, 0x7
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
	},
	0x80
};


static const gfx_layout firetrk_tile_layout =
{
	16, 16, /* width, height */
	64,     /* total         */
	1,      /* planes        */
	{ 0 },  /* plane offsets */
	{
		0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
		0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
		0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0
	},
	0x100
};


static const gfx_layout superbug_tile_layout =
{
	16, 16, /* width, height */
	64,     /* total         */
	1,      /* planes        */
	{ 0 },  /* plane offsets */
	{
		0x07, 0x06, 0x05, 0x04, 0x0f, 0x0e, 0x0d, 0x0c,
		0x17, 0x16, 0x15, 0x14, 0x1f, 0x1e, 0x1d, 0x1c
	},
	{
		0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
		0x100, 0x120, 0x140, 0x160, 0x180, 0x1a0, 0x1c0, 0x1e0
	},
	0x200
};


static const gfx_layout firetrk_car_layout1 =
{
	32, 32, /* width, height */
	4,      /* total         */
	1,      /* planes        */
	{ 0 },  /* plane offsets */
	{
		0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180, 0x1c0,
		0x200, 0x240, 0x280, 0x2c0, 0x300, 0x340, 0x380, 0x3c0,
		0x400, 0x440, 0x480, 0x4c0, 0x500, 0x540, 0x580, 0x5c0,
		0x600, 0x640, 0x680, 0x6c0, 0x700, 0x740, 0x780, 0x7c0
	},
	{
		0x04, 0x05, 0x06, 0x07, 0x0c, 0x0d, 0x0e, 0x0f,
		0x14, 0x15, 0x16, 0x17, 0x1c, 0x1d, 0x1e, 0x1f,
		0x24, 0x25, 0x26, 0x27, 0x2c, 0x2d, 0x2e, 0x2f,
		0x34, 0x35, 0x36, 0x37, 0x3c, 0x3d, 0x3e, 0x3b
	},
	0x800
};


static const gfx_layout superbug_car_layout1 =
{
	32, 32, /* width, height */
	4,      /* total         */
	1,      /* planes        */
	{ 0 },  /* plane offsets */
	{
		0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700,
		0x0800, 0x0900, 0x0a00, 0x0b00, 0x0c00, 0x0d00, 0x0e00, 0x0f00,
		0x1000, 0x1100, 0x1200, 0x1300, 0x1400, 0x1500, 0x1600, 0x1700,
		0x1800, 0x1900, 0x1a00, 0x1b00, 0x1c00, 0x1d00, 0x1e00, 0x1f00
	},
	{
		0x04, 0x0c, 0x14, 0x1c, 0x24, 0x2c, 0x34, 0x3c,
		0x44, 0x4c, 0x54, 0x5c, 0x64, 0x6c, 0x74, 0x7c,
		0x84, 0x8c, 0x94, 0x9c, 0xa4, 0xac, 0xb4, 0xbc,
		0xc4, 0xcc, 0xd4, 0xdc, 0xe4, 0xec, 0xf4, 0xfc
	},
	0x001
};


static const gfx_layout montecar_car_layout =
{
	32, 32, /* width, height */
	8,      /* total         */
	2,      /* planes        */
			/* plane offsets */
	{ 1, 0 },
	{
		0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e,
		0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e,
		0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e,
		0x30, 0x32, 0x34, 0x36, 0x38, 0x3a, 0x3c, 0x3e
	},
	{
		0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180, 0x1c0,
		0x200, 0x240, 0x280, 0x2c0, 0x300, 0x340, 0x380, 0x3c0,
		0x400, 0x440, 0x480, 0x4c0, 0x500, 0x540, 0x580, 0x5c0,
		0x600, 0x640, 0x680, 0x6c0, 0x700, 0x740, 0x780, 0x7c0
	},
	0x800
};


static const gfx_layout firetrk_car_layout2 =
{
	32, 32, /* width, height */
	4,      /* total         */
	1,      /* planes        */
	{ 0 },  /* plane offsets */
	{
		0x04, 0x05, 0x06, 0x07, 0x0c, 0x0d, 0x0e, 0x0f,
		0x14, 0x15, 0x16, 0x17, 0x1c, 0x1d, 0x1e, 0x1f,
		0x24, 0x25, 0x26, 0x27, 0x2c, 0x2d, 0x2e, 0x2f,
		0x34, 0x35, 0x36, 0x37, 0x3c, 0x3d, 0x3e, 0x3b
	},
	{
		0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180, 0x1c0,
		0x200, 0x240, 0x280, 0x2c0, 0x300, 0x340, 0x380, 0x3c0,
		0x400, 0x440, 0x480, 0x4c0, 0x500, 0x540, 0x580, 0x5c0,
		0x600, 0x640, 0x680, 0x6c0, 0x700, 0x740, 0x780, 0x7c0
	},
	0x800
};


static const gfx_layout superbug_car_layout2 =
{
	32, 32, /* width, height */
	4,      /* total         */
	1,      /* planes        */
	{ 0 },  /* plane offsets */
	{
		0x04, 0x0c, 0x14, 0x1c, 0x24, 0x2c, 0x34, 0x3c,
		0x44, 0x4c, 0x54, 0x5c, 0x64, 0x6c, 0x74, 0x7c,
		0x84, 0x8c, 0x94, 0x9c, 0xa4, 0xac, 0xb4, 0xbc,
		0xc4, 0xcc, 0xd4, 0xdc, 0xe4, 0xec, 0xf4, 0xfc
	},
	{
		0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700,
		0x0800, 0x0900, 0x0a00, 0x0b00, 0x0c00, 0x0d00, 0x0e00, 0x0f00,
		0x1000, 0x1100, 0x1200, 0x1300, 0x1400, 0x1500, 0x1600, 0x1700,
		0x1800, 0x1900, 0x1a00, 0x1b00, 0x1c00, 0x1d00, 0x1e00, 0x1f00
	},
	0x001
};

static const UINT32 firetrk_trailer_layout_xoffset[64] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

static const UINT32 firetrk_trailer_layout_yoffset[64] =
{
	0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180, 0x1c0,
	0x200, 0x240, 0x280, 0x2c0, 0x300, 0x340, 0x380, 0x3c0,
	0x400, 0x440, 0x480, 0x4c0, 0x500, 0x540, 0x580, 0x5c0,
	0x600, 0x640, 0x680, 0x6c0, 0x700, 0x740, 0x780, 0x7c0,
	0x800, 0x840, 0x880, 0x8c0, 0x900, 0x940, 0x980, 0x9c0,
	0xa00, 0xa40, 0xa80, 0xac0, 0xb00, 0xb40, 0xb80, 0xbc0,
	0xc00, 0xc40, 0xc80, 0xcc0, 0xd00, 0xd40, 0xd80, 0xdc0,
	0xe00, 0xe40, 0xe80, 0xec0, 0xf00, 0xf40, 0xf80, 0xfc0
};

static const gfx_layout firetrk_trailer_layout =
{
	64, 64, /* width, height */
	8,      /* total         */
	1,      /* planes        */
	{ 0 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	0x1000,
	firetrk_trailer_layout_xoffset,
	firetrk_trailer_layout_yoffset
};


static GFXDECODE_START( firetrk )
	GFXDECODE_ENTRY( "gfx1", 0, firetrk_text_layout, 26, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, firetrk_tile_layout, 0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, firetrk_tile_layout, 16, 3 )
	GFXDECODE_ENTRY( "gfx3", 0, firetrk_car_layout1, 22, 2 )
	GFXDECODE_ENTRY( "gfx3", 0, firetrk_car_layout2, 22, 2 )
	GFXDECODE_ENTRY( "gfx4", 0, firetrk_trailer_layout, 22, 2 )
GFXDECODE_END


static GFXDECODE_START( superbug )
	GFXDECODE_ENTRY( "gfx1", 0, superbug_text_layout, 26, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, superbug_tile_layout, 0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, superbug_tile_layout, 16, 3 )
	GFXDECODE_ENTRY( "gfx3", 0, superbug_car_layout1, 22, 2 )
	GFXDECODE_ENTRY( "gfx3", 0, superbug_car_layout2, 22, 2 )
GFXDECODE_END


static GFXDECODE_START( montecar )
	GFXDECODE_ENTRY( "gfx1", 0, montecar_text_layout, 44, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, firetrk_tile_layout, 0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, firetrk_tile_layout, 16, 4 )
	GFXDECODE_ENTRY( "gfx3", 0, montecar_car_layout, 24, 1 )
	GFXDECODE_ENTRY( "gfx4", 0, montecar_car_layout, 28, 4 )
GFXDECODE_END


static MACHINE_CONFIG_START( firetrk, firetrk_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, MASTER_CLOCK/12) /* 750Khz during service mode */
	MCFG_CPU_PROGRAM_MAP(firetrk_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", firetrk_state, firetrk_scanline, "screen", 0, 1)
	MCFG_WATCHDOG_VBLANK_INIT(5)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(320, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(firetrk_state, screen_update_firetrk)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 28)
	MCFG_PALETTE_INIT_OWNER(firetrk_state, firetrk)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", firetrk)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(firetrk)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( superbug, firetrk )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(superbug_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(firetrk_state, screen_update_superbug)

	MCFG_VIDEO_START_OVERRIDE(firetrk_state,superbug)
	MCFG_GFXDECODE_MODIFY("gfxdecode", superbug)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(28)
	MCFG_PALETTE_INIT_OWNER(firetrk_state, firetrk)

	/* sound hardware */
	MCFG_SOUND_REPLACE("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(superbug)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( montecar, firetrk )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(montecar_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(firetrk_state, screen_update_montecar)

	MCFG_VIDEO_START_OVERRIDE(firetrk_state,montecar)
	MCFG_GFXDECODE_MODIFY("gfxdecode", montecar)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(46)
	MCFG_PALETTE_INIT_OWNER(firetrk_state,montecar)

	/* sound hardware */
	MCFG_SOUND_REPLACE("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(montecar)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( firetrk )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD(          "032823-02.c1", 0x2000, 0x800, CRC(9570bdd3) SHA1(4d26a9490d05d53da55fc59459a4dce5bca6c761) )
	ROM_LOAD(          "032824-01.d1", 0x2800, 0x800, CRC(a5fc5629) SHA1(bf20510d8623eda2740ff296a7813a3e6f7ec76e) )
	ROM_LOAD_NIB_HIGH( "032816-01.k1", 0x3000, 0x800, CRC(c0535598) SHA1(15cb6985b0b22140b7fae1e050e0b63dd4d0f793) )
	ROM_LOAD_NIB_LOW ( "032820-01.k2", 0x3000, 0x800, CRC(5733f9ed) SHA1(0f19a40793dadfb7de2c2b54a44929b414d0f4ed) )
	ROM_LOAD_NIB_HIGH( "032815-01.j1", 0x3800, 0x800, CRC(506ee759) SHA1(d111356c84f3d9942a27fbe243e716d14c258a16) )
	ROM_LOAD_NIB_LOW ( "032819-01.j2", 0x3800, 0x800, CRC(f1c3fa87) SHA1(d75cf4ad0bcac3289c068837fc24cfe84ce7542a) )

	ROM_REGION( 0x0800, "gfx1", 0 ) /* text */
	ROM_LOAD( "032827-01.r3", 0x000, 0x800, CRC(cca31d2b) SHA1(78235176c9cb2abd73a5778b54560b87634ca0e4) )

	ROM_REGION( 0x0800, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "032828-02.f5", 0x000, 0x800, CRC(68ef5f19) SHA1(df227d6a57bba6298ebdeb5a118878da21d889f6) )

	ROM_REGION( 0x0400, "gfx3", 0 ) /* cab */
	ROM_LOAD( "032831-01.p7", 0x000, 0x400, CRC(bb8d144f) SHA1(9a1355ea6f88e96926c32e0e36ac0525b0243906) )

	ROM_REGION( 0x1000, "gfx4", 0 ) /* trailer */
	ROM_LOAD( "032829-01.j5", 0x000, 0x800, CRC(e7267d71) SHA1(7132b98622e899227a378ba8c010dde39c479978) )
	ROM_LOAD( "032830-01.l5", 0x800, 0x800, CRC(e4d8b685) SHA1(30978658899c83e32dabdf554a13cf5e5235c725) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "009114.prm", 0x0000, 0x100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) ) /* sync */
ROM_END


ROM_START( superbug )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "009121.d1", 0x0800, 0x800, CRC(350df308) SHA1(b957c830bb95e0752ea9793e3edcfdd52235e0ab) )
	ROM_LOAD( "009122.c1", 0x1000, 0x800, CRC(eb6e3e37) SHA1(5237f6bd3a7a3eca737c728296230cf0d1f436b0) )
	ROM_LOAD( "009123.a1", 0x1800, 0x800, CRC(f42c6bbe) SHA1(41470984fe951eac9f6dc77862b00ecfe8aaa51d) )

	ROM_REGION( 0x0800, "gfx1", 0 ) /* text */
	ROM_LOAD( "009124.m3", 0x0000, 0x400, CRC(f8af8dd5) SHA1(49ab85550f546f85048e2f73163837c602dde568) )
	ROM_LOAD( "009471.n3", 0x0400, 0x400, CRC(52250698) SHA1(cc55254c54dbcd3fd1465c82a715f2e567f44951) )

	ROM_REGION( 0x1000, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "009126.f5", 0x0000, 0x400, CRC(ee695137) SHA1(295fdfef88e0c841fe8ad505151ca0837e77ef83) )
	ROM_LOAD( "009472.h5", 0x0400, 0x400, CRC(5ddb80ac) SHA1(bdbbbba6efdd4cca75630d203f7c7eaf41b1a32d) )
	ROM_LOAD( "009127.e5", 0x0800, 0x400, CRC(be1386b4) SHA1(17e92df58b25075ec7a383a958db02b42066578a) )
	ROM_RELOAD(          0x0C00, 0x400 )

	ROM_REGION( 0x0400, "gfx3", 0 ) /* car */
	ROM_LOAD( "009125.k6", 0x0000, 0x400, CRC(a3c835df) SHA1(e9b6dba1919c389bb55a8fe3c074b6702322e4e5) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "009114.prm", 0x0000, 0x100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) ) /* sync */
ROM_END


ROM_START( montecar )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "35766-01.h1", 0x2000, 0x800, CRC(d3695f09) SHA1(8aa3b3921acd0d2c3230d610843042613defcba9) )
	ROM_LOAD( "35765-01.f1", 0x2800, 0x800, CRC(9491a7ee) SHA1(712959c5f97be3db7be1d5bd70c780d4da2f6d47) )
	ROM_LOAD( "35764-01.d1", 0x3000, 0x800, CRC(899aaf4e) SHA1(84fab58d135ffc6e4b076d438b4d588b394364b6) )
	ROM_LOAD( "35763-01.c1", 0x3800, 0x800, CRC(378bfe47) SHA1(fd6b28907340a2ffc82a4e634273c3f03ab76642) )

	ROM_REGION( 0x0400, "gfx1", 0 ) /* text */
	ROM_LOAD( "35778-01.m4", 0x0000, 0x400, CRC(294ee08e) SHA1(fbb0656468a027b2795073d811affc93c50994ec) )

	ROM_REGION( 0x0800, "gfx2", 0 ) /* tiles */
	ROM_LOAD( "35775-01.e6", 0x0000, 0x800, CRC(504106e9) SHA1(33eae2cf39b24eaf5b438a2af3060b2fdc0012b5) )

	ROM_REGION( 0x0800, "gfx3", 0 ) /* car */
	ROM_LOAD( "35779-01.m6", 0x0000, 0x800, CRC(4fbb3fe1) SHA1(4267cd098a19892322d21f8fa7b55896158f8d6a) )

	ROM_REGION( 0x0800, "gfx4", 0 ) /* drone */
	ROM_LOAD( "35780-01.b6", 0x0000, 0x800, CRC(9d0f1374) SHA1(52d1130d48dc877e1e47e26b2e4548633ed91b21) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "35785-01.e7", 0x0000, 0x200, CRC(386c543a) SHA1(04edda180e6ff432b438947ffa46621ca0a823b4) ) /* color */
	ROM_LOAD( "9114.prm",    0x0200, 0x100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) ) /* sync */
ROM_END


GAMEL(1977, superbug, 0, superbug, superbug, driver_device, 0, ROT270, "Atari (Kee Games)", "Super Bug", 0, layout_superbug )
GAME( 1978, firetrk,  0, firetrk,  firetrk, driver_device,  0, ROT270, "Atari", "Fire Truck / Smokey Joe", 0 )
GAME( 1979, montecar, 0, montecar, montecar, driver_device, 0, ROT270, "Atari", "Monte Carlo", 0 )
