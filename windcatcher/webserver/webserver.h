#ifndef _WEBSERVER
#define _WEBSERVER

#define  WEBSEVER_TASK_PRIORITY ( tskIDLE_PRIORITY + 2 )

/* The size of the buffer in which the dynamic WEB page is created. */
#define webMAX_PAGE_SIZE        512

/* Standard GET response. */
#define webHTTP_OK      "HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n"

/* The port on which we listen. */
#define webHTTP_PORT            ( 80 )

/* Delay on close error. */
#define webSHORT_DELAY          ( 10 )

/* Format of the dynamic page that is returned on each connection. */
#define webHTML_START \
"<html>\
<head>\
</head>\
<BODY bgcolor=\"#CCCCff\">\ "

#define webHTML_END \
"\r\n</pre>\
\r\n</BODY>\
</html>"
#define STR_ABOUT "� ����������� \r\n \
��������-���������� ��������������� ��������� ����������� ������������������������ ����� 40 ��� �������� �� �������� ������������� �������������, ������������ ���������� ������ �������� ����� �� ��������������� � ����������� ���������� ����, �������: \r\n \
����������\r\n \
������������ � �������� ������������ \r\n \
������� \r\n \
��������� \r\n  \
���������� � ��������� ���  \r\n \
�����������\r\n "


extern void webStartServer( void *);

#endif



