# Wireless-Sensor-Networks-EE407
Class project for EE407

What you will likely care about can be found under 
/ns-allinone-3.30.1/ns-3.30.1/src/dvhop/

If you already have ns3 installed:
download the dvhop.zip folder
unzip the folder
unzip dvhop.zip
move dvhop/ to ns-allinone-3.30.1/ns-3.30.1/

To configure & run:
under ns-allinone-3.30.1/ns-3.30.1/
    ./waf configure --enable-examples
    ./waf --run dvhop-example

to view animation, run netanim, and find animation.xml under ns-allinone-3.30.1/ns-3.30.1/src/
to view pcap files, use wireshark, and open any "aodv-node-x-x.pcap" file
to view hop counts, open dvhop.distances
to view estimated coordinates, scroll through command line output
