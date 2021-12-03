#ifndef __REQUEST_H__

void request_handle(int fd, char* buf, char* method, char* uri, char* version, char* filename);

#endif // __REQUEST_H__
