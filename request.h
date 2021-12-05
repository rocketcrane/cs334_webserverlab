#ifndef __REQUEST_H__

void request_handle(int fd, char* buf, char* method, char* uri, char* version, char* filename, char* cgiargs, int is_static);
int request_parse_uri(char *uri, char *filename, char *cgiargs);

#endif // __REQUEST_H__
