#include "sc7a20e.h"
#include "hal_gsensor.h"
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define I2C_BUS_PREF "/dev/i2c-"
#define USAGE_MESSAGE                                   \
    "Usage:\n"                                          \
    "  %s r [addr] [register]   "                       \
    "to read value from [register]\n"                   \
    "  %s w [addr] [register] [value]   "               \
    "to write a value [value] to register [register]\n" \
    ""

static int32_t set_i2c_register(int32_t file, unsigned char addr, unsigned char reg,
			    unsigned char value)
{
	unsigned char outbuf[2];
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages[1];

	messages[0].addr = addr;
	messages[0].flags = 0;
	messages[0].len = sizeof(outbuf);
	messages[0].buf = outbuf;

	/* The first byte indicates which register we'll write */
	outbuf[0] = reg;

	/*
	 * The second byte indicates the value to write.  Note that for many
	 * devices, we can write multiple, sequential registers at once by
	 * simply making outbuf bigger.
	 */
	outbuf[1] = value;

	/* Transfer the i2c packets to the kernel and verify it worked */
	packets.msgs = messages;
	packets.nmsgs = 1;
	if (ioctl(file, I2C_RDWR, &packets) < 0) {
		perror("Unable to send data");
		printf("%s:%d\n", __func__,__LINE__);
		return 1;
	}
	printf("%s:%d\n", __func__,__LINE__);
	return 0;
}

static int32_t get_i2c_register(int32_t file, unsigned char addr, unsigned char reg,
			    unsigned char *val)
{
	unsigned char inbuf, outbuf;
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages[2];

	/*
	 * In order to read a register, we first do a "dummy write" by writing
	 * 0 bytes to the register we want to read from.  This is similar to
	 * the packet in set_i2c_register, except it's 1 byte rather than 2.
	 */
	outbuf = reg;
	messages[0].addr = addr;
	messages[0].flags = 0;
	messages[0].len = sizeof(outbuf);
	messages[0].buf = &outbuf;

	/* The data will get returned in this structure */
	messages[1].addr = addr;
	messages[1].flags = I2C_M_RD /* | I2C_M_NOSTART*/;
	messages[1].len = sizeof(inbuf);
	messages[1].buf = &inbuf;

	/* Send the request to the kernel and get the result back */
	packets.msgs = messages;
	packets.nmsgs = 2;
	if (ioctl(file, I2C_RDWR, &packets) < 0) {
		perror("Unable to send data");
		printf("%s:%d\n", __func__,__LINE__);
		return 1;
	}
	*val = inbuf;
	// printf("%s:%d\n", __func__,__LINE__);
	return 0;
}

static int32_t i2c_file = 0;

int32_t gsensor_i2c_bus_init(int32_t busnum)
{
	char busname[15];
	sprintf(busname, "%s%d", I2C_BUS_PREF, busnum);
	if ((i2c_file = open(busname, O_RDWR)) < 0) {
		perror("Unable to open i2c control file");
		printf("========>>>i2c file %s opened failed\n", busname);
		return -1;
	}
	printf("========>>>i2c file %s opened\n", busname);
	return 0;
}

int32_t gsensor_i2c_bus_deinit(void)
{
	close(i2c_file);
	return 0;
}
#define abs(x) (((x) < 0) ? -(x) : (x))
#define IDC_SADDR_G_Sensor_W (0x32>>1)// or 0x30 ==> 需关闭 SDO 内部上拉电阻
#define IDC_SADDR_G_Sensor_R (0x32>>1)// or 0x30
#define IDC_STYPE_G_Sensor IDC2

#ifdef KERNEL_DRV
void __I2CWriteByte8(unsigned char chip_addr, unsigned char reg_addr,
		     unsigned char value);
unsigned char __I2CReadByte8(unsigned char chip_addr, unsigned char reg_addr);

void __I2CWriteByte8(unsigned char chip_addr, unsigned char reg_addr,
		     unsigned char value)
{
	int32_t ret;
	unsigned char buf[2];
	struct i2c_client *client = gsensor_i2c_client;

	gsensor_i2c_client->addr = chip_addr;

	buf[0] = reg_addr;
	buf[1] = value;
	// printf("reg_addr[%2x] value[%2x]\n", reg_addr, value);
	ret = i2c_master_send(client, buf, 2);
	udelay(300);
}

unsigned char __I2CReadByte8(unsigned char chip_addr, unsigned char reg_addr)
{
	int32_t ret_data = 0xFF;
	int32_t ret;
	struct i2c_client *client = gsensor_i2c_client;
	unsigned char buf[2];

	gsensori2c_client->addr = chip_addr;

	buf[0] = reg_addr;
	ret = i2c_master_recv(client, buf, 1);
	if (ret >= 0) {
		ret_data = buf[0];
	}
	return ret_data;
}
#endif
/*return value: 0: is ok    other: is failed*/
int32_t i2c_read_byte_data(unsigned char addr, unsigned char *data)
{
	int32_t ret = 0;
	ret = get_i2c_register(i2c_file, IDC_SADDR_G_Sensor_R, addr, data);
	return ret;
}

/*return value: 0: is ok    other: is failed*/
int32_t i2c_write_byte_data(unsigned char addr, unsigned char data)
{
	int32_t ret = 0;
	ret = set_i2c_register(i2c_file, IDC_SADDR_G_Sensor_W, addr, data);
	return ret;
}

/*return value: 0: is count    other: is failed*/
int32_t i2c_read_block_data(unsigned char base_addr, unsigned char count,
			unsigned char *data)
{
	int32_t i = 0;

	for (i = 0; i < count; i++) {
		if (i2c_read_byte_data(base_addr + i, (data + i))) {
			return -1;
		}
	}

	return count;
}

int32_t sc7a20e_register_read(unsigned char addr, unsigned char *data)
{
	int32_t res = 0;

	res = i2c_read_byte_data(addr, data);
	if (res != 0) {
		return res;
	}

	return res;
}

int32_t sc7a20e_register_write(unsigned char addr, unsigned char data)
{
	int32_t res = 0;

	res = i2c_write_byte_data(addr, data);
	if (res != 0) {
		return res;
	}

	return res;
}

int32_t sc7a20e_register_read_continuously(unsigned char addr, unsigned char count,
				      unsigned char *data)
{
	int32_t res = 0;

	res = (count == i2c_read_block_data(addr, count, data)) ? 0 : 1;
	if (res != 0) {
		return res;
	}

	return res;
}

int32_t sc7a20e_register_mask_write(unsigned char addr, unsigned char mask,
			       unsigned char data)
{
	int32_t res = 0;
	unsigned char tmp_data;

	res = sc7a20e_register_read(addr, &tmp_data);
	if (res) {
		return res;
	}

	tmp_data &= ~mask;
	tmp_data |= data & mask;
	res = sc7a20e_register_write(addr, tmp_data);

	return res;
}

static int32_t sc7a20e_set_enable(char enable)
{
	int32_t res = 0;
	/*
	if (enable)
		res = sc7a20e_register_mask_write(NSA_REG_POWERMODE_BW, 0xC0, 0x40);
	else
		res = sc7a20e_register_mask_write(NSA_REG_POWERMODE_BW, 0xC0, 0x80);
	*/
	printf("------sc7a20e_set_enable----- %d\r\n",enable);
	return res;
}

static int32_t sc7a20e_close_interrupt(void)
{
	int32_t res = 0;
	//res = sc7a20e_register_write(NSA_REG_INTERRUPT_SETTINGS1, 0x00);
	//res = sc7a20e_register_write(NSA_REG_INTERRUPT_MAPPING1, 0x00);
	//res |= sc7a20e_register_write(0x24, 0x20);
	res |= sc7a20e_register_write(0x32, 0x7e);
	res |= sc7a20e_register_write(0x22, 0x00);
	//res |= sc7a20e_register_write(0x24, 0x20); //aoi_en reset ; aoi_lir 0
	printf("------sc7a20e_close_interrupt-----\r\n");
	return res;
}

static int32_t sc7a20e_open_interrupt(int32_t num)
{
	int32_t res = 0;
	unsigned char data;
	#if 0
	res |= sc7a20e_register_write(NSA_REG_INT_PIN_CONFIG, 0x01);
	res |= sc7a20e_register_write(NSA_REG_INTERRUPT_SETTINGS1,0x03);//0x07);//
	res |= sc7a20e_register_write(NSA_REG_INTERRUPT_SETTINGS2,0xA0); // interuppt plus  keep 1 second
	res |= sc7a20e_register_write(NSA_REG_INT_LATCH, 0x06);
	res |= sc7a20e_register_write(NSA_REG_ACTIVE_DURATION,0x01);// 0x03
	res |= sc7a20e_register_write(NSA_REG_ACTIVE_THRESHOLD,0x14);//0x05//0x26////DEBUG//38//DF

	switch(num){
		case 0:
			res = sc7a20e_register_write(NSA_REG_INTERRUPT_MAPPING1,0x04 );
			break;
		case 1:
			res = sc7a20e_register_write(NSA_REG_INTERRUPT_MAPPING3,0x04 );
			break;
	}
	#endif
	//res = sc7a20e_register_write(0x22, 0x40);
	if (num == 0) {
		data = 0x7e;
	} else if (num == 1) {
		data = 0x50;
	} else if (num == 2) {
		data = 0x10;
	} else if (num == 3) {
		data = 0x05;
	} else {
		data = 0x7e;
	}
	res |= sc7a20e_register_write(0x32, data);

	//res |= sc7a20e_register_write(0x23, 0x88);
	//res |= sc7a20e_register_write(0x3b, 0x7f);
	//res |= sc7a20e_register_write(0x3c, 0x60);
	//res |= sc7a20e_register_write(0x38, 0x07);
	if (num == 0) {
		res |= sc7a20e_register_write(0x22, 0x0);
	}
	else {
		res |= sc7a20e_register_write(0x22, 0x40);
	}

	printf("------sc7a20e_open_interrupt----- %d\r\n",num);
	// res = sc7a20e_register_read(NSA_REG_INTERRUPT_SETTINGS1, &data);
	// printf("------sc7a20e_register_read NSA_REG_INTERRUPT_SETTINGS1 %x-----\r\n", data);
	// res = sc7a20e_register_read(NSA_REG_INTERRUPT_MAPPING1, &data);
	// printf("------sc7a20e_register_read NSA_REG_INTERRUPT_MAPPING1 %x-----\r\n", data);

	return res;
}

/**
 * --------------------------------

开机配置建议:

   寄存器地址, 设置值,
   0x1f,0x08,
   0x20,0x47,
   0x23,0x88,    //+-2g
   0x21,0x31,
   0x22,0x40,    //AOI中断on int1
   0x25,0x00,
   0x24,0x00,
   0x30,0x2a,    //x,y,z高事件或检测
   0x32,0x05,    //检测门限: 1-127, 值越小, 灵敏度越高
   0x33,0x00,
   0x58,0x02,

碰撞触发判断:

* 方法一: 读reg31h, reg31h bit6为1则为碰撞触发

* 方法二: reg21h bit3必须置1

   signed char	x,y,z;

   y=RReg(0x29);

   z=RReg(0x2b);

   x=RReg(0x2d);

   if (abs(x)>level || abs(y)>level || abs(z)>level)
     return 1; //有碰撞
   else
     return 0;

--------------------------------

关机配置建议:

   寄存器地址, 设置值,
   0x1f,0x08,
   0x20,0x47,
   0x23,0x80,    //+-2g
   0x21,0x31,
   0x22,0x40,    //AOI中断on int1
   0x25,0x00,
   0x24,0x08,
   0x30,0x2a,    //x,y,z高事件或检测
   0x32,0x10,    //检测门限: 1-127, 值越小, 灵敏度越高
   0x33,0x01,
   0x58,0x02,

停车监控开机后，先读reg31h, reg31h bit6为1则为停车监控开机

--------------------------------
*/
#define CHIP_ID							0x11
/*return value: 0: is ok    other: is failed*/
int32_t gsensor_init(void)
{
	int32_t res = 0;
	unsigned char data = 0;

	sc7a20e_register_read(NSA_REG_WHO_AM_I, &data); //0x0f
	if (data != CHIP_ID/*0x13*/) {
		printf("------sc7a20e read chip id  error= %x-----\r\n", data);
		return -1;
	}

	printf("------sc7a20e chip id = %x-----\r\n", data);

	//res = sc7a20e_register_mask_write(NSA_REG_SPI_I2C, 0x24, 0x24);

	usleep(5000);

/**
	res |= sc7a20e_register_mask_write(NSA_REG_G_RANGE, 0x03, 0x00);
	res |= sc7a20e_register_mask_write(NSA_REG_POWERMODE_BW, 0xFF, 0x34);
	res |= sc7a20e_register_mask_write(NSA_REG_ODR_AXIS_DISABLE, 0xFF, 0x06);
	res |= sc7a20e_register_write(0x16, 0x00);
	res |= sc7a20e_register_write(0x19, 0x0);
	res |= sc7a20e_close_interrupt();
*/
	res |= sc7a20e_close_interrupt();
	res |= sc7a20e_register_write(0x1f, 0x08);
	res |= sc7a20e_register_write(0x20, 0x47);
	res |= sc7a20e_register_write(0x23, 0x88);
	res |= sc7a20e_register_write(0x21, 0x31);
	//res |= sc7a20e_register_write(0x22, 0x40); // ==> sc7a20e_open_interrupt()
	res |= sc7a20e_open_interrupt(3);
	res |= sc7a20e_register_write(0x25, 0x00);
	res |= sc7a20e_register_write(0x24, 0x00);
	res |= sc7a20e_register_write(0x30, 0x2a);
	res |= sc7a20e_register_write(0x32, 0x05);
	res |= sc7a20e_register_write(0x33, 0x00);
	res |= sc7a20e_register_write(0x58, 0x02);
	//res |= sc7a20e_close_interrupt();
	// res |= sc7a20e_open_interrupt(0);
	sc7a20e_set_enable(1);
	return res;
}

int32_t gsensor_deinit(void)
{
	int32_t res = 0;
	//sc7a20e_set_enable(0);
	res |= sc7a20e_register_write(0x1f, 0x08);
	res |= sc7a20e_register_write(0x20, 0x47);
	res |= sc7a20e_register_write(0x23, 0x80);
	res |= sc7a20e_register_write(0x21, 0x31);
	//res |= sc7a20e_register_write(0x22, 0x40); // ==> sc7a20e_close_interrupt()
	//res |= sc7a20e_close_interrupt();
	res |= sc7a20e_register_write(0x25, 0x00);
	//res |= sc7a20e_register_write(0x24, 0x08);
	res |= sc7a20e_register_write(0x30, 0x2a);
	res |= sc7a20e_register_write(0x32, 0x10);
	res |= sc7a20e_register_write(0x33, 0x01);
	res |= sc7a20e_register_write(0x58, 0x02);
	//return 0;

	{
		unsigned char data = 0;

		sc7a20e_register_read(0x31, &data); //0x0f
	}


	return res;
}

int32_t gsensor_set_sensitity(unsigned char num)
{
	int32_t res = 0;
	/*
	res |= sc7a20e_register_write(0x11, 0x34);
	res |= sc7a20e_register_write(0x10, 0x07);
	res |= sc7a20e_register_write(0x16, 0x07);
	res |= sc7a20e_register_write(0x19, 0x04);
	res |= sc7a20e_register_write(0x20, 0x01);
	res |= sc7a20e_register_write(0x21, 0x06);
	res |= sc7a20e_register_write(0x27, 0x01);
	res |= sc7a20e_register_write(0x28, num);
	*/
	res |= sc7a20e_register_write(0x32, num);
	printf("------gsensor_set_sensitity => %x-----\r\n", num);
	return res;
}

/*return value: 1:has intterupt     0: no intterupt*/
int32_t gsensor_read_interrupt_statu(void){

	unsigned char data = 0;

	sc7a20e_register_read(0x31, &data);//(NSA_REG_MOTION_FLAG, &data);
	if((data & 0x40) == 0x40){
		return 1;
	}
	return 0;
}
/*return value: 0: is ok    other: is failed*/
int32_t gsensor_read_data(short *x, short *y, short *z)
{
	unsigned char tmp_data[6] = {0};

	//if (sc7a20e_register_read_continuously(NSA_REG_ACC_X_LSB, 6, tmp_data) !=  0) {
	if (sc7a20e_register_read_continuously(0x28, 6, tmp_data) !=  0) {
		return -1;
	}

	#if 0
	*x = ((short)(tmp_data[1] << 8 | tmp_data[0])) >> 4;
	*y = ((short)(tmp_data[3] << 8 | tmp_data[2])) >> 4;
	*z = ((short)(tmp_data[5] << 8 | tmp_data[4])) >> 4;
	#endif
	*x = tmp_data[1];
	*y = tmp_data[3];
	*z = tmp_data[5];

	return 0;
}

int32_t gsensor_read_int_status(unsigned char *flag)
{
	unsigned char data = 0;
	int32_t ret = 0;
	//ret = sc7a20e_register_read(NSA_REG_MOTION_FLAG, &data);
	//if (data & 0x04) {
	sc7a20e_register_read(0x31, &data);//(NSA_REG_MOTION_FLAG, &data);
	if((data & 0x40) == 0x40){
		*flag = 1;
	} else {
		*flag = 0;
	}
	return ret;
}


HAL_GSENSOR_OBJ_S gsensorObj = {
	.i2c_bus_init = gsensor_i2c_bus_init,
	.i2c_bus_deinit = gsensor_i2c_bus_deinit,
	.init = gsensor_init,
	.deinit = gsensor_deinit,
	.read_data = gsensor_read_data,
	.set_sensitity = gsensor_set_sensitity,
	.read_int_status = gsensor_read_int_status,
	.open_interrupt = sc7a20e_open_interrupt,
	.read_interrupt = gsensor_read_interrupt_statu,
};
#ifdef KERNEL_DRV
#define I2C_0 (0)
#define I2C_1 (1)
#define I2C_2 (2)
#define I2C_3 (3)

struct i2c_client *gsensor_i2c_client;

static struct i2c_board_info hi_info = {
	I2C_BOARD_INFO("gsensor", 0x64),
};

int32_t gsensor_open(struct inode *inode, struct file *file);
int32_t gsensor_close(struct inode *inode, struct file *file);
long gsensor_ioctl(struct file *file, uint32_t cmd, unsigned long arg);

/*******************************************************************************************************
 * Initialize Function
 *
 *******************************************************************************************************/
int32_t gsensor_open(struct inode *inode, struct file *file)
{
	return 0;
}

int32_t gsensor_close(struct inode *inode, struct file *file)
{
	return 0;
}

long gsensor_ioctl(struct file *file, uint32_t cmd, unsigned long arg)
{


	return 0;
}
static int32_t i2c_client_init(void)
{
	struct i2c_adapter *i2c_adap;

	i2c_adap = i2c_get_adapter(I2C_1);
	gsensor_i2c_client = i2c_new_device(i2c_adap, &hi_info);
	i2c_put_adapter(i2c_adap);

	return 0;
}

static void i2c_client_exit(void)
{
	i2c_unregister_device(gsensor_i2c_client);
}

static struct file_operations gsensor_fops = {.owner = THIS_MODULE,
	       .unlocked_ioctl = gsensor_ioctl,
		.open = gsensor_open,
		 .release = gsensor_close
};

static struct miscdevice gsensor_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gsensor",
	.fops = &gsensor_fops,
};

static int32_t __init gsensor_module_init(void)
{
	int32_t ret = 0;
	unsigned char chip_id = 0xFF;

	ret = misc_register(&gsensor_dev);

	if (ret) {
		printf("ERROR: could not register gsensor devices:%#x \n", ret);
		return -1;
	}

	i2c_client_init();

	gsensor_init();
}

/*******************************************************************************
 *	Description		: It is called when "rmmod nvp61XX_ex.ko"
 *command run Argurments		: void Return value	: void Modify :
 *warning			:
 *******************************************************************************/
static void __exit gsensor_module_exit(void)
{
	misc_deregister(&gsensor_dev);
	i2c_client_exit();

	printf("%s\n", __func__);
}

module_init(gsensor_module_init);
module_exit(gsensor_module_exit);

MODULE_LICENSE("GPL");

/*******************************************************************************
 *	End of file
 *******************************************************************************/
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
