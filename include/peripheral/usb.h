typedef enum { DEVICE                = (uint8_t)0x01,
               CONFIGURATION         = (uint8_t)0x02,
               STRING                = (uint8_t)0x03,
               INTERFACE             = (uint8_t)0x04,
               ENDPOINT              = (uint8_t)0x05,
               DEVICE_QUALIFIER      = (uint8_t)0x06, // Only for High Speed USB
               OTHER_SPEED_CONFIG    = (uint8_t)0x07, // Only for High Speed USB
               OTG                   = (uint8_t)0x09
             }usb_descriptor_type_t;

typedef enum{ CLEAR_FEATURE = (uint8_t)0x01,
              GET_CONFIG    = (uint8_t)0x08,
              GET_DESCRIPTOR= (uint8_t)0x06,
              GET_INTERFACE = (uint8_t)0x0A,
              GET_STATUS    = (uint8_t)0x00,
              SET_ADDRESS   = (uint8_t)0x05,
              SET_CONFIG    = (uint8_t)0x09,
              SET_DESCRIPTOR= (uint8_t)0x07,
              SET_FEATURE   = (uint8_t)0x03,
              SET_INTERFACE = (uint8_t)0x0B,
              SYNCH_FRAME   = (uint8_t)0x0C
            }usb_request_t;

/* INTERFACE descriptor */
#define CONTROL_TRANSFER 0
#define ISOCHRONOUS_TRANSFER 0
#define BULK_TRANSFER 0
#define INTERRUPT_TRANSFER
typedef struct
{
  uint8_t  length;                        // size of the descriptor in bytes
  usb_descriptor_type_t descriptor_type;  // ENDPOINT descriptor type
  uint8_t  endpoint_address;              // bit7: direction, 0=OUT 1=IN
                                          // bit[6:4]: 0, 
                                          // bit[3:0]: EndpointNumber
  uint8_t  attributes;                    // bit[1:0]: TRANSFERT type + synchronisation attributes if ISOCHRONOUS transfer used
  uint16_t max_packet_size;               // max packet size in bytes
  uint8_t  interval;                      // interval for polling: 
                          // if LS or FS: expressed in frames(ms)
                          // if HS      : expressed in microframes (us)
} __attribute__((packed)) usb_ep_desc_t;

typedef struct
{
  uint8_t length;                         // size of the descriptor in bytes
  usb_descriptor_type_t descriptor_type;  // INTERFACE descriptor type
  uint8_t interface_number;               // interface number
  uint8_t alternate_setting;              // ??
  uint8_t num_endpoints;                  // number of endpoints used by this interface
  uint8_t interface_class;                // 0xFF if not class given by USB-IF
  uint8_t interface_subclass;             // 0xFF if not class given by USB-IF
  uint8_t interface_protocol;             // 0xFF if not class given by USB-IF 
  uint8_t interface_string_index;         // 0 if no description STRING descriptor
  usb_ep_desc_t eps[2];                   //FIXME should be a pointer
} __attribute__((packed)) usb_iface_desc_t;


/* CONFIGURATION descriptor */
typedef struct 
{
  uint8_t length;                         // size of the descriptor in bytes
  usb_descriptor_type_t descriptor_type;  // CONFIGURATION or OTHER_SPEED_CONFIG descriptor type
  uint16_t total_length;                  // total length in bytes: \
                                             if descriptor are constant size:\
  total_length = ConfigDescriptor.length +  \
                 numInterface*InterfaceDescriptor.length + \
                 Interface.num_endpoint*endpoint.length(foreach Interface)
  uint8_t  num_interfaces;                // number of interfaces in this configuration
  uint8_t  config_value;           // Value used by SetConfiguration request to select this configuration
  uint8_t  config_string_index;    // 0 if no description STRING descriptor
  uint8_t  attributes;                    // 0x80, defaut
                                          // | bit6: self powered 
                                          // | bit5: remote wakeup
  uint8_t  max_power;                     // max current in 2mA steps
  usb_iface_desc_t ifaces[1];             //FIXME should be a pointer
} __attribute__((packed)) usb_config_desc_t;


/* DEVICE descriptor */
typedef enum {USB1_0=(uint16_t)0x0100,
              USB1_1=(uint16_t)0x0110,
              USB2_0=(uint16_t)0x0200
             }usb_version_number_t;

typedef struct
{
  uint8_t length;                         // size of the descriptor in bytes
  usb_descriptor_type_t descriptor_type;  // DEVICE descriptor type
  usb_version_number_t bcdUSB;            // USB Spec release / Verison Number
  uint8_t device_class;                   // 0xFF if no official class
  uint8_t device_sub_class;               // 0xFF if no official sub class
  uint8_t device_protocol;                // 0xFF if no official protocol
  uint8_t max_packet_size_0;              // max packet size for EP0 (control EndPoint)
  uint16_t idVendor;                      // Official Vendor ID (from USB-IF)
  uint16_t idProduct;                     // From manufacturer
  uint16_t bcdDevice;                     // Device Release Number
  uint8_t Manufacturer;                   // manufacturer string descriptor index (0 if no string)
  uint8_t Product;                        // product string descriptor index (0 if no string)
  uint8_t SerialNumber;                   // serial number string descriptor index (0 if no string)
  uint8_t NumConfiguration;               // number of possible configurations (usually one, Windows built-in never checks the others)

} __attribute__((packed)) usb_dev_desc_t;


/* STRING descriptor */
typedef struct
{
  uint8_t length;                         // size of the descriptor in bytes
  usb_descriptor_type_t descriptor_type;  // STRING descriptor type
  uint16_t string;                        // UNICODe encoded string
} __attribute__((packed)) usb_str_desc_t;

/* High Speed USB descriptors */
typedef struct
{
  uint8_t length;                         // size of the descriptor in bytes
  usb_descriptor_type_t descriptor_type;  // DEVICE_QUALIFIER descriptor type
  usb_version_number_t bcdUSB;            // USB Spec release / Verison Number
  uint8_t device_class;                   // 0xFF if no official class
  uint8_t device_sub_class;               // 0xFF if no official sub class
  uint8_t device_protocol;                // 0xFF if no official protocol
  uint8_t max_packet_size_0;              // max packet size for EP0 (control EndPoint)
  uint8_t NumConfiguration;               // number of possible configurations (usually one, Windows built-in never checks the others)
  uint8_t Reserved;                       //

} __attribute__((packed)) usb_dev_qualif_desc_t;

typedef struct
{
  uint8_t request_type;                   // request type
  usb_request_t request;                  // request 
  uint16_t value;                         // depends on request...
  uint16_t index;                         // depends on request...
  uint16_t length;                        // number of bytes to transfer if data stage involved
  
} __attribute__((packed)) usb_setup_request_t;


typedef struct
{
  uint8_t length;                         // size of the descriptor in bytes
  usb_descriptor_type_t descriptor_type;  // STRING descriptor type
  uint8_t  attributes;                    // bit0: SRP Support,bit1: HNP support
} __attribute__((packed)) usb_otg_desc_t;
