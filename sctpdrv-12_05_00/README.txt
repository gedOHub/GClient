--------------------------------------------------------------------------------
Copyright (C) 2008 by CO-CONV, Corp., Kyoto, Japan.
Copyright (C) 2010-2013 Bruce Cran.

Developed by co-research between CO-CONV, Corp. and WIDE Project.
--------------------------------------------------------------------------------

Introduction:
The source code in this directory is for SctpDrv, an
SCTP driver for Windows. All the code is distributed under
the modified BSD license.

Explanation:
	apps/		Sample apps
	drv/		Driver's entry point etc.
	kern/		Kernel interface for Winsock, etc.
	net/		Network interface, routing, etc.
	netinet/	SCTP (derived from FreeBSD SCTP code)
	netinet6/	IPv6 specific 
	sp/		Userland interface for Winsock
	subr/		Subroutines
	sys/		Kernel specific header files
	wix/		Installer

Environment:
Windows Operating System on which you can execute WDK

How to get the latest code:
You can get the code from the Subversion repository at:
	https://code.bluestop.org/sctpdrv/

How to build:

Before you build SctpDrv, you have to download the WDK (Windows Driver Kit) from
Microsoft (http://msdn.microsoft.com/en-gb/library/windows/hardware/gg487428.aspx)
and install it.

The WiX (Windows Installer XML) toolset is also needed. Please install WiX 3.7
from http://wixtoolset.org

To build the installer package launch a Visual Studio command prompt, change to the
directory containing this README file, and run "msbuild /m". The installer MSI files
should be created in the "stage" directory.
--------------------------------------------------------------------------------


Happy SCTPing!
