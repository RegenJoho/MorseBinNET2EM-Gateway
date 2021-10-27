//Lease (the time how long an address is valid) in milliseconds
#define lease 300000
// bridge mode allows the MBNET2EM network to be accsessed over the network, thus allowing normal computers to send and receive messages to and from the network.
#define bridgeMode
#ifdef bridgeMode
  #define serverHostname "MorseBinNETGateWay"
  #define serverPort 34677
#endif
