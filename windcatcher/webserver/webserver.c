#include <stdlib.h>
#include <string.h>

#include "typedef.h"
#include "webserver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "dbg.h"
#include "ppp.h"
#include "gprscon.h"




#define FILESNUM 5
typedef enum
  {
  FILE_INDEX, FILE_404, FILE_IMAGE1, FILE_501,FILE_ABOUT
  } FILE_ID;

typedef struct
  {
    FILE_ID id;
    const char *pagename;
  } WEBFILE;

WEBFILE fs[FILESNUM] =
  { FILE_INDEX, "/index.html", FILE_404, "/404.html", FILE_IMAGE1, "/img1",
      FILE_501, "/501.html",FILE_ABOUT,"/about.html"};

void webThread(void *)  __attribute__((naked));
/*------------------------------------------------------------*/

/*
 * Process an incoming connection on port 80.
 *
 * This simply checks to see if the incoming data contains a GET request, and
 * if so sends back a single dynamically created page.  The connection is then
 * closed.  A more complete implementation could create a task for each
 * connection.
 */
static void vProcessConnection(struct netconn *pxNetCon);
FILE_ID vParseRequest(char *pxReqest, int usLength);
void vSendPage(FILE_ID id,struct netconn *pxNetCon);

void webStartServer(void *pxThrCtlCur)
  {
      xTaskCreate  ( webThread, ( const signed portCHAR * const ) "webServer",
      200,
      pxThrCtlCur,
      WEBSEVER_TASK_PRIORITY,
      ( xTaskHandle * ) NULL);
}
void webStopServer()
  {

  }
void webThread(void *pvParameters)
  {
    ePPPThreadControl *pxePPPThrCtlCur = pvParameters;
    struct netconn *pxHTTPListener, *pxNewConnection;
    dbgmessage("Web server thread started \r\n");
    pxHTTPListener = netconn_new(NETCONN_TCP);
    netconn_bind(pxHTTPListener, NULL, webHTTP_PORT);
    netconn_listen(pxHTTPListener);
    dbgmessage("Web server init... complite \r\n");
    while (1)
      {
        if (((*pxePPPThrCtlCur) != CONNECTED))
          {
            dbgmessage("Web Server waiting ppp connection !!! ... \r\n");
          }
        while (*pxePPPThrCtlCur != CONNECTED)
          {
            vTaskDelay(10);
          }
        dbgmessage("Web Server ppp connected !!! ... \r\n");
        dbgStackFreeSize("Web server Task Stack");
        pxNewConnection = netconn_accept(pxHTTPListener);
        if (pxNewConnection != NULL)
          {
            dbgmessage("WEB NETCON Create ... \r\n");
            //           dbgmessage(">NETCON done !!! ... \r\n");
            /* Service connection. */
            vProcessConnection(pxNewConnection);
            while (netconn_delete(pxNewConnection) != ERR_OK)
              {

                vTaskDelay(webSHORT_DELAY);
              }
            dbgmessage("WEB NETCON Delete ... \r\n");
          }
        vTaskDelay(5);
      }
  }

static void vProcessConnection(struct netconn *pxNetCon)
  {
    struct netbuf *pxRxBuffer;
    portCHAR *pcRxString;
    unsigned portSHORT usLength;

    /* We expect to immediately get data. */
    pxRxBuffer = netconn_recv(pxNetCon);
    dbgmessage(">NETCON receive ... \r\n");
    if (pxRxBuffer != NULL)
      {
        /* Where is the data? */
        netbuf_data(pxRxBuffer, (void *) &pcRxString, &usLength);
        trace(">Get Data  %d bytes  \r\n", (int) usLength);
        /* Is this a GET?  We don't handle anything else. */
        // Parse request AndSendPage
        vSendPage(vParseRequest(pcRxString, (int) usLength),pxNetCon);

        if (!strncmp(pcRxString, "GET", 3))
          {

          }

        netbuf_delete(pxRxBuffer);
      }

    netconn_close(pxNetCon);
  }
/*---------------------------------------
 * ---------------------*/
FILE_ID vParseRequest(char *pxReqest, int usLength)
  {
    int i;
    err_t request_supported;

    /* default is request not supported */
    request_supported = ERR_ARG;

    /* @todo: support POST, check p->len */
    if (strncmp(pxReqest, "GET ", 4) == 0)
      {
        request_supported = ERR_OK;
        for (i = 0; i < 40; i++)
          {
            if (((char *) pxReqest + 4)[i] == ' ' || ((char *) pxReqest + 4)[i]
                == '\r' || ((char *) pxReqest + 4)[i] == '\n')
              {
                ((char *) pxReqest + 4)[i] = 0;
              }
          }
      }

    if (request_supported == ERR_OK)
      {
        if (*(char *) (pxReqest + 4) == '/' && *(char *) (pxReqest + 5) == 0)
          {
            /* root -> index.html */
            /* @todo: trailing / -> /../index.html */
           return FILE_INDEX;
          }
        else
          {
//            /* @todo: filter out hostname (valid request!) */
//            //filename = (const char *) pxReqest + 4;
//           for(i=0;i<FILESNUM;i++)
//             {
//               if(strcmp(fs[i].pagename,(char *) (pxReqest + 4) ))
//                     {
//                       return fs[i].id;
//                     }
//             }
//
//            return FILE_404;
          }
      }
    else
      {
        /* invalid request/not implemented */
        return FILE_501;
      }
    return FILE_404;
  }
void vSendPage(FILE_ID id,struct netconn *pxNetCon)
  {
    static portCHAR cDynamicPage[webMAX_PAGE_SIZE], cPageHits[11];
      switch (id)
      {
    case FILE_INDEX:
      /* Write out the HTTP OK header. */
      netconn_write(pxNetCon, webHTTP_OK, (u16_t) strlen(webHTTP_OK),
          NETCONN_COPY);

      /* Generate the dynamic page...
       ... First the page header. */
      strcpy(cDynamicPage, webHTML_START);
      /* ... Then the hit count... */
      strcat(cDynamicPage, cPageHits);
      strcat(cDynamicPage,
          "<br>************************************************<br>");
      strcat(cDynamicPage, "<br> \"Belemn PICPNGS WEB SERVER\"<br>");
      strcat(cDynamicPage, " <a href=\"/about.html\">Additional information.</a>");
      strcat(cDynamicPage, " <a href=\"http://www.belemn.com\">Main site belemn.com</a>");
      strcat(cDynamicPage,
          "<br>************************************************<br>");
      /* ... Finally the page footer. */
      strcat(cDynamicPage, webHTML_END);
      /* Write out the dynamically generated page. */
      netconn_write(pxNetCon, cDynamicPage, (uint16) strlen(cDynamicPage),
          NETCONN_COPY);
      break;
    case FILE_404:
      /* Write out the HTTP OK header. */
      netconn_write(pxNetCon, webHTTP_OK, (u16_t) strlen(webHTTP_OK),
          NETCONN_COPY);

      /* Generate the dynamic page...
       ... First the page header. */
      strcpy(cDynamicPage, webHTML_START);
      /* ... Then the hit count... */
      strcat(cDynamicPage, cPageHits);
      strcat(cDynamicPage,
          "<br>************************************************<br>");
      strcat(cDynamicPage, "<br>404 Page not found !!! <br>");
      strcat(cDynamicPage,
          "<br>************************************************<br>");
      /* ... Finally the page footer. */
      strcat(cDynamicPage, webHTML_END);
      /* Write out the dynamically generated page. */
      netconn_write(pxNetCon, cDynamicPage, (uint16) strlen(cDynamicPage),
          NETCONN_COPY);
      break;
    case FILE_501:
      /* Write out the HTTP OK header. */
      netconn_write(pxNetCon, webHTTP_OK, (u16_t) strlen(webHTTP_OK),
          NETCONN_COPY);

      /* Generate the dynamic page...
       ... First the page header. */
      strcpy(cDynamicPage, webHTML_START);
      /* ... Then the hit count... */
      strcat(cDynamicPage, cPageHits);
      strcat(cDynamicPage,
          "<br>************************************************<br>");
      strcat(cDynamicPage, "<br>404 Page not found !!! <br>");
      strcat(cDynamicPage,
          "<br>************************************************<br>");
      /* ... Finally the page footer. */
      strcat(cDynamicPage, webHTML_END);
      /* Write out the dynamically generated page. */
      netconn_write(pxNetCon, cDynamicPage, (uint16) strlen(cDynamicPage),
          NETCONN_COPY);
      break;
    case FILE_ABOUT:
      /* Write out the HTTP OK header. */
            netconn_write(pxNetCon, webHTTP_OK, (u16_t) strlen(webHTTP_OK),
                NETCONN_COPY);

            /* Generate the dynamic page...
             ... First the page header. */
            strcpy(cDynamicPage, webHTML_START);
            /* ... Then the hit count... */
            strcat(cDynamicPage, cPageHits);
            strcat(cDynamicPage,
                "<br>*******************************************************************************<br>");
            strcat(cDynamicPage,STR_ABOUT);
            strcat(cDynamicPage,
                "<br>*******************************************************************************<br>");
            /* ... Finally the page footer. */
            strcat(cDynamicPage, webHTML_END);
            /* Write out the dynamically generated page. */
            netconn_write(pxNetCon, cDynamicPage, (uint16) strlen(cDynamicPage),
                NETCONN_COPY);
        break;

    case FILE_IMAGE1:
      break;

      }

  }

