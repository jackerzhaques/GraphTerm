# GraphTerm
A terminal program that allows graphing and exporting data from serial. This software was designed for tuning control loops, but can be used in any situation.

The program connects over a native com port to the device. All data sent to and received from the device is shown in the terminal.

The device can send the following commands to the terminal for graphing

Create a data series

Op ID Name \n
Where Op is the opcode, Id is the 32 bit ID, and Name is a series of bytes (padded to be 32 bits) that gives the series a name.

Add a data point

Op ID Data1, Data2, ...DataX \n
Where Op is the opcode, ID is the 32 bit ID of the series the data belongs to, and DataX is the floating point data (4 consecutive bytes).
