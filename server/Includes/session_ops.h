#ifndef SESSION_OPS_H
#define SESSION_OPS_H

void handleLogout(client*c, char targetinout[PATHSIZE]);
void handleLogin(client*c,char* fieldmess,char targetinout[PATHSIZE]);
int clientIsLoggedIn(client* c);
int kickClient(char ip_addr_str[FIELDSIZE]);

#endif
