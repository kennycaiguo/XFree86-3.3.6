/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_savage/s3sav_misc.c,v 1.1.2.1 1999/07/30 11:21:35 hohndel Exp $ */

/*
 *
 * Copyright 1995-1997 The XFree86 Project, Inc.
 *
 */

/* 
 * Various functions used in the virge driver. 
 * Right now, this only contains the PCI probing function.
 * 
 * Created 18/03/97 by Sebastien Marineau
 * Revision: 
 * [0.2] 08/02/98: Rewrite to use the VGA PCI information instead of re-probing
 *       the PCI bus.
 *
 * [0.1] 18/03/97: Added PCI probe function, taken from accel/s3_virge server.
 *       Not sure if the code used to adjust the PCI base address is 
 *       still needed for the ViRGE chipsets.
 */

#include "X.h"
#include "input.h"
#include "screenint.h"

#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "xf86_PCI.h"
#include "vga.h"
#include "vgaPCI.h"

#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

#include "regs3sav.h"
#include "s3sav_driver.h"

/** Why isn't all of this included in s3sav_driver.c? **/

extern vgaPCIInformation *vgaPCIInfo;
extern SymTabRec s3savChipTable[];
extern S3VPRIV s3vPriv;

static S3PCIInformation Mappings[] = {
    { PCI_SAVAGE3D,	S3_SAVAGE3D },
    { PCI_SAVAGE3D_MV,	S3_SAVAGE3D_MV },
    { PCI_SAVAGE4,	S3_SAVAGE4 },
    { PCI_SAVAGE2000,	S3_SAVAGE2000 },
    { PCI_SAVAGE_MX_MV,	S3_SAVAGE_MX },
    { PCI_SAVAGE_MX,	S3_SAVAGE_MX },
    { PCI_SAVAGE_IX_MV,	S3_SAVAGE_MX },
    { PCI_SAVAGE_IX,	S3_SAVAGE_MX },
    { PCI_SAVAGE_370,	S3_SAVAGE_370 },
};

#define NMAPPINGS	(sizeof(Mappings)/sizeof(Mappings[0]))


/*
 * s3vGetPCIInfo -- probe for PCI information
 */

S3PCIInformation *
S3SAVGetPCIInfo()
{
   static S3PCIInformation info = {0, };
   pciConfigPtr pcrp = NULL;
   Bool found = FALSE;
   int i = 0;


   if (vgaPCIInfo && vgaPCIInfo->AllCards) {
      while (pcrp = vgaPCIInfo->AllCards[i]) {
         if((pcrp->_vendor == PCI_S3_VENDOR_ID) && 
	    (pcrp->_base_class == PCI_CLASS_DISPLAY) &&
	    (pcrp->_sub_class == PCI_SUBCLASS_DISPLAY_VGA) &&
	    (pcrp->_command != 0)
	 ) {
	    int ChipId = pcrp->_device;
	    int j;
	    if (vga256InfoRec.chipID) {
	      ErrorF("%s %s: S3 chipset override, using chip_id = 0x%04x instead of 0x%04x\n",
		  XCONFIG_GIVEN, vga256InfoRec.name, vga256InfoRec.chipID, ChipId);
	      ChipId = vga256InfoRec.chipID;
	    }
	    found = TRUE;
	
	    for( j = 0; j < NMAPPINGS; j++ )
	    {
		if( ChipId == Mappings[j].DevID ) {
		    info.ChipType = Mappings[j].ChipType;
		    break;
		}
	    }

	    if( j >= NMAPPINGS )
	    {
		info.ChipType = S3_UNKNOWN;
		info.DevID = pcrp->_device;
	    }

	    info.ChipRev = pcrp->_rev_id;

	    info.MemBase = pcrp->_base0 & 0xFFFFFFF0;

	    if(
		(ChipId == PCI_SAVAGE4) ||
		(ChipId == PCI_SAVAGE2000) ||
		(ChipId == PCI_SAVAGE_370)
	    ) {
		info.MemBase1 = pcrp->_base1 & 0xFFFFFFF0;
	    }
	    break;
         }
      i++;
      }
   }
   else 
      return (FALSE);

   if (found && xf86Verbose) {
      if (info.ChipType != S3_UNKNOWN) {
	 ErrorF("%s %s: SAVAGE: %s rev %x, Linear FB @ 0x%08lx\n", XCONFIG_PROBED,
		vga256InfoRec.name,xf86TokenToString(s3savChipTable, info.ChipType), 
		info.ChipRev, info.MemBase);
      }
   }

   if (found)
      return &info;
   else
      return NULL;
}
