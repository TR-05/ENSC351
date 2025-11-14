// udp.h
// Module to read from UDP
#ifndef _UDP_H_
#define _UDP_H_

// Begin/end the background thread
void UDP_init(void);
void UDP_cleanup(void);
void UDP_join_thread(void);

#endif