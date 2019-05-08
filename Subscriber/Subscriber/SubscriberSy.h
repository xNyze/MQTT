/*SuscriberSy.h - Win32 Konsole Version */

extern int initClient();

extern void getRequest(struct genericMsg *req);

extern int joinMCAddress(char *mcGroupAddr);

extern void sendRequest(int topic, struct subscribeMsg *req);

void sendUDPMsg(struct genericMsg* annMsg, char *mcGroupAddr, char *port);

extern void closeClient();