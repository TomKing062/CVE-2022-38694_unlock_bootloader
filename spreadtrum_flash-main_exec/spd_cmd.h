
#define HDLC_HEADER 0x7e
#define HDLC_ESCAPE 0x7d

/*

SC6531EFM.xml (SC6531E):
	FDL1 = 0x40004000
	FDL = 0x14000000

NOR_FLASH_SC6530.xml (SC6531DA):
	FDL = 0x34000000

IDs:
	BOOTLOADER = 0x80000000
	PS = 0x80000003
	NV = 0x90000001
	PHASE_CHECK = 0x90000002, 0x1000
	FLASH = 0x90000003, 0xc0000
	MMIRES = 0x90000004
	ERASE_UDISK = 0x90000005
	UDISK_IMG = 0x90000006
	DSP_CODE = 0x90000009

FDL1:
	BSL_CMD_CONNECT()
	BSL_SET_BAUDRATE(u32 baud)
	BSL_CMD_START_DATA(u32 start_addr, u32 file_size)
	BSL_CMD_MIDST_DATA(...)
	BSL_CMD_END_DATA()
	BSL_CMD_EXEC_DATA()
	unknown: bootPanic

FDL2:
	BSL_CMD_CONNECT()
	BSL_CMD_START_DATA(u32 start_addr, u32 file_size)
	BSL_CMD_MIDST_DATA(...)
	BSL_CMD_END_DATA()
	BSL_CMD_NORMAL_RESET()
	BSL_CMD_READ_FLASH(u32 addr, u32 size, u32 offset)
	BSL_REPARTITION(): nop
	BSL_ERASE_FLASH(u32 addr, u32 size)
	BSL_CMD_POWER_OFF()
	unknown: BSL_REP_INVALID_CMD, bootPanic
*/

enum {
	/* Link Control */    
	BSL_CMD_CONNECT              = 0x00,

	/* Data Download */
	BSL_CMD_START_DATA           = 0x01, /* The start flag of the data downloading */
	BSL_CMD_MIDST_DATA           = 0x02, /* The midst flag of the data downloading */
	BSL_CMD_END_DATA             = 0x03, /* The end flag of the data downloading */
	BSL_CMD_EXEC_DATA            = 0x04, /* Execute from a certain address */

 	BSL_CMD_NORMAL_RESET         = 0x05, /* Reset to normal mode */
	BSL_CMD_READ_FLASH           = 0x06, /* Read flash content */
	BSL_CMD_READ_CHIP_TYPE       = 0x07, /* Read chip type */
	BSL_CMD_READ_NVITEM          = 0x08, /* Lookup a nvitem in specified area */
	BSL_CMD_CHANGE_BAUD          = 0x09, /* Change baudrate */
	BSL_CMD_ERASE_FLASH          = 0x0A, /* Erase an area of flash */
	BSL_CMD_REPARTITION          = 0x0B, /* Repartition nand flash */
	BSL_CMD_READ_FLASH_TYPE      = 0x0C, /* Read flash type */
	BSL_CMD_READ_FLASH_INFO      = 0x0D, /* Read flash infomation */
	BSL_CMD_READ_SECTOR_SIZE     = 0x0F, /* Read Nor flash sector size */
	BSL_CMD_READ_START           = 0x10, /* Read flash start */
	BSL_CMD_READ_MIDST           = 0x11, /* Read flash midst */
	BSL_CMD_READ_END             = 0x12, /* Read flash end */

	BSL_CMD_KEEP_CHARGE          = 0x13, /* Keep charge */
	BSL_CMD_EXTTABLE             = 0x14, /* Set ExtTable */
	BSL_CMD_READ_FLASH_UID       = 0x15, /* Read flash UID */
	BSL_CMD_READ_SOFTSIM_EID     = 0x16, /* Read softSIM EID */
	BSL_CMD_POWER_OFF            = 0x17, /* Power Off */
	BSL_CMD_CHECK_ROOT           = 0x19, /* Check Root */
	BSL_CMD_READ_CHIP_UID        = 0x1A, /* Read Chip UID */
	BSL_CMD_ENABLE_WRITE_FLASH   = 0x1B, /* Enable flash */
	BSL_CMD_ENABLE_SECUREBOOT    = 0x1C, /* Enable secure boot */   
	BSL_CMD_IDENTIFY_START       = 0x1D, /* Identify start */   
	BSL_CMD_IDENTIFY_END         = 0x1E, /* Identify end */   
	BSL_CMD_READ_CU_REF          = 0x1F, /* Read CU ref */ 
	BSL_CMD_READ_REFINFO         = 0x20, /* Read Ref Info */
	BSL_CMD_DISABLE_TRANSCODE    = 0x21, /* Use the non-escape function */
	BSL_CMD_WRITE_DATETIME       = 0x22, /* Write pac file build time to miscdata */
	BSL_CMD_CUST_DUMMY           = 0x23, /* Customized Dummy */
	BSL_CMD_READ_RF_TRANSCEIVER_TYPE = 0x24, /* Read RF transceiver type */
	BSL_CMD_SET_DEBUGINFO        = 0x25,
	BSL_CMD_DDR_CHECK            = 0x26,
	BSL_CMD_SELF_REFRESH         = 0x27,
	BSL_CMD_WRITE_RAW_DATA_ENABLE = 0x28, /* Init for 0x31 and 0x33 */
	BSL_CMD_READ_NAND_BLOCK_INFO = 0x29,
	BSL_CMD_SET_FIRST_MODE       = 0x2A,
	BSL_CMD_READ_PARTITION       = 0x2D, /* Partition list */
	BSL_CMD_DLOAD_RAW_START      = 0x31, /* Raw packet */
	BSL_CMD_WRITE_FLUSH_DATA     = 0x32,
	BSL_CMD_DLOAD_RAW_START2     = 0x33, /* Whole raw file */
	BSL_CMD_READ_LOG             = 0x35,

	BSL_CMD_CHECK_BAUD           = 0x7E, /* CheckBaud command, for internal use */
	BSL_CMD_END_PROCESS          = 0x7F, /* End flash process */

	/* response from the phone */
	BSL_REP_ACK                  = 0x80, /* The operation acknowledge */     
	BSL_REP_VER                  = 0x81,
	BSL_REP_INVALID_CMD          = 0x82,
	BSL_REP_UNKNOW_CMD           = 0x83,
	BSL_REP_OPERATION_FAILED     = 0x84,

	/* Link Control */    
	BSL_REP_NOT_SUPPORT_BAUDRATE = 0x85,

	/* Data Download */ 
	BSL_REP_DOWN_NOT_START       = 0x86,
	BSL_REP_DOWN_MULTI_START     = 0x87,
	BSL_REP_DOWN_EARLY_END       = 0x88,
	BSL_REP_DOWN_DEST_ERROR      = 0x89,
	BSL_REP_DOWN_SIZE_ERROR      = 0x8A,
	BSL_REP_VERIFY_ERROR         = 0x8B,
	BSL_REP_NOT_VERIFY           = 0x8C,

	/* Phone Internal Error */
	BSL_PHONE_NOT_ENOUGH_MEMORY  = 0x8D,
	BSL_PHONE_WAIT_INPUT_TIMEOUT = 0x8E,

	/* Phone Internal return value */
	BSL_PHONE_SUCCEED            = 0x8F,
	BSL_PHONE_VALID_BAUDRATE     = 0x90,
	BSL_PHONE_REPEAT_CONTINUE    = 0x91,
	BSL_PHONE_REPEAT_BREAK       = 0x92,

	/* End of the Command can be transmited by phone */
	BSL_REP_READ_FLASH           = 0x93,
	BSL_REP_READ_CHIP_TYPE       = 0x94,
	BSL_REP_READ_NVITEM          = 0x95,

	BSL_REP_INCOMPATIBLE_PARTITION = 0x96,
	BSL_REP_SIGN_VERIFY_ERROR    = 0xA6,
	BSL_REP_READ_CHIP_UID        = 0xAB,
	BSL_REP_READ_PARTITION       = 0xBA,
	BSL_REP_READ_LOG             = 0xBB,
	BSL_REP_UNSUPPORTED_COMMAND  = 0xFE,
};
