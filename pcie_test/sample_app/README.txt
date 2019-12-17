The testutil example application works with the Verilog chnl_tester module
which receives data and then sends data back to the workstation. 

To compile the example application:

make

The riffa C/C++ library must be installed (it gets installed with the driver).


#Print FGPA list information
./testutil 0

#Performance test
./testutil <fpga_id> <channel_id> <number of words to transfer>, e.g. :
./testutil 2 0 0 10000000