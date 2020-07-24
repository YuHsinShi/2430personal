struct ssv6xxx_gpio_structure
{
		int address;
		int data;
};

static const struct ssv6xxx_gpio_structure gpio_setting[]={
	/* Cabrio E: GPIO setting */
	//SMAC_REG_WRITE(sh, ADR_PAD53, 0x21);
	{0xc0000388, 0x00000021},
	//SMAC_REG_WRITE(sh, ADR_PAD54, 0x3000);
	{0xc000038c, 0x00003000},
	//SMAC_REG_WRITE(sh, ADR_PIN_SEL_0, 0x4000);
	{0xc00003cc, 0x00004000},
	/* TR switch: */
	{0xc0000304, 0x00000001},
	{0xc0000308, 0x00000001},
	/* PMU setting */
	{0xc0001d08, 0x00000001},
};
