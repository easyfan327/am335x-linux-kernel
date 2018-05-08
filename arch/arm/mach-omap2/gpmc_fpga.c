#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/mtd/physmap.h>
#include <linux/mtd/partitions.h>

#include <plat/board.h>
#include <plat/gpmc.h>

//#define STNOR_GPMC_CONFIG1	0x28601000
//#define STNOR_GPMC_CONFIG2	0x00011001
//#define STNOR_GPMC_CONFIG3	0x00020201
//#define STNOR_GPMC_CONFIG4	0x08031003
//#define STNOR_GPMC_CONFIG5	0x000f1111
//#define STNOR_GPMC_CONFIG6	0x0f030080

#define GPMC_CS 1


#if 0
# define STNOR_GPMC_CONFIG1	0x00001200
# define STNOR_GPMC_CONFIG2	0x00101000
# define STNOR_GPMC_CONFIG3	0x00030301
# define STNOR_GPMC_CONFIG4	0x10041004
# define STNOR_GPMC_CONFIG5	0x000C1010
# define STNOR_GPMC_CONFIG6	0x08070280
# define STNOR_GPMC_CONFIG7	0x00000F41
#else
#define STNOR_GPMC_CONFIG1 0x28001000    //地址数据复用同步通信模式  0x28001000 0x28001200
#define STNOR_GPMC_CONFIG2 0x00070700    //片选时间参数配置
#define STNOR_GPMC_CONFIG3 0x11010100    //地址锁存时间配置
#define STNOR_GPMC_CONFIG4 0x06021611    //读写时间参数配置  0x06021111
#define STNOR_GPMC_CONFIG5 0x01050606
#define STNOR_GPMC_CONFIG6 0x03030101    //0x030301c1
#endif

static const u32 gpmc_nor[7] = {
STNOR_GPMC_CONFIG1,
STNOR_GPMC_CONFIG2,
STNOR_GPMC_CONFIG3,
STNOR_GPMC_CONFIG4,
STNOR_GPMC_CONFIG5,
STNOR_GPMC_CONFIG6, 
0
};

static struct mtd_partition board_fpga_parts[] = {
	[0] = {
		.name = "spl",
		.offset = 0,
		.size  = SZ_1M,
	},
};
static void board_fpga_set_vpp(struct platform_device *pdev,int on)
{

}
static struct physmap_flash_data board_fpga_data = {
	.width = 2,
	.parts = board_fpga_parts,
	.nr_parts = ARRAY_SIZE(board_fpga_parts),
	.set_vpp = board_fpga_set_vpp,
};
static struct resource gpmc_fpga_resource = {
	.flags = IORESOURCE_MEM,
	.start = 0x02000000,
};

static struct platform_device gpmc_fpga_device = {
	.name = "fpga-flash",
	.id = -1,
	.num_resources = 1,
	.resource = &gpmc_fpga_resource,
	.dev = {
		.platform_data = &board_fpga_data,
	}
};	

static int omap2_fpga_gpmc_config(int cs)
{
	//gpmc_cs_write_reg(cs,GPMC_CS_CONFIG1,(2 << 23)|(0 << 22)|(0 << 21)|(0 << 18)|(1 << 12)|(2 << 8));
	//gpmc_cs_write_reg(cs,GPMC_CS_CONFIG2,(16 << 16)|(16 << 8)|(3));
	//gpmc_cs_write_reg(cs,GPMC_CS_CONFIG3,(2 << 16)|(2 << 8)|(1 << 0));
	//gpmc_cs_write_reg(cs,GPMC_CS_CONFIG4,(15 << 24)|(4 << 16)|(16 << 8)|(4 << 0)); 
	//gpmc_cs_write_reg(cs,GPMC_CS_CONFIG5,(15 << 16)|(17 << 8)|(17 << 0));
	//gpmc_cs_write_reg(cs,GPMC_CS_CONFIG6,(15 << 24)|(7 << 16)); 

	gpmc_cs_write_reg(cs, GPMC_CS_CONFIG1, gpmc_nor[0]);
	gpmc_cs_write_reg(cs, GPMC_CS_CONFIG2, gpmc_nor[1]);
	gpmc_cs_write_reg(cs, GPMC_CS_CONFIG3, gpmc_nor[2]);
	gpmc_cs_write_reg(cs, GPMC_CS_CONFIG4, gpmc_nor[3]);
	gpmc_cs_write_reg(cs, GPMC_CS_CONFIG5, gpmc_nor[4]);
	gpmc_cs_write_reg(cs, GPMC_CS_CONFIG6, gpmc_nor[5]);
	gpmc_cs_write_reg(cs, GPMC_CS_CONFIG7, 0x00000F01);
	printk("GPMC_CS_CONFIG7 2 = 0x%08X\n", gpmc_cs_read_reg(cs, GPMC_CS_CONFIG7));


	return 0;
}
int __init gpmc_fpga_init(void *data)//no data
{
	unsigned long phy_base;
	int err = 0;
	u32 val = 0;
	
	printk("gpmc_fpga_init.\n");
	val = gpmc_cs_read_reg(GPMC_CS, GPMC_CS_CONFIG7);
	val &= ~GPMC_CONFIG7_CSVALID;
	gpmc_cs_write_reg(GPMC_CS, GPMC_CS_CONFIG7, val);
	printk("GPMC_CS_CONFIG7 1 = 0x%08X\n", gpmc_cs_read_reg(GPMC_CS, GPMC_CS_CONFIG7));

	err = omap2_fpga_gpmc_config(GPMC_CS);
	if(err < 0)
	{
		dev_err(&gpmc_fpga_device.dev,"gpmc nor cannot set retime\n");
		goto out;
	}

	err = gpmc_cs_request(GPMC_CS,SZ_1M,&phy_base);
	if(err < 0)
	{
	     dev_err(&gpmc_fpga_device.dev,"gpmc NOR cannot allocate CS\n");
	     return err;
	}
	printk("GPMC_CS_CONFIG7 3 = 0x%08X\n", gpmc_cs_read_reg(GPMC_CS, GPMC_CS_CONFIG7));
	gpmc_fpga_resource.start = phy_base;
	gpmc_fpga_resource.end = phy_base + SZ_1M - 1;
	
	err = platform_device_register(&gpmc_fpga_device);
	if(err < 0)
	{
		dev_err(&gpmc_fpga_device.dev,"gpmc nor register device failure\n");
		goto out;
	}
	return 0;
out:	
	gpmc_cs_free(0);
	return err;
}
