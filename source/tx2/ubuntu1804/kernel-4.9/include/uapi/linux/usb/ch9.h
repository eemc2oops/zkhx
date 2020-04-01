/*
 * This file holds USB constants and structures that are needed for
 * USB device APIs.  These are used by the USB device model, which is
 * defined in chapter 9 of the USB 2.0 specification and in the
 * Wireless USB 1.0 (spread around).  Linux has several APIs in C that
 * need these:
 *
 * - the master/host side Linux-USB kernel driver API;
 * - the "usbfs" user space API; and
 * - the Linux "gadget" slave/device/peripheral side driver API.
 *
 * USB 2.0 adds an additional "On The Go" (OTG) mode, which lets systems
 * act either as a USB master/host or as a USB slave/device.  That means
 * the master and slave side APIs benefit from working well together.
 *
 * There's also "Wireless USB", using low power short range radios for
 * peripheral interconnection but otherwise building on the USB framework.
 *
 * Note all descriptors are declared '__attribute__((packed))' so that:
 *
 * [a] they never get padded, either internally (USB spec writers
 *     probably handled that) or externally;
 *
 * [b] so that accessing bigger-than-a-bytes fields will never
 *     generate bus errors on any platform, even when the location of
 *     its descriptor inside a bundle isn't "naturally aligned", and
 *
 * [c] for consistency, removing all doubt even when it appears to
 *     someone that the two other points are non-issues for that
 *     particular descriptor type.
 */

#ifndef _UAPI__LINUX_USB_CH9_H
#define _UAPI__LINUX_USB_CH9_H

#include <linux/types.h>	/* __u8 etc */
#include <asm/byteorder.h>	/* le16_to_cpu */

/*-------------------------------------------------------------------------*/

/* CONTROL REQUEST SUPPORT */

/*
 * USB directions
 *
 * This bit flag is used in endpoint descriptors' bEndpointAddress field.
 * It's also one of three fields in control requests bRequestType.
 */
#define USB_DIR_OUT			0		/* to device */
#define USB_DIR_IN			0x80		/* to host */

/*
 * USB types, the second of three bRequestType fields
 */
#define USB_TYPE_MASK			(0x03 << 5)
#define USB_TYPE_STANDARD		(0x00 << 5)
#define USB_TYPE_CLASS			(0x01 << 5)
#define USB_TYPE_VENDOR			(0x02 << 5)
#define USB_TYPE_RESERVED		(0x03 << 5)

/*
 * USB recipients, the third of three bRequestType fields
 */
#define USB_RECIP_MASK			0x1f
#define USB_RECIP_DEVICE		0x00
#define USB_RECIP_INTERFACE		0x01
#define USB_RECIP_ENDPOINT		0x02
#define USB_RECIP_OTHER			0x03
/* From Wireless USB 1.0 */
#define USB_RECIP_PORT			0x04
#define USB_RECIP_RPIPE		0x05

/*
 * Standard requests, for the bRequest field of a SETUP packet.
 *
 * These are qualified by the bRequestType field, so that for example
 * TYPE_CLASS or TYPE_VENDOR specific feature flags could be retrieved
 * by a GET_STATUS request.
 */
#define USB_REQ_GET_STATUS		0x00
#define USB_REQ_CLEAR_FEATURE		0x01
#define USB_REQ_SET_FEATURE		0x03
#define USB_REQ_SET_ADDRESS		0x05
#define USB_REQ_GET_DESCRIPTOR		0x06
#define USB_REQ_SET_DESCRIPTOR		0x07
#define USB_REQ_GET_CONFIGURATION	0x08
#define USB_REQ_SET_CONFIGURATION	0x09
#define USB_REQ_GET_INTERFACE		0x0A
#define USB_REQ_SET_INTERFACE		0x0B
#define USB_REQ_SYNCH_FRAME		0x0C
#define USB_REQ_SET_SEL			0x30
#define USB_REQ_SET_ISOCH_DELAY		0x31

#define USB_REQ_SET_ENCRYPTION		0x0D	/* Wireless USB */
#define USB_REQ_GET_ENCRYPTION		0x0E
#define USB_REQ_RPIPE_ABORT		0x0E
#define USB_REQ_SET_HANDSHAKE		0x0F
#define USB_REQ_RPIPE_RESET		0x0F
#define USB_REQ_GET_HANDSHAKE		0x10
#define USB_REQ_SET_CONNECTION		0x11
#define USB_REQ_SET_SECURITY_DATA	0x12
#define USB_REQ_GET_SECURITY_DATA	0x13
#define USB_REQ_SET_WUSB_DATA		0x14
#define USB_REQ_LOOPBACK_DATA_WRITE	0x15
#define USB_REQ_LOOPBACK_DATA_READ	0x16
#define USB_REQ_SET_INTERFACE_DS	0x17

/* specific requests for USB Power Delivery */
#define USB_REQ_GET_PARTNER_PDO		20
#define USB_REQ_GET_BATTERY_STATUS	21
#define USB_REQ_SET_PDO			22
#define USB_REQ_GET_VDM			23
#define USB_REQ_SEND_VDM		24

/* The Link Power Management (LPM) ECN defines USB_REQ_TEST_AND_SET command,
 * used by hubs to put ports into a new L1 suspend state, except that it
 * forgot to define its number ...
 */

/*
 * USB feature flags are written using USB_REQ_{CLEAR,SET}_FEATURE, and
 * are read as a bit array returned by USB_REQ_GET_STATUS.  (So there
 * are at most sixteen features of each type.)  Hubs may also support a
 * new USB_REQ_TEST_AND_SET_FEATURE to put ports into L1 suspend.
 */
#define USB_DEVICE_SELF_POWERED		0	/* (read only) */
#define USB_DEVICE_REMOTE_WAKEUP	1	/* dev may initiate wakeup */
#define USB_DEVICE_TEST_MODE		2	/* (wired high speed only) */
#define USB_DEVICE_BATTERY		2	/* (wireless) */
#define USB_DEVICE_B_HNP_ENABLE		3	/* (otg) dev may initiate HNP */
#define USB_DEVICE_WUSB_DEVICE		3	/* (wireless)*/
#define USB_DEVICE_A_HNP_SUPPORT	4	/* (otg) RH port supports HNP */
#define USB_DEVICE_A_ALT_HNP_SUPPORT	5	/* (otg) other RH port does */
#define USB_DEVICE_DEBUG_MODE		6	/* (special devices only) */

/*
 * Test Mode Selectors
 * See USB 2.0 spec Table 9-7
 */
#define	TEST_J		1
#define	TEST_K		2
#define	TEST_SE0_NAK	3
#define	TEST_PACKET	4
#define	TEST_FORCE_EN	5

/*
 * New Feature Selectors as added by USB 3.0
 * See USB 3.0 spec Table 9-7
 */
#define USB_DEVICE_U1_ENABLE	48	/* dev may initiate U1 transition */
#define USB_DEVICE_U2_ENABLE	49	/* dev may initiate U2 transition */
#define USB_DEVICE_LTM_ENABLE	50	/* dev may send LTM */
#define USB_INTRF_FUNC_SUSPEND	0	/* function suspend */

#define USB_INTR_FUNC_SUSPEND_OPT_MASK	0xFF00
/*
 * Suspend Options, Table 9-8 USB 3.0 spec
 */
#define USB_INTRF_FUNC_SUSPEND_LP	(1 << (8 + 0))
#define USB_INTRF_FUNC_SUSPEND_RW	(1 << (8 + 1))

/*
 * Interface status, Figure 9-5 USB 3.0 spec
 */
#define USB_INTRF_STAT_FUNC_RW_CAP     1
#define USB_INTRF_STAT_FUNC_RW         2

#define USB_ENDPOINT_HALT		0	/* IN/OUT will STALL */

/* Bit array elements as returned by the USB_REQ_GET_STATUS request. */
#define USB_DEV_STAT_U1_ENABLED		2	/* transition into U1 state */
#define USB_DEV_STAT_U2_ENABLED		3	/* transition into U2 state */
#define USB_DEV_STAT_LTM_ENABLED	4	/* Latency tolerance messages */

/*
 * Feature selectors from Table 9-8 USB Power Delivery spec
 */
#define USB_DEVICE_BATTERY_WAKE_MASK	40
#define USB_DEVICE_OS_IS_PD_AWARE	41
#define USB_DEVICE_POLICY_MODE		42
#define USB_PORT_PR_SWAP		43
#define USB_PORT_GOTO_MIN		44
#define USB_PORT_RETURN_POWER		45
#define USB_PORT_ACCEPT_PD_REQUEST	46
#define USB_PORT_REJECT_PD_REQUEST	47
#define USB_PORT_PORT_PD_RESET		48
#define USB_PORT_C_PORT_PD_CHANGE	49
#define USB_PORT_CABLE_PD_RESET		50
#define USB_DEVICE_CHARGING_POLICY	54

/**
 * struct usb_ctrlrequest - SETUP data for a USB device control request
 * @bRequestType: matches the USB bmRequestType field
 * @bRequest: matches the USB bRequest field
 * @wValue: matches the USB wValue field (le16 byte order)
 * @wIndex: matches the USB wIndex field (le16 byte order)
 * @wLength: matches the USB wLength field (le16 byte order)
 *
 * This structure is used to send control requests to a USB device.  It matches
 * the different fields of the USB 2.0 Spec section 9.3, table 9-2.  See the
 * USB spec for a fuller description of the different fields, and what they are
 * used for.
 *
 * Note that the driver for any interface can issue control requests.
 * For most devices, interfaces don't coordinate with each other, so
 * such requests may be made at any time.
 */
struct usb_ctrlrequest {
	__u8 bRequestType;
	__u8 bRequest;
	__le16 wValue;
	__le16 wIndex;
	__le16 wLength;
} __attribute__ ((packed));

/*-------------------------------------------------------------------------*/

/*
 * STANDARD DESCRIPTORS ... as returned by GET_DESCRIPTOR, or
 * (rarely) accepted by SET_DESCRIPTOR.
 *
 * Note that all multi-byte values here are encoded in little endian
 * byte order "on the wire".  Within the kernel and when exposed
 * through the Linux-USB APIs, they are not converted to cpu byte
 * order; it is the responsibility of the client code to do this.
 * The single exception is when device and configuration descriptors (but
 * not other descriptors) are read from usbfs (i.e. /proc/bus/usb/BBB/DDD);
 * in this case the fields are converted to host endianness by the kernel.
 */

/*
 * Descriptor types ... USB 2.0 spec table 9.5
 */
#define USB_DT_DEVICE			0x01
#define USB_DT_CONFIG			0x02
#define USB_DT_STRING			0x03
#define USB_DT_INTERFACE		0x04
#define USB_DT_ENDPOINT			0x05
#define USB_DT_DEVICE_QUALIFIER		0x06
#define USB_DT_OTHER_SPEED_CONFIG	0x07
#define USB_DT_INTERFACE_POWER		0x08
/* these are from a minor usb 2.0 revision (ECN) */
#define USB_DT_OTG			0x09
#define USB_DT_DEBUG			0x0a
#define USB_DT_INTERFACE_ASSOCIATION	0x0b
/* these are from the Wireless USB spec */
#define USB_DT_SECURITY			0x0c
#define USB_DT_KEY			0x0d
#define USB_DT_ENCRYPTION_TYPE		0x0e
#define USB_DT_BOS			0x0f
#define USB_DT_DEVICE_CAPABILITY	0x10
#define USB_DT_WIRELESS_ENDPOINT_COMP	0x11
#define USB_DT_WIRE_ADAPTER		0x21
#define USB_DT_RPIPE			0x22
#define USB_DT_CS_RADIO_CONTROL		0x23
/* From the T10 UAS specification */
#define USB_DT_PIPE_USAGE		0x24
/* From the USB 3.0 spec */
#define	USB_DT_SS_ENDPOINT_COMP		0x30
/* From the USB 3.1 spec */
#define	USB_DT_SSP_ISOC_ENDPOINT_COMP	0x31

/* Conventional codes for class-specific descriptors.  The convention is
 * defined in the USB "Common Class" Spec (3.11).  Individual class specs
 * are authoritative for their usage, not the "common class" writeup.
 */
#define USB_DT_CS_DEVICE		(USB_TYPE_CLASS | USB_DT_DEVICE)
#define USB_DT_CS_CONFIG		(USB_TYPE_CLASS | USB_DT_CONFIG)
#define USB_DT_CS_STRING		(USB_TYPE_CLASS | USB_DT_STRING)
#define USB_DT_CS_INTERFACE		(USB_TYPE_CLASS | USB_DT_INTERFACE)
#define USB_DT_CS_ENDPOINT		(USB_TYPE_CLASS | USB_DT_ENDPOINT)

/* All standard descriptors have these 2 fields at the beginning */
struct usb_descriptor_header {
	__u8  bLength;
	__u8  bDescriptorType;
} __attribute__ ((packed));


/*-------------------------------------------------------------------------*/

/* USB_DT_DEVICE: Device descriptor */
// usb设备描述符，协议规范，与设备交互用
struct usb_device_descriptor {
	__u8  bLength;    // 此描述表的字节数
	__u8  bDescriptorType; // 描述符的类型（此处应为0x01，即设备描述符）

	__le16 bcdUSB;  // 此设备与描述表兼容的USB设备说明版本号
	__u8  bDeviceClass; /*设备类码：
						如果此域的值为0则一个设置下每个接口指出它自己的类，各个接口各自独立工作。 
						如果此域的值处于1~FEH之间，则设备在不同的接口上支持不同的类。并这些接口可能不能独立工作。
						此值指出了这些接口集体的类定义。 
						如果此域设为FFH，则此设备的类由厂商定义*/
	__u8  bDeviceSubClass;  /*子类掩码 
							这些码值的具体含义根据bDeviceClass 域来看。 
							如bDeviceClass 域为零，此域也须为零 
							如bDeviceClass 域为FFH，此域的所有值保留。*/
	__u8  bDeviceProtocol; /*协议码?
							这些码的值视bDeviceClass 和 bDeviceSubClass 的值而定。?
							如果设备支持设备类相关的协议，此码标志了设备类的值。
							如果此域的值为零，则此设备不支持设备类相关的协议，
							然而，可能它的接口支持设备类相关的协议。
							如果此域的值为FFH，此设备使用厂商定义的协议。
							*/
	__u8  bMaxPacketSize0;  // 端点0的最大包大小（仅8,16,32,64为合法值）
	__le16 idVendor;  // 厂商标志（由USB-IF组织赋值）
	__le16 idProduct; // 产品标志（由厂商赋值）
	__le16 bcdDevice; // 设备发行号（BCD 码）
	__u8  iManufacturer; // 描述厂商信息的字符串描述符的索引值。
	__u8  iProduct; // 描述产品信息的字串描述符的索引值。
	__u8  iSerialNumber;  // 描述设备序列号信息的字串描述符的索引值。
	__u8  bNumConfigurations;  // 可能的配置描述符数目
} __attribute__ ((packed));

#define USB_DT_DEVICE_SIZE		18


/*
 * Device and/or Interface Class codes
 * as found in bDeviceClass or bInterfaceClass
 * and defined by www.usb.org documents
 */
#define USB_CLASS_PER_INTERFACE		0	/* for DeviceClass */
#define USB_CLASS_AUDIO			1
#define USB_CLASS_COMM			2
#define USB_CLASS_HID			3
#define USB_CLASS_PHYSICAL		5
#define USB_CLASS_STILL_IMAGE		6
#define USB_CLASS_PRINTER		7
#define USB_CLASS_MASS_STORAGE		8
#define USB_CLASS_HUB			9
#define USB_CLASS_CDC_DATA		0x0a
#define USB_CLASS_CSCID			0x0b	/* chip+ smart card */
#define USB_CLASS_CONTENT_SEC		0x0d	/* content security */
#define USB_CLASS_VIDEO			0x0e
#define USB_CLASS_WIRELESS_CONTROLLER	0xe0
#define USB_CLASS_MISC			0xef
#define USB_CLASS_APP_SPEC		0xfe
#define USB_CLASS_VENDOR_SPEC		0xff

#define USB_SUBCLASS_VENDOR_SPEC	0xff

/*-------------------------------------------------------------------------*/

/* USB_DT_CONFIG: Configuration descriptor information.
 *
 * USB_DT_OTHER_SPEED_CONFIG is the same descriptor, except that the
 * descriptor type is different.  Highspeed-capable devices can look
 * different depending on what speed they're currently running.  Only
 * devices with a USB_DT_DEVICE_QUALIFIER have any OTHER_SPEED_CONFIG
 * descriptors.
 */
// usb设备的配置描述符
struct usb_config_descriptor {
	__u8  bLength;  // 此描述表的字节数长度
	__u8  bDescriptorType;  // 配置描述表类型（此处为0x02）

	__le16 wTotalLength;  // 此配置信息的总长（包括配置，接口，端点和设备类及厂商定义的描述符）
	__u8  bNumInterfaces;  // 此配置所支持的接口个数
	__u8  bConfigurationValue; // 在SetConfiguration（）请求中用作参数来选定此配置
	__u8  iConfiguration; // 描述此配置的字串描述表索引
	__u8  bmAttributes; /*配置特性：?
						D7： 保留（设为一）?
						D6： 自给电源?
						D5： 远程唤醒?
						D4..0：保留（设为一）?
						一个既用总线电源又有自给电源的设备会在MaxPower域指出需要从总线取的电量。
						并设置D6为一。运行时期的实际电源模式可由GetStatus(DEVICE) 请求得到。*/
	__u8  bMaxPower;  // 在此配置下的总线电源耗费量。以 2mA 为一个单位
} __attribute__ ((packed));

#define USB_DT_CONFIG_SIZE		9

/* from config descriptor bmAttributes */
#define USB_CONFIG_ATT_ONE		(1 << 7)	/* must be set */
#define USB_CONFIG_ATT_SELFPOWER	(1 << 6)	/* self powered */
#define USB_CONFIG_ATT_WAKEUP		(1 << 5)	/* can wakeup */
#define USB_CONFIG_ATT_BATTERY		(1 << 4)	/* battery powered */

/*-------------------------------------------------------------------------*/

/* USB_DT_STRING: String descriptor */
// 字符串描述符
struct usb_string_descriptor {
	__u8  bLength; // 此描述表的字节数，这个长度-2就是字符串的长度，字符个数是字符串长度除以2
	__u8  bDescriptorType; // 字串描述表类型（此处应为0x03）

	__le16 wData[1];		/* UTF-16LE encoded */  // UTF-16 字符串，不带0结尾，
													// 每个字符占2个字节
} __attribute__ ((packed));

/* note that "string" zero is special, it holds language codes that
 * the device supports, not Unicode characters.
 */

/*-------------------------------------------------------------------------*/

/* USB_DT_INTERFACE: Interface descriptor */
// 
struct usb_interface_descriptor {
	__u8  bLength;  // 此表的字节数
	__u8  bDescriptorType;  // 接口描述表类（此处应为0x04）

	__u8  bInterfaceNumber; // 接口号，当前配置支持的接口数组索引（从零开始）。
	__u8  bAlternateSetting; // 可选设置的索引值
	__u8  bNumEndpoints; // 此接口用的端点数量，如果是零则说明此接口只用缺省控制管道。
	__u8  bInterfaceClass;  /*接口所属的类值： 
								零值为将来的标准保留。 
								如果此域的值设为FFH，则此接口类由厂商说明。 
								所有其它的值由USB 说明保留。*/
	__u8  bInterfaceSubClass; /*子类码 
								这些值的定义视bInterfaceClass域而定。 
								如果bInterfaceClass域的值为零则此域的值必须为零。 
								bInterfaceClass域不为FFH则所有值由USB 所保留。*/
	__u8  bInterfaceProtocol; /*协议码：bInterfaceClass 和bInterfaceSubClass 域的值而定.
							如果一个接口支持设备类相关的请求此域的值指出了设备类说明中所定义的协议.*/
	__u8  iInterface;   // 描述此接口的字串描述表的索引值。
} __attribute__ ((packed));

#define USB_DT_INTERFACE_SIZE		9

/*-------------------------------------------------------------------------*/

/* USB_DT_ENDPOINT: Endpoint descriptor */
struct usb_endpoint_descriptor {
	__u8  bLength;  // 此描述表的字节数长度
	__u8  bDescriptorType;  // 端点描述表类（此处应为0x05）

	__u8  bEndpointAddress;  /*此描述表所描述的端点的地址、方向： 
							Bit 3..0 : 端点号.
							Bit 6..4 : 保留,为零 
							Bit 7:    方向,如果控制端点则略。 
							0：输出端点（主机到设备） 
							1：输入端点（设备到主机）*/
	__u8  bmAttributes;  /*此域的值描述的是在bConfigurationValue域所指的配置下端点的特性。 
						Bit 1..0 :传送类型 
							00=控制传送 
							01=同步传送 
							10=批传送 
							11=中断传送 
						所有其它的位都保留。*/
	__le16 wMaxPacketSize; /*当前配置下此端点能够接收或发送的最大数据包的大小。 
							对于实进传输，此值用于为每帧的数据净负荷预留时间。
							在实际运行时，管道可能不完全需要预留的带宽，
							实际带宽可由设备通过一种非USB定义的机制汇报给主机。
							对于中断传输，批量传输和控制传输，端点可能发送比之短的数据包*/
	__u8  bInterval;  /*周期数据传输端点的时间间隙。 
						此域的值对于批传送的端点及控制传送的端点无意义。
						对于同步传送的端点此域必需为1，表示周期为1ms。
						对于中断传送的端点此域值的范围为1ms到255ms。*/

	/* NOTE:  these two are _only_ in audio endpoints. */
	/* use USB_DT_ENDPOINT*_SIZE in bLength, not sizeof. */
	__u8  bRefresh;
	__u8  bSynchAddress;
} __attribute__ ((packed));

#define USB_DT_ENDPOINT_SIZE		7
#define USB_DT_ENDPOINT_AUDIO_SIZE	9	/* Audio extension */


/*
 * Endpoints
 */
#define USB_ENDPOINT_NUMBER_MASK	0x0f	/* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK		0x80

#define USB_ENDPOINT_XFERTYPE_MASK	0x03	/* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL	0
#define USB_ENDPOINT_XFER_ISOC		1
#define USB_ENDPOINT_XFER_BULK		2
#define USB_ENDPOINT_XFER_INT		3
#define USB_ENDPOINT_MAX_ADJUSTABLE	0x80

#define USB_EP_MAXP_MULT_SHIFT	11
#define USB_EP_MAXP_MULT_MASK	(3 << USB_EP_MAXP_MULT_SHIFT)
#define USB_EP_MAXP_MULT(m) \
	(((m) & USB_EP_MAXP_MULT_MASK) >> USB_EP_MAXP_MULT_SHIFT)

/* The USB 3.0 spec redefines bits 5:4 of bmAttributes as interrupt ep type. */
#define USB_ENDPOINT_INTRTYPE		0x30
#define USB_ENDPOINT_INTR_PERIODIC	(0 << 4)
#define USB_ENDPOINT_INTR_NOTIFICATION	(1 << 4)

#define USB_ENDPOINT_SYNCTYPE		0x0c
#define USB_ENDPOINT_SYNC_NONE		(0 << 2)
#define USB_ENDPOINT_SYNC_ASYNC		(1 << 2)
#define USB_ENDPOINT_SYNC_ADAPTIVE	(2 << 2)
#define USB_ENDPOINT_SYNC_SYNC		(3 << 2)

#define USB_ENDPOINT_USAGE_MASK		0x30
#define USB_ENDPOINT_USAGE_DATA		0x00
#define USB_ENDPOINT_USAGE_FEEDBACK	0x10
#define USB_ENDPOINT_USAGE_IMPLICIT_FB	0x20	/* Implicit feedback Data endpoint */

/*-------------------------------------------------------------------------*/

/**
 * usb_endpoint_num - get the endpoint's number
 * @epd: endpoint to be checked
 *
 * Returns @epd's number: 0 to 15.
 */
static inline int usb_endpoint_num(const struct usb_endpoint_descriptor *epd)
{
	return epd->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
}

/**
 * usb_endpoint_type - get the endpoint's transfer type
 * @epd: endpoint to be checked
 *
 * Returns one of USB_ENDPOINT_XFER_{CONTROL, ISOC, BULK, INT} according
 * to @epd's transfer type.
 */
static inline int usb_endpoint_type(const struct usb_endpoint_descriptor *epd)
{
	return epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
}

/**
 * usb_endpoint_dir_in - check if the endpoint has IN direction
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type IN, otherwise it returns false.
 */
static inline int usb_endpoint_dir_in(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN);
}

/**
 * usb_endpoint_dir_out - check if the endpoint has OUT direction
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type OUT, otherwise it returns false.
 */
static inline int usb_endpoint_dir_out(
				const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT);
}

/**
 * usb_endpoint_xfer_bulk - check if the endpoint has bulk transfer type
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type bulk, otherwise it returns false.
 */
static inline int usb_endpoint_xfer_bulk(
				const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
		USB_ENDPOINT_XFER_BULK);
}

/**
 * usb_endpoint_xfer_control - check if the endpoint has control transfer type
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type control, otherwise it returns false.
 */
static inline int usb_endpoint_xfer_control(
				const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
		USB_ENDPOINT_XFER_CONTROL);
}

/**
 * usb_endpoint_xfer_int - check if the endpoint has interrupt transfer type
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type interrupt, otherwise it returns
 * false.
 */
static inline int usb_endpoint_xfer_int(
				const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
		USB_ENDPOINT_XFER_INT);
}

/**
 * usb_endpoint_xfer_isoc - check if the endpoint has isochronous transfer type
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint is of type isochronous, otherwise it returns
 * false.
 */
static inline int usb_endpoint_xfer_isoc(
				const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
		USB_ENDPOINT_XFER_ISOC);
}

/**
 * usb_endpoint_is_bulk_in - check if the endpoint is bulk IN
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint has bulk transfer type and IN direction,
 * otherwise it returns false.
 */
static inline int usb_endpoint_is_bulk_in(
				const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_bulk(epd) && usb_endpoint_dir_in(epd);
}

/**
 * usb_endpoint_is_bulk_out - check if the endpoint is bulk OUT
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint has bulk transfer type and OUT direction,
 * otherwise it returns false.
 */
static inline int usb_endpoint_is_bulk_out(
				const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_bulk(epd) && usb_endpoint_dir_out(epd);
}

/**
 * usb_endpoint_is_int_in - check if the endpoint is interrupt IN
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint has interrupt transfer type and IN direction,
 * otherwise it returns false.
 */
static inline int usb_endpoint_is_int_in(
				const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_int(epd) && usb_endpoint_dir_in(epd);
}

/**
 * usb_endpoint_is_int_out - check if the endpoint is interrupt OUT
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint has interrupt transfer type and OUT direction,
 * otherwise it returns false.
 */
static inline int usb_endpoint_is_int_out(
				const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_int(epd) && usb_endpoint_dir_out(epd);
}

/**
 * usb_endpoint_is_isoc_in - check if the endpoint is isochronous IN
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint has isochronous transfer type and IN direction,
 * otherwise it returns false.
 */
static inline int usb_endpoint_is_isoc_in(
				const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_isoc(epd) && usb_endpoint_dir_in(epd);
}

/**
 * usb_endpoint_is_isoc_out - check if the endpoint is isochronous OUT
 * @epd: endpoint to be checked
 *
 * Returns true if the endpoint has isochronous transfer type and OUT direction,
 * otherwise it returns false.
 */
static inline int usb_endpoint_is_isoc_out(
				const struct usb_endpoint_descriptor *epd)
{
	return usb_endpoint_xfer_isoc(epd) && usb_endpoint_dir_out(epd);
}

/**
 * usb_endpoint_maxp - get endpoint's max packet size
 * @epd: endpoint to be checked
 *
 * Returns @epd's max packet
 */
static inline int usb_endpoint_maxp(const struct usb_endpoint_descriptor *epd)
{
	return __le16_to_cpu(epd->wMaxPacketSize);
}

/**
 * usb_endpoint_maxp_mult - get endpoint's transactional opportunities
 * @epd: endpoint to be checked
 *
 * Return @epd's wMaxPacketSize[12:11] + 1
 */
static inline int
usb_endpoint_maxp_mult(const struct usb_endpoint_descriptor *epd)
{
	int maxp = __le16_to_cpu(epd->wMaxPacketSize);

	return USB_EP_MAXP_MULT(maxp) + 1;
}

static inline int usb_endpoint_interrupt_type(
		const struct usb_endpoint_descriptor *epd)
{
	return epd->bmAttributes & USB_ENDPOINT_INTRTYPE;
}

/*-------------------------------------------------------------------------*/

/* USB_DT_SSP_ISOC_ENDPOINT_COMP: SuperSpeedPlus Isochronous Endpoint Companion
 * descriptor
 */
struct usb_ssp_isoc_ep_comp_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__le16 wReseved;
	__le32 dwBytesPerInterval;
} __attribute__ ((packed));

#define USB_DT_SSP_ISOC_EP_COMP_SIZE		8

/*-------------------------------------------------------------------------*/

/* USB_DT_SS_ENDPOINT_COMP: SuperSpeed Endpoint Companion descriptor */
struct usb_ss_ep_comp_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__u8  bMaxBurst;
	__u8  bmAttributes;
	__le16 wBytesPerInterval;
} __attribute__ ((packed));

#define USB_DT_SS_EP_COMP_SIZE		6

/* Bits 4:0 of bmAttributes if this is a bulk endpoint */
static inline int
usb_ss_max_streams(const struct usb_ss_ep_comp_descriptor *comp)
{
	int		max_streams;

	if (!comp)
		return 0;

	max_streams = comp->bmAttributes & 0x1f;

	if (!max_streams)
		return 0;

	max_streams = 1 << max_streams;

	return max_streams;
}

/* Bits 1:0 of bmAttributes if this is an isoc endpoint */
#define USB_SS_MULT(p)			(1 + ((p) & 0x3))
/* Bit 7 of bmAttributes if a SSP isoc endpoint companion descriptor exists */
#define USB_SS_SSP_ISOC_COMP(p)		((p) & (1 << 7))

/*-------------------------------------------------------------------------*/

/* USB_DT_DEVICE_QUALIFIER: Device Qualifier descriptor */
struct usb_qualifier_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__le16 bcdUSB;
	__u8  bDeviceClass;
	__u8  bDeviceSubClass;
	__u8  bDeviceProtocol;
	__u8  bMaxPacketSize0;
	__u8  bNumConfigurations;
	__u8  bRESERVED;
} __attribute__ ((packed));


/*-------------------------------------------------------------------------*/

/* USB_DT_OTG (from OTG 1.0a supplement) */
struct usb_otg_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__u8  bmAttributes;	/* support for HNP, SRP, etc */
} __attribute__ ((packed));

/* USB_DT_OTG (from OTG 2.0 supplement) */
struct usb_otg20_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__u8  bmAttributes;	/* support for HNP, SRP and ADP, etc */
	__le16 bcdOTG;		/* OTG and EH supplement release number
				 * in binary-coded decimal(i.e. 2.0 is 0200H)
				 */
} __attribute__ ((packed));

/* from usb_otg_descriptor.bmAttributes */
#define USB_OTG_SRP		(1 << 0)
#define USB_OTG_HNP		(1 << 1)	/* swap host/device roles */
#define USB_OTG_ADP		(1 << 2)	/* support ADP */

#define OTG_STS_SELECTOR	0xF000		/* OTG status selector */
/*-------------------------------------------------------------------------*/

/* USB_DT_DEBUG:  for special highspeed devices, replacing serial console */
struct usb_debug_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	/* bulk endpoints with 8 byte maxpacket */
	__u8  bDebugInEndpoint;
	__u8  bDebugOutEndpoint;
} __attribute__((packed));

/*-------------------------------------------------------------------------*/

/* USB_DT_INTERFACE_ASSOCIATION: groups interfaces */
struct usb_interface_assoc_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__u8  bFirstInterface;
	__u8  bInterfaceCount;
	__u8  bFunctionClass;
	__u8  bFunctionSubClass;
	__u8  bFunctionProtocol;
	__u8  iFunction;
} __attribute__ ((packed));

#define USB_DT_INTERFACE_ASSOCIATION_SIZE	8

/*-------------------------------------------------------------------------*/

/* USB_DT_SECURITY:  group of wireless security descriptors, including
 * encryption types available for setting up a CC/association.
 */
struct usb_security_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__le16 wTotalLength;
	__u8  bNumEncryptionTypes;
} __attribute__((packed));

/*-------------------------------------------------------------------------*/

/* USB_DT_KEY:  used with {GET,SET}_SECURITY_DATA; only public keys
 * may be retrieved.
 */
struct usb_key_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__u8  tTKID[3];
	__u8  bReserved;
	__u8  bKeyData[0];
} __attribute__((packed));

/*-------------------------------------------------------------------------*/

/* USB_DT_ENCRYPTION_TYPE:  bundled in DT_SECURITY groups */
struct usb_encryption_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__u8  bEncryptionType;
#define	USB_ENC_TYPE_UNSECURE		0
#define	USB_ENC_TYPE_WIRED		1	/* non-wireless mode */
#define	USB_ENC_TYPE_CCM_1		2	/* aes128/cbc session */
#define	USB_ENC_TYPE_RSA_1		3	/* rsa3072/sha1 auth */
	__u8  bEncryptionValue;		/* use in SET_ENCRYPTION */
	__u8  bAuthKeyIndex;
} __attribute__((packed));


/*-------------------------------------------------------------------------*/

/* USB_DT_BOS:  group of device-level capabilities */
// 二进制设备对象描述符
struct usb_bos_descriptor {
	__u8  bLength;  // size of descriptor
	__u8  bDescriptorType; // BOS descriptor type

	__le16 wTotalLength;  // length of this descriptor and all of its sub descriptor
	__u8  bNumDeviceCaps; // the number of separate device capability descriptors in the BOS
} __attribute__((packed));

#define USB_DT_BOS_SIZE		5
/*-------------------------------------------------------------------------*/

/* USB_DT_DEVICE_CAPABILITY:  grouped with BOS */
// 设备能力描述符
struct usb_dev_cap_header {
	__u8  bLength; // size of descriptor
	__u8  bDescriptorType; // descriptor type : DEVICE CAPABILITY TYPE 
	__u8  bDevCapabilityType;  // 目前支持三种 无线USB (1)、USB2.0 (2) 扩展和USB3.0 (3)
								// USB_CAP_TYPE_WIRELESS_USB
								// USB_CAP_TYPE_EXT
								// USB_SS_CAP_TYPE
} __attribute__((packed));

#define	USB_CAP_TYPE_WIRELESS_USB	1   // usb_dev_cap_header.bDevCapabilityType

struct usb_wireless_cap_descriptor {	/* Ultra Wide Band */
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bDevCapabilityType;

	__u8  bmAttributes;
#define	USB_WIRELESS_P2P_DRD		(1 << 1)
#define	USB_WIRELESS_BEACON_MASK	(3 << 2)
#define	USB_WIRELESS_BEACON_SELF	(1 << 2)
#define	USB_WIRELESS_BEACON_DIRECTED	(2 << 2)
#define	USB_WIRELESS_BEACON_NONE	(3 << 2)
	__le16 wPHYRates;	/* bit rates, Mbps */
#define	USB_WIRELESS_PHY_53		(1 << 0)	/* always set */
#define	USB_WIRELESS_PHY_80		(1 << 1)
#define	USB_WIRELESS_PHY_107		(1 << 2)	/* always set */
#define	USB_WIRELESS_PHY_160		(1 << 3)
#define	USB_WIRELESS_PHY_200		(1 << 4)	/* always set */
#define	USB_WIRELESS_PHY_320		(1 << 5)
#define	USB_WIRELESS_PHY_400		(1 << 6)
#define	USB_WIRELESS_PHY_480		(1 << 7)
	__u8  bmTFITXPowerInfo;	/* TFI power levels */
	__u8  bmFFITXPowerInfo;	/* FFI power levels */
	__le16 bmBandGroup;
	__u8  bReserved;
} __attribute__((packed));

#define USB_DT_USB_WIRELESS_CAP_SIZE	11

/* USB 2.0 Extension descriptor */
#define	USB_CAP_TYPE_EXT		2  // usb_dev_cap_header.bDevCapabilityType

struct usb_ext_cap_descriptor {		/* Link Power Management */
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bDevCapabilityType;
	__le32 bmAttributes;
#define USB_LPM_SUPPORT			(1 << 1)	/* supports LPM */
#define USB_BESL_SUPPORT		(1 << 2)	/* supports BESL */
#define USB_BESL_BASELINE_VALID		(1 << 3)	/* Baseline BESL valid*/
#define USB_BESL_DEEP_VALID		(1 << 4)	/* Deep BESL valid */
#define USB_GET_BESL_BASELINE(p)	(((p) & (0xf << 8)) >> 8)
#define USB_GET_BESL_DEEP(p)		(((p) & (0xf << 12)) >> 12)
} __attribute__((packed));

#define USB_DT_USB_EXT_CAP_SIZE	7

/*
 * SuperSpeed USB Capability descriptor: Defines the set of SuperSpeed USB
 * specific device level capabilities
 */
#define		USB_SS_CAP_TYPE		3  // usb_dev_cap_header.bDevCapabilityType
struct usb_ss_cap_descriptor {		/* Link Power Management */
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bDevCapabilityType;
	__u8  bmAttributes;
#define USB_LTM_SUPPORT			(1 << 1) /* supports LTM */
	__le16 wSpeedSupported;
#define USB_LOW_SPEED_OPERATION		(1)	 /* Low speed operation */
#define USB_FULL_SPEED_OPERATION	(1 << 1) /* Full speed operation */
#define USB_HIGH_SPEED_OPERATION	(1 << 2) /* High speed operation */
#define USB_5GBPS_OPERATION		(1 << 3) /* Operation at 5Gbps */
	__u8  bFunctionalitySupport;
	__u8  bU1devExitLat;
	__le16 bU2DevExitLat;
} __attribute__((packed));

#define USB_DT_USB_SS_CAP_SIZE	10

/*
 * Container ID Capability descriptor: Defines the instance unique ID used to
 * identify the instance across all operating modes
 */
#define	CONTAINER_ID_TYPE	4
struct usb_ss_container_id_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bDevCapabilityType;
	__u8  bReserved;
	__u8  ContainerID[16]; /* 128-bit number */
} __attribute__((packed));

#define USB_DT_USB_SS_CONTN_ID_SIZE	20

/*
 * SuperSpeed Plus USB Capability descriptor: Defines the set of
 * SuperSpeed Plus USB specific device level capabilities
 */
#define	USB_SSP_CAP_TYPE	0xa
struct usb_ssp_cap_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bDevCapabilityType;
	__u8  bReserved;
	__le32 bmAttributes;
#define USB_SSP_SUBLINK_SPEED_ATTRIBS	(0x1f << 0) /* sublink speed entries */
#define USB_SSP_SUBLINK_SPEED_IDS	(0xf << 5)  /* speed ID entries */
	__le16  wFunctionalitySupport;
#define USB_SSP_MIN_SUBLINK_SPEED_ATTRIBUTE_ID	(0xf)
#define USB_SSP_MIN_RX_LANE_COUNT		(0xf << 8)
#define USB_SSP_MIN_TX_LANE_COUNT		(0xf << 12)
	__le16 wReserved;
	__le32 bmSublinkSpeedAttr[1]; /* list of sublink speed attrib entries */
#define USB_SSP_SUBLINK_SPEED_SSID	(0xf)		/* sublink speed ID */
#define USB_SSP_SUBLINK_SPEED_LSE	(0x3 << 4)	/* Lanespeed exponent */
#define USB_SSP_SUBLINK_SPEED_ST	(0x3 << 6)	/* Sublink type */
#define USB_SSP_SUBLINK_SPEED_RSVD	(0x3f << 8)	/* Reserved */
#define USB_SSP_SUBLINK_SPEED_LP	(0x3 << 14)	/* Link protocol */
#define USB_SSP_SUBLINK_SPEED_LSM	(0xff << 16)	/* Lanespeed mantissa */
} __attribute__((packed));

/*
 * USB Power Delivery Capability Descriptor:
 * Defines capabilities for PD
 */
/* Defines the various PD Capabilities of this device */
#define USB_PD_POWER_DELIVERY_CAPABILITY	0x06
/* Provides information on each battery supported by the device */
#define USB_PD_BATTERY_INFO_CAPABILITY		0x07
/* The Consumer characteristics of a Port on the device */
#define USB_PD_PD_CONSUMER_PORT_CAPABILITY	0x08
/* The provider characteristics of a Port on the device */
#define USB_PD_PD_PROVIDER_PORT_CAPABILITY	0x09

struct usb_pd_cap_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bDevCapabilityType; /* set to USB_PD_POWER_DELIVERY_CAPABILITY */
	__u8  bReserved;
	__le32 bmAttributes;
#define USB_PD_CAP_BATTERY_CHARGING	(1 << 1) /* supports Battery Charging specification */
#define USB_PD_CAP_USB_PD		(1 << 2) /* supports USB Power Delivery specification */
#define USB_PD_CAP_PROVIDER		(1 << 3) /* can provide power */
#define USB_PD_CAP_CONSUMER		(1 << 4) /* can consume power */
#define USB_PD_CAP_CHARGING_POLICY	(1 << 5) /* supports CHARGING_POLICY feature */
#define USB_PD_CAP_TYPE_C_CURRENT	(1 << 6) /* supports power capabilities defined in the USB Type-C Specification */

#define USB_PD_CAP_PWR_AC		(1 << 8)
#define USB_PD_CAP_PWR_BAT		(1 << 9)
#define USB_PD_CAP_PWR_USE_V_BUS	(1 << 14)

	__le16 bmProviderPorts; /* Bit zero refers to the UFP of the device */
	__le16 bmConsumerPorts;
	__le16 bcdBCVersion;
	__le16 bcdPDVersion;
	__le16 bcdUSBTypeCVersion;
} __attribute__((packed));

struct usb_pd_cap_battery_info_descriptor {
	__u8 bLength;
	__u8 bDescriptorType;
	__u8 bDevCapabilityType;
	/* Index of string descriptor shall contain the user friendly name for this battery */
	__u8 iBattery;
	/* Index of string descriptor shall contain the Serial Number String for this battery */
	__u8 iSerial;
	__u8 iManufacturer;
	__u8 bBatteryId; /* uniquely identifies this battery in status Messages */
	__u8 bReserved;
	/*
	 * Shall contain the Battery Charge value above which this
	 * battery is considered to be fully charged but not necessarily
	 * “topped off.”
	 */
	__le32 dwChargedThreshold; /* in mWh */
	/*
	 * Shall contain the minimum charge level of this battery such
	 * that above this threshold, a device can be assured of being
	 * able to power up successfully (see Battery Charging 1.2).
	 */
	__le32 dwWeakThreshold; /* in mWh */
	__le32 dwBatteryDesignCapacity; /* in mWh */
	__le32 dwBatteryLastFullchargeCapacity; /* in mWh */
} __attribute__((packed));

struct usb_pd_cap_consumer_port_descriptor {
	__u8 bLength;
	__u8 bDescriptorType;
	__u8 bDevCapabilityType;
	__u8 bReserved;
	__u8 bmCapabilities;
/* port will oerate under: */
#define USB_PD_CAP_CONSUMER_BC		(1 << 0) /* BC */
#define USB_PD_CAP_CONSUMER_PD		(1 << 1) /* PD */
#define USB_PD_CAP_CONSUMER_TYPE_C	(1 << 2) /* USB Type-C Current */
	__le16 wMinVoltage; /* in 50mV units */
	__le16 wMaxVoltage; /* in 50mV units */
	__u16 wReserved;
	__le32 dwMaxOperatingPower; /* in 10 mW - operating at steady state */
	__le32 dwMaxPeakPower; /* in 10mW units - operating at peak power */
	__le32 dwMaxPeakPowerTime; /* in 100ms units - duration of peak */
#define USB_PD_CAP_CONSUMER_UNKNOWN_PEAK_POWER_TIME 0xffff
} __attribute__((packed));

struct usb_pd_cap_provider_port_descriptor {
	__u8 bLength;
	__u8 bDescriptorType;
	__u8 bDevCapabilityType;
	__u8 bReserved1;
	__u8 bmCapabilities;
/* port will oerate under: */
#define USB_PD_CAP_PROVIDER_BC		(1 << 0) /* BC */
#define USB_PD_CAP_PROVIDER_PD		(1 << 1) /* PD */
#define USB_PD_CAP_PROVIDER_TYPE_C	(1 << 2) /* USB Type-C Current */
	__u8 bNumOfPDObjects;
	__u8 bReserved2;
	__le32 wPowerDataObject[];
} __attribute__((packed));

/*
 * Precision time measurement capability descriptor: advertised by devices and
 * hubs that support PTM
 */
#define	USB_PTM_CAP_TYPE	0xb
struct usb_ptm_cap_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bDevCapabilityType;
} __attribute__((packed));

#define USB_DT_USB_PTM_ID_SIZE		3
/*
 * The size of the descriptor for the Sublink Speed Attribute Count
 * (SSAC) specified in bmAttributes[4:0].
 */
#define USB_DT_USB_SSP_CAP_SIZE(ssac)	(16 + ssac * 4)

/*-------------------------------------------------------------------------*/

/* USB_DT_WIRELESS_ENDPOINT_COMP:  companion descriptor associated with
 * each endpoint descriptor for a wireless device
 */
struct usb_wireless_ep_comp_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__u8  bMaxBurst;
	__u8  bMaxSequence;
	__le16 wMaxStreamDelay;
	__le16 wOverTheAirPacketSize;
	__u8  bOverTheAirInterval;
	__u8  bmCompAttributes;
#define USB_ENDPOINT_SWITCH_MASK	0x03	/* in bmCompAttributes */
#define USB_ENDPOINT_SWITCH_NO		0
#define USB_ENDPOINT_SWITCH_SWITCH	1
#define USB_ENDPOINT_SWITCH_SCALE	2
} __attribute__((packed));

/*-------------------------------------------------------------------------*/

/* USB_REQ_SET_HANDSHAKE is a four-way handshake used between a wireless
 * host and a device for connection set up, mutual authentication, and
 * exchanging short lived session keys.  The handshake depends on a CC.
 */
struct usb_handshake {
	__u8 bMessageNumber;
	__u8 bStatus;
	__u8 tTKID[3];
	__u8 bReserved;
	__u8 CDID[16];
	__u8 nonce[16];
	__u8 MIC[8];
} __attribute__((packed));

/*-------------------------------------------------------------------------*/

/* USB_REQ_SET_CONNECTION modifies or revokes a connection context (CC).
 * A CC may also be set up using non-wireless secure channels (including
 * wired USB!), and some devices may support CCs with multiple hosts.
 */
struct usb_connection_context {
	__u8 CHID[16];		/* persistent host id */
	__u8 CDID[16];		/* device id (unique w/in host context) */
	__u8 CK[16];		/* connection key */
} __attribute__((packed));

/*-------------------------------------------------------------------------*/

/* USB 2.0 defines three speeds, here's how Linux identifies them */

enum usb_device_speed {
	USB_SPEED_UNKNOWN = 0,			/* enumerating */
	USB_SPEED_LOW, USB_SPEED_FULL,		/* usb 1.1 */
	USB_SPEED_HIGH,				/* usb 2.0 */
	USB_SPEED_WIRELESS,			/* wireless (usb 2.5) */
	USB_SPEED_SUPER,			/* usb 3.0 */
	USB_SPEED_SUPER_PLUS,			/* usb 3.1 */
};


enum usb_device_state {
	/* NOTATTACHED isn't in the USB spec, and this state acts
	 * the same as ATTACHED ... but it's clearer this way.
	 */
	USB_STATE_NOTATTACHED = 0,

	/* chapter 9 and authentication (wireless) device states */
	USB_STATE_ATTACHED,
	USB_STATE_POWERED,			/* wired */
	USB_STATE_RECONNECTING,			/* auth */
	USB_STATE_UNAUTHENTICATED,		/* auth */
	USB_STATE_DEFAULT,			/* limited function */
	USB_STATE_ADDRESS,
	USB_STATE_CONFIGURED,			/* most functions */

	USB_STATE_SUSPENDED

	/* NOTE:  there are actually four different SUSPENDED
	 * states, returning to POWERED, DEFAULT, ADDRESS, or
	 * CONFIGURED respectively when SOF tokens flow again.
	 * At this level there's no difference between L1 and L2
	 * suspend states.  (L2 being original USB 1.1 suspend.)
	 */
};

enum usb3_link_state {
	USB3_LPM_U0 = 0,
	USB3_LPM_U1,
	USB3_LPM_U2,
	USB3_LPM_U3
};

/*
 * A U1 timeout of 0x0 means the parent hub will reject any transitions to U1.
 * 0xff means the parent hub will accept transitions to U1, but will not
 * initiate a transition.
 *
 * A U1 timeout of 0x1 to 0x7F also causes the hub to initiate a transition to
 * U1 after that many microseconds.  Timeouts of 0x80 to 0xFE are reserved
 * values.
 *
 * A U2 timeout of 0x0 means the parent hub will reject any transitions to U2.
 * 0xff means the parent hub will accept transitions to U2, but will not
 * initiate a transition.
 *
 * A U2 timeout of 0x1 to 0xFE also causes the hub to initiate a transition to
 * U2 after N*256 microseconds.  Therefore a U2 timeout value of 0x1 means a U2
 * idle timer of 256 microseconds, 0x2 means 512 microseconds, 0xFE means
 * 65.024ms.
 */
#define USB3_LPM_DISABLED		0x0
#define USB3_LPM_U1_MAX_TIMEOUT		0x7F
#define USB3_LPM_U2_MAX_TIMEOUT		0xFE
#define USB3_LPM_DEVICE_INITIATED	0xFF

struct usb_set_sel_req {
	__u8	u1_sel;
	__u8	u1_pel;
	__le16	u2_sel;
	__le16	u2_pel;
} __attribute__ ((packed));

/*
 * The Set System Exit Latency control transfer provides one byte each for
 * U1 SEL and U1 PEL, so the max exit latency is 0xFF.  U2 SEL and U2 PEL each
 * are two bytes long.
 */
#define USB3_LPM_MAX_U1_SEL_PEL		0xFF
#define USB3_LPM_MAX_U2_SEL_PEL		0xFFFF

/*-------------------------------------------------------------------------*/

/*
 * As per USB compliance update, a device that is actively drawing
 * more than 100mA from USB must report itself as bus-powered in
 * the GetStatus(DEVICE) call.
 * http://compliance.usb.org/index.asp?UpdateFile=Electrical&Format=Standard#34
 */
#define USB_SELF_POWER_VBUS_MAX_DRAW		100
#define USB_HS_VBUS_MAX_DRAW			500

#endif /* _UAPI__LINUX_USB_CH9_H */
