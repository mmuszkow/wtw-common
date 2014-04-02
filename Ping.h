/** Ping.
  * @author Maciej Muszkowski
  */
#pragma once

#include <iphlpapi.h>
#include <icmpapi.h>
#pragma comment(lib, "iphlpapi.lib")

#define PING_TIMEOUT 3000

/** Pings host.
  *
  * @param host host name
  * @return true on ping succeded
  */
bool ping(const char* host) {	
	static const char PING_DATA[] = "ping";
	
	// Get host address info
	struct hostent* hp = gethostbyname(host);
	if(hp == NULL)
		return false;

	// Only IPv4
	if(hp->h_addrtype != AF_INET)
		return false;

	// Get host IP address
	ULONG ipaddr = (*(struct in_addr *)*hp->h_addr_list).S_un.S_addr;
	if (ipaddr == INADDR_NONE || ipaddr == 0)
		return false;
    
	// Create ICMP request handle
	HANDLE hIcmpFile = IcmpCreateFile();
	if (hIcmpFile == INVALID_HANDLE_VALUE)
		return false;

	// Send and recv
	BYTE replyBuffer[sizeof(ICMP_ECHO_REPLY)+sizeof(PING_DATA)];
	DWORD iseRes = IcmpSendEcho(hIcmpFile, ipaddr, 
		(LPVOID) PING_DATA, sizeof(PING_DATA), NULL, 
		(LPVOID) replyBuffer, sizeof(replyBuffer), PING_TIMEOUT);		
	
	// Check if any reply and reply status == success
	if(iseRes > 0) {
		PICMP_ECHO_REPLY reply = (PICMP_ECHO_REPLY) replyBuffer;
		if(reply->Status != IP_SUCCESS)
			iseRes = 0;
	}

	IcmpCloseHandle(hIcmpFile);

	return (iseRes != 0) ? true : false;
}
