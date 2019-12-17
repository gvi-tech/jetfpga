After you've installed the driver in your system it will be loaded at boot time. If there is a RIFFA 2.0.1 capable FPGA installed as well, 
the driver will detect it. Output in the system log will provide additional 
information. This makefile will install the C/C++ native library.


Install the driver and library:

sudo make install

The system will be configured to load the driver at boot time. The C/C++ library
will be installed in the default library path. The header files will be placed
in the default include path. You will want to reboot after you've installed for
the driver to be (re)loaded.
