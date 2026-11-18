#include "notFound_HTML.h"
#include "brd_settings_HTML.h"
#include "esp_Control_panel_HTML.h"

void custom_webserver_path(){
  #ifdef ENABLE_UDP
    serverOn_UDP();
  #endif

}

void init_WEB_server(){
  HTTP_enabled = 1;

  //init_global_login();

  server.on(wifiPath, HTTP_ANY, [](AsyncWebServerRequest *request){
    request->redirect("/login");
  });

  #if include_indexHTML == 1

    server.on("/", HTTP_ANY, [](AsyncWebServerRequest * request) {
      if (request->client() != nullptr) {
        snprintf(HTTP_client, sizeof(HTTP_client), "%s", request->client()->remoteIP().toString().c_str());
      }

      if (apActive) {
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        response->addHeader("Pragma", "no-cache");
        response->addHeader("Expires", "-1");
        
        SendHTMLStream(response);
        request->send(response);
      } else {
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gz, index_html_gz_len);
        response->addHeader("Content-Encoding", "gzip");
        response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        response->addHeader("Pragma", "no-cache");
        response->addHeader("Expires", "-1");
        request->send(response);
      }
    });

    server.on("/home", HTTP_GET, [](AsyncWebServerRequest * request) {
      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gz, index_html_gz_len);
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    });

  #else

    server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
      if (request->client() != nullptr) {
        snprintf(HTTP_client, sizeof(HTTP_client), "%s", request->client()->remoteIP().toString().c_str());
      }

      if (apActive) {
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        response->addHeader("Pragma", "no-cache");
        response->addHeader("Expires", "-1");
        
        SendHTMLStream(response);
        request->send(response);
      } else {
        AsyncWebServerResponse *response;
        if (LittleFS.exists("/index.html")) {
          response = request->beginResponse(LittleFS, "/index.html", "text/html");
        }
        else if (LittleFS.exists("/index.html.gz")) {
          response = request->beginResponse(LittleFS, "/index.html.gz", "text/html");
          response->addHeader("Content-Encoding", "gzip");
        } else {
          request->redirect("/login");
          DEBUG_PRINTLN("NO index.html redirecting...");
          return;
        }
        response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        response->addHeader("Pragma", "no-cache");
        response->addHeader("Expires", "-1");
        request->send(response);
      }
    });

    server.on("/home", HTTP_GET, [](AsyncWebServerRequest *request) {
      if (LittleFS.exists("/index.html")) {
        request->send(LittleFS, "/index.html", "text/html");
      }
      else if (LittleFS.exists("/index.html.gz")) {
        AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html.gz", "text/html");
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
      } else {
        request->redirect("/login");
      }
    });

  #endif

  server.on(serverURL_ESP_info, HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");
    
    ESPinfo(*response); 
    request->send(response);
  });

  server.on(serverURL_ESP_CP, HTTP_ANY, [](AsyncWebServerRequest * request) {
    /*
    if (!request->authenticate(login_username, login_password)) {
      return request->requestAuthentication();
    }*/
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", esp_cp_html_gz, esp_cp_html_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

    // board settings
  server.on(serverURL_BRD_sett_json, HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    brd_json(*response);
    request->send(response);
  });
  server.on(serverURL_BRD_sett_save, HTTP_ANY, handle_esp_settings_save);

  server.on(serverURL_BRD_sett_html, HTTP_GET, [](AsyncWebServerRequest * request) {
      if (request->client() != nullptr) {
        snprintf(Settings_client, sizeof(Settings_client), "%s", request->client()->remoteIP().toString().c_str());
      }
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", brd_settings_html_gz, brd_settings_html_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");
    request->send(response);
  });

    // scanners
  server.on(serverURL_scan_wifi, HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    scanWifi(*response);
    request->send(response);
  });

  server.on(serverURL_scan_i2c, HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    scan_i2c(*response); 
    request->send(response);
  });

  /* // dont remove
    server.on(serverURL_scan_OneWire, HTTP_GET, [](AsyncWebServerRequest *request) {
      //request->send(200, "application/json", scan_1wire());
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      scan_1wire(*response);
      request->send(response);
    });
  */
    custom_webserver_path();

    //LEAVE this here!!
    server.serveStatic("/", LittleFS, "/");

  server.onNotFound([](AsyncWebServerRequest *request){
      if (request->client() != nullptr) {
        snprintf(NotFound_client, sizeof(NotFound_client), "%s", request->client()->remoteIP().toString().c_str());
      }

    AsyncWebServerResponse *response = request->beginResponse_P(404, "text/html", notFound_html_gz, notFound_html_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");
    request->send(response);
    DEBUG_PRINTF(" 404 od [%s] -> http://%s%s\n", NotFound_client, request->host().c_str(), request->url().c_str());
  });


  // WEB_Start server
  server.begin();
  DEBUG_PRINTLN("WEB server started!");
}


void WIFIMANAGER_server_path(){

  server.on(serverURL_edit_ap, HTTP_ANY, [](AsyncWebServerRequest *request) {
      if (request->client() != nullptr) {
        snprintf(wifi_edit_client, sizeof(wifi_edit_client), "%s", request->client()->remoteIP().toString().c_str());
      }

    AsyncResponseStream *response = request->beginResponseStream("text/html");
      response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
      response->addHeader("Pragma", "no-cache");
      response->addHeader("Expires", "-1");
        SendHTMLStream(response);
      request->send(response);
    DEBUG_PRINTLN("Served AP EDIT Page");
  });

  server.on(serverURL_ESP_cmd, HTTP_POST, handle_wm_cmd);
  server.on(serverURL_wifi_save, HTTP_POST, handle_wifisave);
}
