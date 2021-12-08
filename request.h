#ifndef __REQUEST_H__

//these functions can be used by other files
void request_handle(int fd, char* buf, char* method, char* uri, char* version, char* filename, char* cgiargs, int is_static, int not_in_directory);
int request_parse_uri(char *uri, char *filename, char *cgiargs);
void request_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

#endif // __REQUEST_H__
