#ifndef CLASS_PCI_DEVICE_HPP
#define CLASS_PCI_DEVICE_HPP

#include <stdio.h>
//#include <syscalls.hpp>
#include <hw/pci.h>

#define PCI_WTF 0xffffffff


/** 
    @brief PCI device message format
    
    Used to communicate with PCI devices
*/
  union pci_msg{

    //! The whole message
    uint32_t data;
    
    /** Packed attribtues, ordered low to high. 
        @note Doxygen thinks this is a function - it's not 
        it's a GCC-directive.*/
    struct __attribute__((packed)){

      //! The PCI register
      uint8_t reg;
      
      //! The 16-bit PCI-address @see pci_addr()
      uint16_t addr;
      uint8_t code;
    };
  };

/** Relevant class codes (many more) */
enum classcode_t {CL_OLD,CL_STORAGE,CL_NIC,CL_DISPLAY,
                  CL_MULTIMEDIA,CL_MEMORY,CL_BRIDGE};

/**
   @brief Communication class for all PCI devices
   
   All low level communication with PCI devices should (ideally) go here.
   
   @todo 
   - Consider if we ever need to separate the address into 'bus/dev/func' parts.
   - Do we ever need anything but PCI Devices?
*/
class PCI_Device
  //:public Device //Why not? A PCI device is too general to be accessible?
{  
  
  //@brief The 3-part PCI address
  uint16_t pci_addr_;
  
  //@brief The three address parts derived (if needed)      
  uint8_t busno_ = 0;
  uint8_t devno_ = 0;
  uint8_t funcno_ = 0;
  
  // @brief The 2-part ID retrieved from the device
  union vendor_product{
    uint32_t __value;
    struct __attribute__((packed)){
      uint16_t vendor;
      uint16_t product;
    };
  }device_id_;

  // @brief The class code (device type)
  union class_revision{
    uint32_t reg;
    struct __attribute__((packed)){
      uint8_t rev_id;
      uint8_t prog_if;
      uint8_t subclass;
      uint8_t classcode;
    };
    struct __attribute__((packed)){
      uint16_t class_subclass;
      uint8_t __prog_if; //Overlaps the above
      uint8_t revision;        
    };
  }devtype_;

  
  // @brief Printable names
  const char *classname_;
  const char *vendorname_;
  const char *productname_;
  
  /*
    Device Resources
   */
  
  //! @brief Resource types, "Memory" or "I/O"
  enum resource_t{RES_MEM,RES_IO};
  
  /** A device resource - possibly a list */
  template<resource_t RT>
  struct Resource{
    const resource_t type = RT;
    uint32_t start_;
    uint32_t len_;
    Resource<RT>* next = 0;
    Resource<RT>(uint32_t start,uint32_t len):start_(start),len_(len){};
  };

  //! @brief Resource lists. Members added by add_resource();
  Resource<RES_MEM>* res_mem_ = 0;
  Resource<RES_IO>* res_io_ = 0;
   

  //! @brief Write to device with implicit pci_address (e.g. used by Nic)
  inline void write_dword(uint8_t reg,uint32_t value){
    pci_msg req;
    req.data=0x80000000;
    req.addr=pci_addr_;
    req.reg=reg;
    
    outpd(PCI_CONFIG_ADDR,(uint32_t)0x80000000 | req.data );
    outpd(PCI_CONFIG_DATA, value);
  };

  /**   Add a resource to a resource queue.
         
        (This seems pretty dirty; private class, reference to pointer etc.) */
  template<resource_t R_T>
  void add_resource(Resource<R_T>* res,Resource<R_T>*& Q){
    Resource<R_T>* q;
    if (Q) {
      q = Q;
      while (q->next) q=q->next;
      q->next = res;
    } else {
      Q = res;
    }
  };

public:
  /*
    Static functions
  */  
  enum{VENDOR_INTEL=0x8086,VENDOR_CIRRUS=0x1013,VENDOR_REALTEK=0x10EC,
       VENDOR_VIRTIO=0x1AF4,VENDOR_AMD=0x1022};
  
  
  //! @brief Read from device with implicit pci_address (e.g. used by Nic)
  inline uint32_t read_dword(uint8_t reg){
    pci_msg req;
    req.data=0x80000000;
    req.addr=pci_addr_;
    req.reg=reg;
    
    outpd(PCI_CONFIG_ADDR,(uint32_t)0x80000000 | req.data );
    return inpd(PCI_CONFIG_DATA);
  };


  //! @brief Read from device with explicit pci_addr
  static inline uint32_t read_dword(uint16_t pci_addr, uint8_t reg){
    pci_msg req;
    req.data=0x80000000;
    req.addr=pci_addr;
    req.reg=reg;
    
    outpd(PCI_CONFIG_ADDR,(uint32_t)0x80000000 | req.data );
    return inpd(PCI_CONFIG_DATA);
  };  


  /** Probe for a device on the given address
      
      @deprecated We got a 20% performance degradation using this for probing
      @see PCI_Device() 
  */
  static PCI_Device* Create(uint16_t pci_addr);  

  // @brief Get a device by address. @see pci_addr().
  static PCI_Device* get(uint16_t pci_addr);

  // @brief Get a device by individual address parts. @todo Will we ever need this?  
  static PCI_Device* get(int busno, int devno,int funcno);
  
  
  /** Constructor
      
      @param pci_addr: A 16-bit PCI address. 
      @param id: A device ID, consisting of PCI vendor- and product- ID's. 
      @see pci_addr() for more about the address  
  */
  PCI_Device(uint16_t pci_addr,uint32_t id);

  
  /** A descriptive name  */
  inline const char* name();
  
  
  /** Get the PCI address of device.
     
     The address is a composite of 'bus', 'device' and 'function', usually used
     (i.e. by Linux) to designate a PCI device.  */
  inline uint16_t pci_addr() { return pci_addr_; };
    

  /** Get the pci class code. */
  inline classcode_t classcode() 
  { return static_cast<classcode_t>(devtype_.classcode); }

  /** Get the pci vendor id */
  inline uint16_t vendor_id() { return device_id_.vendor; }

  /** Parse all Base Address Registers (BAR's)
      
      Used to determine how to communicate with the device. 
      This function adds Resources to the PCI_Device.
   */
  void probe_resources();
  
  /** The base address of the (first) I/O resource */
  uint32_t iobase();
  
  
};



#endif
